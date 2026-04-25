
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

DXUploadContext::DXUploadContext(ComPtr<ID3D12Device> device, std::shared_ptr<RHI::ICommandQueue> queue)
    : m_Device(std::move(device)), m_Queue(queue)
{
    // Borrow a command list from the queue's allocator pool.
    m_CmdList = queue->GetCommandList();
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
    m_Device->GetCopyableFootprints(&resourceDesc, 0, 1, // subresource range: mip 0, 1 subresource
                                    0,                   // base offset into staging buffer
                                    nullptr, // we don't need footprint details - UpdateSubresources handles all this
                                    nullptr, nullptr, &requiredSize);

    // 2. Create the staging buffer
    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc      = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);

    ComPtr<ID3D12Resource> stagingBuffer;
    CHECK(m_Device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
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
                                  RHI::ResourceState::PixelShaderResource);

    // Hold staging buffer alive until GPU confirms completion
    m_PendingUploads.push_back({std::move(stagingBuffer)});
    m_PendingStagingBytes += requiredSize;
}

void DXUploadContext::UploadBuffer(RHI::IBuffer& dest, std::span<const std::byte> data)
{
    auto& dxDest = dynamic_cast<DXBuffer&>(dest);

    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc      = CD3DX12_RESOURCE_DESC::Buffer(data.size_bytes());

    ComPtr<ID3D12Resource> stagingBuffer;
    CHECK(m_Device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
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
    m_Queue->CpuWaitForValue(fenceValue);

    // GPU confirmed done - staging buffers safe to release
    m_PendingUploads.clear();
    m_PendingStagingBytes = 0;
}

u64 DXUploadContext::FlushAsync()
{
    m_CmdList->End();

    m_LastFenceValue = m_Queue->SubmitSingle(*m_CmdList);

    // Reborrow a fresh list for the next batch of uploads
    m_CmdList = m_Queue->GetCommandList();
    m_CmdList->Begin();

    return m_LastFenceValue;
}

bool DXUploadContext::IsComplete(u64 fenceValue) const
{
    return m_Queue->GetCompletedValue() >= fenceValue;
}

u64 DXUploadContext::GetPendingStagingBytes() const
{
    return m_PendingStagingBytes;
}

} // namespace Lemon::DX