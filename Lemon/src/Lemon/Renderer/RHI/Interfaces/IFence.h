#pragma once
#include <Lemon/Core.h>

namespace Lemon::RHI
{
    /// \brief Opaque CPU/GPU sync primitive
    ///
    /// In DX12: ID3D12Fence + event handle
    /// In Vulkan: VkFence (CPU sync) or VkSemaphore (GPU-GPU sync)
    struct IFence {
        virtual ~IFence() = default;

        // Block the CPU until GPU reaches this value
        virtual void CpuWait(u64 value) = 0;

        // Query without blocking
        [[nodiscard]] virtual u64 GetCompletedValue() const = 0;
    };
}