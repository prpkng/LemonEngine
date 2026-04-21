#pragma once
#include <wrl/client.h>

#include "d3d12.h"
#include "Lemon/Renderer/Buffers/Buffer.h"
#include "Lemon/Renderer/Buffers/IndexBuffer.h"
#include "Lemon/Renderer/Buffers/VertexBuffer.h"
#include "Lemon/Renderer/Pipelines/IShaderPipeline.h"

using Microsoft::WRL::ComPtr;

namespace Lemon::DX
{
    class DXDevice
    {
    public:
        struct Desc
        {
            bool enableDebugLayer;
            bool enableValidationLayer;
            const void* nativeWindowPtr;
            unsigned int initialWidth, initialHeight;
        };

        std::shared_ptr<RHI::Buffer> CreateBuffer(const RHI::Buffer::Desc& desc);
        std::shared_ptr<RHI::VertexBuffer> CreateVertexBuffer(const RHI::VertexBuffer::Desc& desc);
        std::shared_ptr<RHI::IndexBuffer> CreateIndexBuffer(const RHI::IndexBuffer::Desc& desc);
        std::shared_ptr<RHI::IPipeline> CreatePipeline(const RHI::IPipeline::Desc& desc);

        DXDevice(const Desc& desc);
        ~DXDevice();

        ComPtr<ID3D12Device> GetHandle() const { return m_Handle; }

        ComPtr<ID3D12Device> m_Handle;
    private:
    };
} // Lemon::DX
