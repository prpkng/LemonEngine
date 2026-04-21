#pragma once
#include <wrl/client.h>

#include "d3d12.h"
#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "Lemon/Renderer/RHI/Interfaces/IDevice.h"
#include "Lemon/Renderer/RHI/Interfaces/IPipeline.h"

using Microsoft::WRL::ComPtr;

namespace Lemon::DX
{
    class DXDevice : public RHI::IDevice, public std::enable_shared_from_this<DXDevice>
    {
    public:
        struct Desc
        {
            bool enableDebugLayer;
            bool enableValidationLayer;
            const void* nativeWindowPtr;
            unsigned int initialWidth, initialHeight;
        };

        [[nodiscard]] std::shared_ptr<RHI::IBuffer> CreateBuffer(const RHI::IBuffer::Desc& desc) override;
        [[nodiscard]] std::shared_ptr<RHI::VertexBuffer> CreateVertexBuffer(const RHI::VertexBuffer::Desc& desc) override;
        [[nodiscard]] std::shared_ptr<RHI::IndexBuffer> CreateIndexBuffer(const RHI::IndexBuffer::Desc& desc) override;
        [[nodiscard]] std::shared_ptr<RHI::IPipeline> CreatePipeline(const RHI::IPipeline::Desc& desc) override;
        [[nodiscard]] std::shared_ptr<RHI::ICommandQueue> CreateCommandQueue(RHI::QueueType type) override;


        explicit DXDevice(const Desc& desc);
        ~DXDevice() override;

        ComPtr<ID3D12Device> GetHandle() const { return m_Handle; }

        ComPtr<ID3D12Device> m_Handle;
    private:
    };
} // Lemon::DX
