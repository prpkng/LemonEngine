#pragma once

#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"
#include <cstddef>
#include <span>
namespace Lemon::RHI
{

struct IUploadContext {
    virtual ~IUploadContext() = default;

    /// Schedules a pixel buffer -> texture upload
    /// @param srcRowPitch bytes por row in the SOURCE data (not GPU-aligned)
    /// @code
    /// DX12: maps upload heap, copies row-by-row with 256b alignment padding,
    ///       records CopyTextureRegion + transition barrier
    /// Vulkan: maps host-visible buffer, records vkCmdCopyBufferToImage
    ///         + pipeline barrier to SHADER_READ_ONLY_OPTIMAL
    /// @endcode
    virtual void UploadTexture(ITexture& dest, std::span<const std::byte> pixels, u32 srcRowPitch) = 0;

    /// Schedules a CPU buffer -> GPU buffer upload
    /// @code
    /// DX12: maps upload heap, memcpy, records CopyBufferRegion
    /// Vulkan: maps host-visible buffer, memcpy, records vkCmdCopyBuffer
    /// @endcode
    virtual void UploadBuffer(IBuffer& dest, std::span<const std::byte> data) = 0;

    /// Submits all pending copies and blocks the CPU until the GPU is done.
    /// Afther this call:
    /// - All textures are in ShaderResource state
    /// - All buffers are readable by the GPU
    /// - All staging memory is released
    /// @code
    // DX12: ExecuteCommandLists + Signal + WaitForSingleObject
    // Vulkan: vkQueueSubmit + vkWaitForFences
    /// @endcode
    virtual void Flush() = 0;

    /// Non-blocking variant - returns a fence value that you can wait on later.
    /// Useful for overlapping uploads with other CPU work
    /// @code
    /// DX12: ExecuteCommandLists + Signal, returns the signaled fence value
    /// Vulkan: vkQueueSubmit with a VkFence, returns handle as uint64
    /// @endcode
    [[nodiscard]] virtual u64 FlushAsync() = 0;

    /// Query whether all submitted work is complete without blocking
    [[nodiscard]] virtual bool IsComplete(u64 fenceValue) const = 0;
    
    /// How many bytes of staging memory are currently allocated
    /// Useful for budgeting upload bandwidth per frame
    [[nodiscard]] virtual u64 GetPendingStagingBytes() const = 0;
};

} // namespace Lemon::RHI