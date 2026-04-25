
#include "DXUploadContext.h"
#include "DXTexture.h"
#include "Lemon/Renderer/Backends/DX/Commands/DXCommandList.h"
#include "Lemon/Renderer/Backends/DX/Resources/DXBuffer.h"
#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"
#include "d3d12.h"
#include "d3dx12_core.h"
#include "d3dx12_resource_helpers.h"

#include <utility>

namespace Lemon::DX
{

DXUploadContext::DXUploadContext(std::shared_ptr<DXDevice> device)
    : m_Device(std::move(device)), m_CopyQueue(m_Device->GetDefaultCopyQueue()), m_GraphicsQueue(m_Device->GetDefaultGraphicsQueue())
{
    // Borrow a command list from the queue's allocator pool.
    m_CmdList = m_CopyQueue->GetCommandList();
    m_CmdList->Begin();
}

void DXUploadContext::UploadTexture(RHI::ITexture& dest, std::span<const std::byte> pixels, u32 srcRowPitch)
{
    auto&       dxDest = dynamic_cast<DXTexture&>(dest);
    const auto& desc   = dest.GetDesc();

    const D3D12_RESOURCE_DESC resourceDesc = dxDest.GetResource()->GetDesc();

    // 1. Ask DX12 how large the staging buffer needs to be
    u64 requiredSize = 0;
    // GetRequiredIntermediateSize(&resourceDesc, 0, 1)
    m_Device->GetHandle()->GetCopyableFootprints(&resourceDesc, 0, 1, // subresource range: mip 0, 1 subresource
                                    0,                   // base offset into staging buffer
                                    nullptr, // we don't need footprint details - UpdateSubresources handles all this
                                    nullptr, nullptr, &requiredSize);

    // 2. Create the staging buffer
    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc      = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);

    ComPtr<ID3D12Resource> stagingBuffer;
    CHECK(m_Device->GetHandle()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer)),
          "Failed to create staging buffer (upload texture)");

    // 3. Describe the source data
    // D3D12_SUBRESOURCE_DATA tells UpdateSubresources where the CPU
    // pixels are and how they're laid out. It handles the row-by-row
    // copy with alignment padding internally.

    D3D12_SUBRESOURCE_DATA subresourceData = {};

    subresourceData.pData      = pixels.data();
    subresourceData.RowPitch   = srcRowPitch;
    subresourceData.SlicePitch = srcRowPitch * desc.height;

    // 4. UpdateSubresources does the heavy lifting
    //    - Maps the staging buffer
    //    - Copies pixels row by row with correct GPU alignment
    //    - Records CopyTextureRegion into the command list
    //    - Unmaps the staging buffer
    {
        auto* rawCmdList = dynamic_cast<DXCommandList&>(*m_CmdList).GetHandle();
        UpdateSubresources<1>(rawCmdList, dxDest.GetResource(), stagingBuffer.Get(), 0, 0, 1, &subresourceData);
    }
    // 5. Transition texture to shader readable state
    // After CopyTextureRegion the texture is still COPY_DEST.
    // We transition here so it's ready to sample after Flush() returns.
    m_CmdList->TransitionResource(dxDest.GetResource(), RHI::ResourceState::CopyDest,
                                  RHI::ResourceState::Common);

    // Hold staging buffer alive until GPU confirms completion
    m_PendingTransitions.push_back(&dest);
    m_PendingUploads.push_back({std::move(stagingBuffer)});
    m_PendingStagingBytes += requiredSize;
}

void DXUploadContext::UploadBuffer(RHI::IBuffer& dest, std::span<const std::byte> data)
{
    auto& dxDest = dynamic_cast<DXBuffer&>(dest);

    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc      = CD3DX12_RESOURCE_DESC::Buffer(data.size_bytes());

    ComPtr<ID3D12Resource> stagingBuffer;
    CHECK(m_Device->GetHandle()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer)),
          "Failed to create staging buffer (upload buffer)");

    // Buffers don't have alignment concerns like textures —
    // UpdateSubresources handles them the same way, just simpler.
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData                  = data.data();
    subresourceData.RowPitch               = static_cast<LONG_PTR>(data.size_bytes());
    subresourceData.SlicePitch             = static_cast<LONG_PTR>(data.size_bytes());

    {
        auto* rawCmdList = dynamic_cast<DXCommandList&>(*m_CmdList).GetHandle();
        UpdateSubresources<1>(rawCmdList, dxDest.GetHandle().Get(), stagingBuffer.Get(), 0, 0, 1, &subresourceData);
    }

    m_CmdList->TransitionResource(dxDest.GetHandle().Get(), RHI::ResourceState::CopyDest,
                                  RHI::ResourceState::VertexAndConstantBuffer);
    // Hold staging buffer alive until GPU confirms completion
    m_PendingUploads.push_back({std::move(stagingBuffer)});
    m_PendingStagingBytes += data.size_bytes();
}

void DXUploadContext::Flush() {
    const u64 fenceValue = FlushAsync();
    m_CopyQueue->CpuWaitForValue(fenceValue);

    // GPU confirmed done - staging buffers safe to release
    m_PendingUploads.clear();
    m_PendingStagingBytes = 0;
}

u64 DXUploadContext::FlushAsync()
{
    m_CmdList->End();

    const u64 copyDone = m_CopyQueue->SubmitSingle(*m_CmdList);


    if (!m_PendingTransitions.empty()) {
        // Graphics queue waits for copy queue to finish - GPU side only
        m_GraphicsQueue->GpuWaitForQueue(*m_CopyQueue, copyDone);

        auto transitionList = m_GraphicsQueue->GetCommandList();
        transitionList->Begin();

        auto* rawList = dynamic_cast<DXCommandList&>(*transitionList).GetHandle();

        std::vector<D3D12_RESOURCE_BARRIER> barriers;
        barriers.reserve(m_PendingTransitions.size());

        for (auto* texture : m_PendingTransitions) {
            auto& dxTexture = dynamic_cast<DXTexture&>(*texture);
            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                dxTexture.GetResource(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        }

        rawList->ResourceBarrier(
            static_cast<UINT>(barriers.size()), barriers.data());

        transitionList->End();
        m_LastFenceValue = m_GraphicsQueue -> SubmitSingle(*transitionList);

        m_PendingTransitions.clear();
    } else {
        m_LastFenceValue = copyDone;
    }

    

    // Reborrow a fresh list for the next batch of uploads
    m_CmdList = m_CopyQueue->GetCommandList();
    m_CmdList->Begin();

    return m_LastFenceValue;
}

bool DXUploadContext::IsComplete(u64 fenceValue) const
{
    return m_CopyQueue->GetCompletedValue() >= fenceValue;
}

u64 DXUploadContext::GetPendingStagingBytes() const
{
    return m_PendingStagingBytes;
}

} // namespace Lemon::DX