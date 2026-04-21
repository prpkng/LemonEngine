#pragma once

#include "../API/Helpers.h"
#include <Lemon/Renderer/RHI/Interfaces/ICommandQueue.h>
#include <Lemon/Renderer/RHI/Interfaces/ICommandList.h>

namespace Lemon::DX
{
    class DXCommandList : public RHI::ICommandList
    {
    public:
        DXCommandList(ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12CommandAllocator> allocator,
                      RHI::QueueType queueType);

        void Begin() override;

        void End() override;
        void BindPipeline(std::shared_ptr<RHI::IPipeline> pipeline) override;
        void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
        void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset,
                         u32 firstInstance) override;
        void PushConstants(RHI::ShaderStage stage, u32 offsetIn32BitWords, std::span<const std::byte> data) override;

        // -----------------------------------------------------------------------
        // Internal access for DX12CommandQueue
        // -----------------------------------------------------------------------

        [[nodiscard]] ID3D12GraphicsCommandList* GetHandle() const noexcept { return m_CmdList.Get(); }
        [[nodiscard]] ComPtr<ID3D12CommandAllocator> ReleaseAllocator() { return std::move(m_CmdAllocator); }
        [[nodiscard]] RHI::QueueType GetQueueType() const override { return m_QueueType; }

    private:
        void AssertRecording() const
        {
            LM_CORE_ASSERT(m_IsRecording, "Command list is not in a recodring state. Call Begin() first.")
        }

        ComPtr<ID3D12GraphicsCommandList> m_CmdList;
        ComPtr<ID3D12CommandAllocator> m_CmdAllocator;
        RHI::QueueType m_QueueType;
        bool m_IsRecording = false;
        bool m_WasClosed = false;
    };
} // Lemon::DX
