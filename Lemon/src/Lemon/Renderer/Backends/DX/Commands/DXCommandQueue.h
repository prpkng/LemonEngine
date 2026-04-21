#pragma once

#include <Lemon/Renderer/RHI/Interfaces/ICommandQueue.h>
#include "../API/Helpers.h"

#include <deque>
#include <mutex>

#include <d3d12.h>

namespace Lemon::DX
{
    class DXDevice;

    /// Per-submission tracking: which allocator was used and what fence
    /// value the GPU will signal when it's done with it
    struct AllocatorEntry
    {
        ComPtr<ID3D12CommandAllocator> allocator;
        u64 fenceValue = 0; /// safe to reset when GPU reaches this value
    };

    ///Owns: the queue, a fence, and a ring-buffer of command allocators
    class DXCommandQueue final : public RHI::ICommandQueue
    {
    public:
        DXCommandQueue(const std::shared_ptr<DXDevice>& device, RHI::QueueType type);
        ~DXCommandQueue() override;

        [[nodiscard]] std::unique_ptr<RHI::ICommandList> GetCommandList() override;
        [[nodiscard]] u64 Submit(std::span<RHI::ICommandList* const> commandLists) override;

        void CpuWaitForValue(u64 fenceValue) override;
        void GpuWaitForQueue(ICommandQueue& otherQueue, u64 fenceValue) override;

        [[nodiscard]] u64 GetLastSubmittedValue() const override { return m_NextFenceValue - 1; }
        [[nodiscard]] u64 GetCompletedValue() const override { return m_Fence->GetCompletedValue(); }
        [[nodiscard]] RHI::QueueType GetQueueType() const override { return m_QueueType; }

        [[nodiscard]] ComPtr<ID3D12CommandQueue> GetHandle() const noexcept { return m_CommandQueue; }

    private:
        [[nodiscard]] ComPtr<ID3D12CommandAllocator> AcquireAllocator();

        std::shared_ptr<DXDevice> m_Device;
        ComPtr<ID3D12CommandQueue> m_CommandQueue;
        ComPtr<ID3D12Fence> m_Fence;
        HANDLE m_FenceEvent = nullptr;
        RHI::QueueType m_QueueType;

        u64 m_NextFenceValue;
        u64 m_LastCompleted;

        std::mutex m_AllocatorMutex;
        std::deque<AllocatorEntry> m_PendingAllocators; //ordered by fenceValue ascending
    };
}
