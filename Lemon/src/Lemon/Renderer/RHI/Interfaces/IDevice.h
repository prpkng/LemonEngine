#pragma once

#include <Lemon/Core.h>
#include <memory>
#include "IPipeline.h"
#include "IBuffer.h"
#include "ICommandQueue.h"
#include "ISwapchain.h"

namespace Lemon::RHI
{

    struct IDevice
    {
        struct Desc
        {
            bool enableDebugLayer;
            bool enableValidationLayer;
            const void* nativeWindowPtr;
            unsigned int initialWidth, initialHeight;
        };

        virtual ~IDevice() = default;

        [[nodiscard]] virtual std::shared_ptr<IBuffer> CreateBuffer(const IBuffer::Desc& desc) = 0;
        [[nodiscard]] virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(const IVertexBuffer::Desc& desc) = 0;
        [[nodiscard]] virtual std::shared_ptr<IIndexBuffer> CreateIndexBuffer(const IIndexBuffer::Desc& desc) = 0;
        [[nodiscard]] virtual std::shared_ptr<IPipeline> CreatePipeline(const IPipeline::Desc& desc) = 0;
        [[nodiscard]] virtual std::shared_ptr<ICommandQueue> CreateCommandQueue(QueueType type) = 0;
        [[nodiscard]] virtual std::shared_ptr<ISwapchain> CreateSwapchain(const std::shared_ptr<ICommandQueue>& cmdQueue, const ISwapchain::Desc& desc) = 0;
    private:
    };
}
