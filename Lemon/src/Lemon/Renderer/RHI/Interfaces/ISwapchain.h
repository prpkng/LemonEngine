#pragma once

#include <Lemon/Core.h>
#include <memory>

#include "ICommandQueue.h"
#include "../Types/RHITypes.h"

namespace Lemon::RHI {

struct ISwapchain {
    struct Desc {
        void* windowHandle;
        u32 width, height;
        u32 bufferCount;
        Format format;
        bool vsync;    
    };

    virtual ~ISwapchain() = default;

    virtual u32 AcquireNextBackbuffer() = 0;

    virtual void Present(u32 swapInterval) = 0;

    virtual void Resize(u32 width, u32 height) = 0;

    [[nodiscard]] virtual const void* GetBackbuffer(u32 index) const = 0;
    [[nodiscard]] virtual u32 GetBackbufferCount() const = 0;
    [[nodiscard]] virtual Format GetBackbufferFormat() const = 0;
};

} // namespace Lemon::RHI