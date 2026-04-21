#pragma once

#include "../Types/RHICommandTypes.h"
#include "ICommandList.h"
#include <memory>
#include <span>

namespace Lemon::RHI
{
    /// ICommandQueue - Submission, sync, and allocator lifetime manager
    ///
    /// Backend-specific info:
    /// \code
    /// DX12: ID3D12CommandQueue
    ///       Internally manages a pool of ID3D12CommandAllocators (one per
    ///       frame-in-flight), reset only after the fence confirms GPU is done.
    ///
    /// Vulkan: VkQueue
    ///         Internally manages a pool of VkCommandPools (same logic).
    ///         VkCommandBuffers are allocated from pools, not individually reset.
    /// \endcode
    struct ICommandQueue
    {
        virtual ~ICommandQueue() = default;

        /// Borrow a command list ready for recording.
        ///
        /// (Internally) picks a free allocator (or waits), resets it, and returns a list
        ///\code
        ///DX12: allocator->Reset(), list->Reset(allocator, nullptr)
        ///Vulkan: vkResetCommandPool or vkResetCommandBuffer, vkBeginCommandBuffer
        ///\endcode
        [[nodiscard]] virtual std::unique_ptr<ICommandList> GetCommandList() = 0;

        /// Submit one or more closed command lists.
        /// \returns The fence value the GPU will signal when done
        ///\code
        /// DX12: ID3D12CommandQueue::Wait(fence, value)
        /// Vulkan: vkQueueSubmit2 with VkSemaphoreSubmitInfo (wait semaphore)
        ///\endcode
        virtual u64 Submit(std::span<ICommandList* const> commandLists) = 0;

        /// Convenience single-list submit
        u64 SubmitSingle(ICommandList& cmdList)
        {
            ICommandList* p = &cmdList;
            return Submit({&p, 1});
        }

        /// Stall CPU until a previously returned fence value is reached.
        /// Use sparingly - prefer overlapping frames with a ring of allocators.
        ///\code
        /// DX12: fence->SetEventOnCompletion + WaitForSingleObject
        /// Vulkan: vkWaitForFences
        ///\endcode
        virtual void CpuWaitForValue(u64 fenceValue) = 0;

        /// GPU-side wait: this queue stalls until another queue's signal is reawched.
        /// Useful for graphics queue wanting on a copy queue upload to finish
        ///
        /// \code
        /// DX12: ID3D12CommandQueue::Wait(fence, value)
        /// Vulkan: vkQueueSubmit2 with VkSemaphoreSubmitInfo (wait semaphore)
        /// \endcode
        virtual void GpuWaitForQueue(ICommandQueue& otherQueue, u64 fenceValue) = 0;

        [[nodiscard]] virtual u64 GetLastSubmittedValue() const = 0;
        [[nodiscard]] virtual u64 GetCompletedValue()     const = 0;
        [[nodiscard]] virtual QueueType GetQueueType()    const = 0;


    };
}
