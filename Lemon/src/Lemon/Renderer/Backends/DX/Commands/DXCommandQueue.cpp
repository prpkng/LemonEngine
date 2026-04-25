#include "DXCommandQueue.h"

#include "DXCommandList.h"
#include "Lemon/Renderer/Backends/DX/DXDevice.h"

namespace Lemon::DX
{
    DXCommandQueue::DXCommandQueue(const std::shared_ptr<DXDevice>& device, RHI::QueueType type)
        : m_Device(device), m_QueueType(type), m_NextFenceValue(1), m_LastCompleted(0)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = Convert::ToCommandListType(type);
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        CHECK(m_Device->GetHandle()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)),
            "Failed to create command queue");

        CHECK(m_Device->GetHandle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)),
            "Failed to create fence");

        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    DXCommandQueue::~DXCommandQueue()
    {
        if (m_FenceEvent) CloseHandle(m_FenceEvent);
    }

    std::unique_ptr<RHI::ICommandList> DXCommandQueue::GetCommandList()
    {
        auto allocator = AcquireAllocator();

        ComPtr<ID3D12GraphicsCommandList> cmdList;
        CHECK(m_Device->GetHandle()->CreateCommandList(
            0,
            Convert::ToCommandListType(m_QueueType),
            allocator.Get(),
            nullptr,
            IID_PPV_ARGS(&cmdList)),
            "Failed to create command list");

        // In DX12 lists are created open - no explicit Begin() needed,
        // but we call it anyway to match the RHI contract
        return std::make_unique<DXCommandList>(
            m_Device,
            std::move(cmdList),
            std::move(allocator),
            m_QueueType);
    }

    u64 DXCommandQueue::Submit(std::span<RHI::ICommandList* const> commandLists)
    {
        std::vector<ID3D12CommandList*> dxLists(commandLists.size());

        for (int i = 0; i < commandLists.size(); ++i)
        {
            auto* dxList = static_cast<DXCommandList*>(commandLists[i]);
            dxList->End(); // Close if not already
            dxLists[i] = dxList->GetHandle();

            std::scoped_lock lock(m_AllocatorMutex);
            m_PendingAllocators.push_back({
                dxList->ReleaseAllocator(),
                m_NextFenceValue
            });
        }

        m_CommandQueue->ExecuteCommandLists(
            static_cast<UINT>(dxLists.size()), dxLists.data());

        CHECK(m_CommandQueue->Signal(m_Fence.Get(), m_NextFenceValue),
            "Failed to signal fence");

        return m_NextFenceValue++;
    }

    void DXCommandQueue::CpuWaitForValue(u64 fenceValue)
    {
        if (m_Fence->GetCompletedValue() < fenceValue)
        {
            CHECK(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent), "SetEventOnCompletion failed");
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
        m_LastCompleted = m_Fence->GetCompletedValue();
    }

    void DXCommandQueue::GpuWaitForQueue(ICommandQueue& otherQueue, u64 fenceValue)
    {
        auto& dxOther = static_cast<DXCommandQueue&>(otherQueue);
        CHECK(m_CommandQueue->Wait(dxOther.m_Fence.Get(), fenceValue),
            "Queue GPU wait failed");
    }

    ComPtr<ID3D12CommandAllocator> DXCommandQueue::AcquireAllocator()
    {
        std::scoped_lock lock(m_AllocatorMutex);
        const u64 completed = m_Fence->GetCompletedValue();

        if (!m_PendingAllocators.empty() && m_PendingAllocators.front().fenceValue <= completed)
        {
            auto entry = std::move(m_PendingAllocators.front());
            m_PendingAllocators.pop_front();
            CHECK(entry.allocator->Reset(), "Failed to reset allocator");
            return std::move(entry.allocator);
        }

        // No free allocator - create a fresh one
        ComPtr<ID3D12CommandAllocator> allocator;
        CHECK(m_Device->GetHandle()->CreateCommandAllocator(
            Convert::ToCommandListType(m_QueueType), IID_PPV_ARGS(&allocator)),
            "Failed to create command allocator");

        return allocator;
    }
}
