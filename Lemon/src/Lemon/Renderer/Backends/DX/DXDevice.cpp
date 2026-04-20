
#include "DXDevice.h"

#include "DXBuffer.h"
#include "DXIndexBuffer.h"
#include "DXUtils.h"
#include "DXVertexBuffer.h"

namespace Lemon::DX
{
    std::shared_ptr<RHI::Buffer> DXDevice::CreateBuffer(const RHI::Buffer::Desc& desc)
    {
        return std::make_shared<DXBuffer>(this, desc);
    }

    std::shared_ptr<RHI::VertexBuffer> DXDevice::CreateVertexBuffer(const RHI::VertexBuffer::Desc& desc)
    {
        return std::make_shared<DXVertexBuffer>(this, desc);
    }

    std::shared_ptr<RHI::IndexBuffer> DXDevice::CreateIndexBuffer(const RHI::IndexBuffer::Desc& desc)
    {
        return std::make_shared<DXIndexBuffer>(this, desc);
    }

    DXDevice::DXDevice(const Desc& desc)
    {
        if (desc.enableDebugLayer)
        {
            ComPtr<ID3D12Debug> debugController;
            CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), "Failed to get debug interface");
            debugController->EnableDebugLayer();

        }

        CHECK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Handle)),
            "Failed to create DX device");

        if (desc.enableDebugLayer)
        {
            ComPtr<ID3D12InfoQueue> infoQueue;
            CHECK(m_Handle->QueryInterface(IID_PPV_ARGS(&infoQueue)), "Failed to retrieve info queue");
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        }
    }

    DXDevice::~DXDevice()
    {

    }
} // Lemon