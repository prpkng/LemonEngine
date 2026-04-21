//
// Created by guiej on 21/04/2026.
//

#include "DXCommandList.h"

#include "Lemon/Renderer/Backends/DX/Pipelines/DXPipeline.h"

namespace Lemon::DX
{
    DXCommandList::DXCommandList(ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12CommandAllocator> allocator,
                                 RHI::QueueType queueType) : m_CmdList(std::move(cmdList)),
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

    void DXCommandList::BindPipeline(std::shared_ptr<RHI::IPipeline> pipeline)
    {
        AssertRecording();
        const auto dxPipeline = std::dynamic_pointer_cast<DXPipeline>(pipeline);
        m_CmdList->SetGraphicsRootSignature(dxPipeline->GetRootSignature().Get());
        m_CmdList->SetPipelineState(dxPipeline->GetPSO().Get());
    }

    void DXCommandList::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        AssertRecording();
        m_CmdList->DrawInstanced(vertexCount, instanceCount,
                                firstVertex, firstInstance);

    }

    void DXCommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset,
                                    u32 firstInstance)
    {
        AssertRecording();
        m_CmdList->DrawIndexedInstanced(indexCount, instanceCount,
                                       firstIndex, vertexOffset, firstInstance);
    }

    void DXCommandList::PushConstants(RHI::ShaderStage stage, u32 offsetIn32BitWords, std::span<const std::byte> data)
    {
        AssertRecording();

        // data.size() must be a multiple of 4 (32-bit words)
        LM_CORE_ASSERT(data.size() % 4 == 0, "Push constant data must be 32-bit aligned");
        const u32 num32BitValues = static_cast<u32>(data.size() / 4);

        m_CmdList->SetGraphicsRoot32BitConstants(
            0,
            num32BitValues,
            data.data(),
            offsetIn32BitWords);
    }
} // Lemon::DX
