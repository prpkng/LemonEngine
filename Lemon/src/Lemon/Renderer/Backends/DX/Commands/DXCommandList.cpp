//
// Created by guiej on 21/04/2026.
//

#include "DXCommandList.h"

#include "Lemon/Renderer/Backends/DX/Pipelines/DXPipeline.h"
#include "Lemon/Renderer/Backends/DX/Resources/DXBuffer.h"

namespace Lemon::DX
{
    DXCommandList::DXCommandList(ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12CommandAllocator> allocator,
                                 const RHI::QueueType queueType) : m_CmdList(std::move(cmdList)),
                                                             m_CmdAllocator(std::move(allocator)),
                                                             m_QueueType(queueType)
    {
    }

    // DX12 lists are created already open, so Begin() is a no-op here.
    // In Vulkan this maps to vkBeginCommandBuffer.
    void DXCommandList::Begin()
    {
        if (m_IsRecording) return;
        m_IsRecording = true;

        // If this list was previously closed and reused,
        // we need to re-open it agains the (already reset) allocator.
        if (m_WasClosed)
        {
            CHECK(m_CmdList->Reset(m_CmdAllocator.Get(), nullptr),
                "Failed to reset command list");
            m_WasClosed = false;
        }
    }

    // DX12: Close() — must be called before Submit()
    // Vulkan: vkEndCommandBuffer
    void DXCommandList::End()
    {
        if (!m_IsRecording) return;
        CHECK(m_CmdList->Close(), "Failed to close the command list");
        m_IsRecording = false;
        m_WasClosed = true;
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
        m_CmdList->DrawInstanced(vertexCount, instanceCount,
                                firstVertex, firstInstance);

    }

    void DXCommandList::DrawIndexed(const u32 indexCount, const u32 instanceCount, const u32 firstIndex, const i32 vertexOffset,
                                    const u32 firstInstance)
    {
        AssertRecording();
        m_CmdList->DrawIndexedInstanced(indexCount, instanceCount,
                                       firstIndex, vertexOffset, firstInstance);
    }

    void DXCommandList::PushConstants(RHI::ShaderStage stage, const u32 slot, const void* data, size_t dataSize,
        u32 offsetIn32BitWords)
    {
        AssertRecording();

        // data.size() must be a multiple of 4 (32-bit words)
        LM_CORE_ASSERT(dataSize % 4 == 0, "Push constant data must be 32-bit aligned");
        const u32 num32BitValues = static_cast<u32>(dataSize / 4);

        m_CmdList->SetGraphicsRoot32BitConstants(
            slot,
            num32BitValues,
            data,
            offsetIn32BitWords);
    }
    void DXCommandList::SetPrimitiveTopology(RHI::PrimitiveTopology topology)
    {
        m_CmdList->IASetPrimitiveTopology(Convert::ToTopology(topology));
    }

    void DXCommandList::TransitionResource(void* resource, const RHI::ResourceState before, const RHI::ResourceState after)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = static_cast<ID3D12Resource*>(resource);
        barrier.Transition.StateBefore = Convert::ToResourceState(before);
        barrier.Transition.StateAfter = Convert::ToResourceState(after);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        m_CmdList->ResourceBarrier(1, &barrier);
    }

    void DXCommandList::SetViewport(const RHI::Viewport& viewport)
    {
        D3D12_VIEWPORT dxViewport{};
        dxViewport.TopLeftX = viewport.x;
        dxViewport.TopLeftY = viewport.y;
        dxViewport.Width = viewport.width;
        dxViewport.Height = viewport.height;
        dxViewport.MaxDepth = viewport.maxDepth;
        dxViewport.MinDepth = viewport.minDepth;
        m_CmdList->RSSetViewports(1, &dxViewport);
    }

    void DXCommandList::SetScissor(const RHI::ScissorRect& scissor)
    {
        D3D12_RECT scissorRect {scissor.left, scissor.top, scissor.right, scissor.bottom};
        m_CmdList->RSSetScissorRects(1, &scissorRect);
    }

    void DXCommandList::ClearRenderTarget(void* renderTarget, const std::array<float, 4>& color)
    {
        //TODO create proper class for render target
        m_CmdList->ClearRenderTargetView(*static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(renderTarget), color.data(), 0, nullptr);
    }

    void DXCommandList::BindVertexBuffer(const std::shared_ptr<RHI::IVertexBuffer> buffer)
    {
        auto dxBuffer = std::dynamic_pointer_cast<DXVertexBuffer>(buffer);
        m_CmdList->IASetVertexBuffers(0, 1, dxBuffer->GetBufferView());
    }

    void DXCommandList::BindIndexBuffer(const std::shared_ptr<RHI::IIndexBuffer> buffer)
    {
        auto dxBuffer = std::dynamic_pointer_cast<DXIndexBuffer>(buffer);
        m_CmdList->IASetIndexBuffer(dxBuffer->GetBufferView());
    }
} // Lemon::DX
