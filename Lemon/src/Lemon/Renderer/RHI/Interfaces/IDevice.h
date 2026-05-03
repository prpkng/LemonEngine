#pragma once

#include <Lemon/Core.h>
#include <memory>
#include <string_view>
#include <utility>
#include "IPipeline.h"
#include "IBuffer.h"
#include "ICommandQueue.h"
#include "ISwapchain.h"
#include "IUploadContext.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"

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

        [[nodiscard]] virtual std::shared_ptr<ICommandQueue> CreateCommandQueue(QueueType type) = 0;

        [[nodiscard]] virtual std::shared_ptr<IBuffer> CreateBuffer(const IBuffer::Desc& desc) = 0;

        [[nodiscard]] virtual std::shared_ptr<IPipeline> CreatePipeline(const IPipeline::Desc& desc) = 0;
        
        [[nodiscard]] virtual std::shared_ptr<ISwapchain> CreateSwapchain(const std::shared_ptr<ICommandQueue>& cmdQueue, const ISwapchain::Desc& desc) = 0;

        [[nodiscard]] virtual ITexture* CreateTexture(RHI::ITexture::Desc desc) = 0;

        [[nodiscard]] virtual std::shared_ptr<IUploadContext> CreateUploadContext() = 0;

        [[nodiscard]] ITexture* LoadTexture(std::string_view path, Format format = Format::RGBA8_UNORM, u32 mipLevels = 1);

        [[nodiscard]] virtual std::shared_ptr<ICommandQueue> GetDefaultCopyQueue() = 0;
        [[nodiscard]] virtual std::shared_ptr<ICommandQueue> GetDefaultGraphicsQueue() = 0;
    private:
    };
}
