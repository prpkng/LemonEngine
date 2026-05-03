#include "DXCommandList.h"

#include <utility>

#include "Lemon/Renderer/Backends/DX/API/Helpers.h"
#include "Lemon/Renderer/Backends/DX/Pipelines/DXPipeline.h"
#include "Lemon/Renderer/Backends/DX/Resources/DXBuffer.h"
#include "Lemon/Renderer/Backends/DX/Resources/DXTexture.h"
#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "d3d12.h"

namespace Lemon::DX
{

DXCommandList::DXCommandList(std::shared_ptr<DXDevice>         device,
                             ComPtr<ID3D12GraphicsCommandList> cmdList,
                             ComPtr<ID3D12CommandAllocator>    allocator,
                             const RHI::QueueType              queueType)
    : m_Device(std::move(device)), m_CmdList(std::move(cmdList)), m_CmdAllocator(std::move(allocator)),
      m_QueueType(queueType)
{
}

// DX12 lists are created already open, so Begin() is a no-op here.
// In Vulkan this maps to vkBeginCommandBuffer.
void DXCommandList::Begin()
{
    if (m_IsRecording)
        return;
    m_IsRecording = true;

    // If this list was previously closed and reused,
    // we need to re-open it agains the (already reset) allocator.
    if (m_WasClosed) {
        CHECK(m_CmdList->Reset(m_CmdAllocator.Get(), nullptr), "Failed to reset command list");
        m_WasClosed = false;
    }

    // Set SRV heap
    if (m_QueueType == QueueType::Graphics) {
        ID3D12DescriptorHeap* heaps[] = {m_Device->m_SrvHeap->GetHeap()};
        m_CmdList->SetDescriptorHeaps(1, heaps);
    }
}

// DX12: Close() — must be called before Submit()
// Vulkan: vkEndCommandBuffer
void DXCommandList::End()
{
    if (!m_IsRecording)
        return;
    CHECK(m_CmdList->Close(), "Failed to close the command list");
    m_IsRecording = false;
    m_WasClosed   = true;
}

void DXCommandList::BindPipeline(const std::shared_ptr<RHI::IPipeline> pipeline)
{
    AssertRecording();

    const auto dxPipeline = std::dynamic_pointer_cast<DXPipeline>(pipeline);
    m_CmdList->SetGraphicsRootSignature(dxPipeline->GetRootSignature().Get());
    m_CmdList->SetPipelineState(dxPipeline->GetPSO().Get());
}

void DXCommandList::Draw(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
{
    AssertRecording();

    m_CmdList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void DXCommandList::DrawIndexed(const u32 indexCount,
                                const u32 instanceCount,
                                const u32 firstIndex,
                                const i32 vertexOffset,
                                const u32 firstInstance)
{
    AssertRecording();

    m_CmdList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void DXCommandList::PushConstants(RHI::ShaderStage stage,
                                  const u32        slot,
                                  std::span<const std::byte> data,
                                  u32              offsetIn32BitWords)
{
    AssertRecording();

    // data.size() must be a multiple of 4 (32-bit words)
    LM_CORE_ASSERT(data.size_bytes() % 4 == 0, "Push constant data must be 32-bit aligned");
    const u32 num32BitValues = static_cast<u32>(data.size_bytes() / 4);

    m_CmdList->SetGraphicsRoot32BitConstants(slot, num32BitValues, data.data(), offsetIn32BitWords);
}
void DXCommandList::SetPrimitiveTopology(RHI::PrimitiveTopology topology)
{
    AssertRecording();

    m_CmdList->IASetPrimitiveTopology(Convert::ToTopology(topology));
}

void DXCommandList::TransitionResource(void* resource, const RHI::ResourceState before, const RHI::ResourceState after)
{
    AssertRecording();

    auto fromState = Convert::ToResourceState(before);
    auto toState   = Convert::ToResourceState(after);

    if (fromState == toState)
        return;
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = static_cast<ID3D12Resource*>(resource);
    barrier.Transition.StateBefore = fromState;
    barrier.Transition.StateAfter  = toState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CmdList->ResourceBarrier(1, &barrier);
}

bool DXCommandList::TransitionTextureInternal(void*                    resource,
                                              const RHI::ResourceState before,
                                              const RHI::ResourceState after)
{
    auto fromState = Convert::ToResourceState(before);
    auto toState   = Convert::ToResourceState(after);

    if (fromState == toState)
        return false;

    TransitionResource(resource, before, after);
    return true;
}

void DXCommandList::TransitionTexture(ITexture* texture, ResourceState after)
{
    auto dxTexture = static_cast<DXTexture*>(texture);
    if (TransitionTextureInternal(dxTexture->GetResource(), texture->GetCurrentState(), after))
        dxTexture->SetCurrentState(after);
}

void DXCommandList::TransitionTexture(ITexture* texture, ResourceState before, ResourceState after)
{
    auto dxTexture = static_cast<DXTexture*>(texture);
    if (TransitionTextureInternal(dxTexture->GetResource(), before, after))
        dxTexture->SetCurrentState(after);
}

void DXCommandList::SetViewport(const RHI::Viewport& viewport)
{
    AssertRecording();

    D3D12_VIEWPORT dxViewport{};
    dxViewport.TopLeftX = viewport.x;
    dxViewport.TopLeftY = viewport.y;
    dxViewport.Width    = viewport.width;
    dxViewport.Height   = viewport.height;
    dxViewport.MaxDepth = viewport.maxDepth;
    dxViewport.MinDepth = viewport.minDepth;
    m_CmdList->RSSetViewports(1, &dxViewport);
}

void DXCommandList::SetScissor(const RHI::ScissorRect& scissor)
{
    AssertRecording();

    D3D12_RECT scissorRect{scissor.left, scissor.top, scissor.right, scissor.bottom};
    m_CmdList->RSSetScissorRects(1, &scissorRect);
}
void DXCommandList::BindTexture(u32 slot, const ITextureView* view)
{
    AssertRecording();

    auto* dxView = reinterpret_cast<const DXTextureView*>(view);
    m_CmdList->SetGraphicsRootDescriptorTable(slot, dxView->GetGPUHandle());
}
void DXCommandList::SetRenderTargets(std::vector<const ITextureView*> renderTargetViews,
                                     const ITextureView*              depthStencilView)
{
    AssertRecording();

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles{};
    handles.reserve(renderTargetViews.size());
    for (const auto* view : renderTargetViews) {
        handles.push_back(dynamic_cast<const DXTextureView*>(view)->GetCPUHandle());
    }
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
    if (depthStencilView != nullptr)
        dsvHandle = dynamic_cast<const DXTextureView*>(depthStencilView)->GetCPUHandle();
    m_CmdList->OMSetRenderTargets(static_cast<UINT>(handles.size()), handles.data(), FALSE,
                                  depthStencilView != nullptr ? &dsvHandle : nullptr);
}
void DXCommandList::ClearDepthStencil(const ITextureView* dsv, float depth, uint8_t stencil)
{
    AssertRecording();

    auto* dxView = static_cast<const DXTextureView*>(dsv);
    m_CmdList->ClearDepthStencilView(dxView->GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, depth, stencil, 0, nullptr);
}

void DXCommandList::ClearRenderTarget(const ITextureView* renderTarget, const std::array<float, 4>& color)
{
    AssertRecording();

    auto* dxView = static_cast<const DXTextureView*>(renderTarget);
    m_CmdList->ClearRenderTargetView(dxView->GetCPUHandle(), color.data(), 0, nullptr);
}

void DXCommandList::BindVertexBuffer(const VertexBufferView view)
{
    AssertRecording();


    const auto bufferAsDX = std::dynamic_pointer_cast<DXBuffer>(view.buffer);

    D3D12_VERTEX_BUFFER_VIEW dxView = {};
    dxView.BufferLocation = bufferAsDX->GetVirtualAddress() + view.offset;
    dxView.SizeInBytes = bufferAsDX->GetSize();
    dxView.StrideInBytes = view.stride;

    m_CmdList->IASetVertexBuffers(0, 1, &dxView);
}

void DXCommandList::BindVertexBuffers(std::span<VertexBufferView> views) {
    AssertRecording();

    std::vector<D3D12_VERTEX_BUFFER_VIEW> dxViews{};
    dxViews.reserve(views.size());
    for (const auto& view : views) {
        const auto bufferAsDX = std::dynamic_pointer_cast<DXBuffer>(view.buffer);

        D3D12_VERTEX_BUFFER_VIEW dxView = {};
        dxView.BufferLocation = bufferAsDX->GetVirtualAddress() + view.offset;
        dxView.SizeInBytes = bufferAsDX->GetSize();
        dxView.StrideInBytes = view.stride;
        dxViews.emplace_back(dxView);
    }
    m_CmdList->IASetVertexBuffers(0, static_cast<UINT>(dxViews.size()), dxViews.data());
}

void DXCommandList::BindIndexBuffer(const IndexBufferView view)
{
    AssertRecording();

    const auto bufferAsDX = std::dynamic_pointer_cast<DXBuffer>(view.buffer);

    D3D12_INDEX_BUFFER_VIEW dxView = {};
    dxView.BufferLocation = bufferAsDX->GetVirtualAddress() + view.offset;
    dxView.SizeInBytes = view.size;
    dxView.Format = TranslateElementTypeToFormat(view.indexType);
    m_CmdList->IASetIndexBuffer(&dxView);
}
} // namespace Lemon::DX
