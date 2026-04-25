#pragma once

#include <Lemon/Core.h>

namespace Lemon::RHI {
/// Maps to D3D12_COMMAND_LIST_TYPE / VkQueueFlagBits
enum class QueueType : u32 {
    Graphics, /// DX12: D3D12_COMMAND_LIST_TYPE_DIRECT    | Vulkan:
              /// VK_QUEUE_GRAPHICS_BIT
    Compute,  /// DX12: D3D12_COMMAND_LIST_TYPE_COMPUTE   | Vulkan:
              /// VK_QUEUE_COMPUTE_BIT
    Copy,     /// DX12: D3D12_COMMAND_LIST_TYPE_COPY      | Vulkan:
              /// VK_QUEUE_TRANSFER_BIT
};

struct Viewport {
    float x, y, width, height;
    float minDepth = 0.0f, maxDepth = 1.0f;
};

struct ScissorRect {
    i32 left, top, right, bottom;
};

// Resource state for barrier abstraction
// DX12: D3D12_RESOURCE_STATE | Vulkan: VkImageLayout + VkAccessFlags
enum class ResourceState : u32 {
    Present,
    RenderTarget,
    Common,
    DepthWrite,
    DepthRead,
    NonPixelShaderResource,
    PixelShaderResource,
    ShaderResource,
    CopySource,
    CopyDest,
    VertexAndConstantBuffer
};

} // namespace Lemon::RHI
