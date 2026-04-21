#pragma once

#include <Lemon/Core.h>

namespace Lemon::RHI
{
    /// Maps to D3D12_COMMAND_LIST_TYPE / VkQueueFlagBits
    enum class QueueType : u32
    {
        Graphics, /// DX12: D3D12_COMMAND_LIST_TYPE_DIRECT    | Vulkan: VK_QUEUE_GRAPHICS_BIT
        Compute,  /// DX12: D3D12_COMMAND_LIST_TYPE_COMPUTE   | Vulkan: VK_QUEUE_COMPUTE_BIT
        Copy,     /// DX12: D3D12_COMMAND_LIST_TYPE_COPY      | Vulkan: VK_QUEUE_TRANSFER_BIT
    };


}
