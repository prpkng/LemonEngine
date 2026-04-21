#pragma once

#include <Lemon/Core.h>
#include <memory>
#include "IPipeline.h"
#include "IBuffer.h"

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

        virtual std::shared_ptr<IBuffer> CreateBuffer(const IBuffer::Desc& desc) = 0;
        virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(const VertexBuffer::Desc& desc) = 0;
        virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(const IndexBuffer::Desc& desc) = 0;
        virtual std::shared_ptr<IPipeline> CreatePipeline(const IPipeline::Desc& desc) = 0;
    private:
    };
}