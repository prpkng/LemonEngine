
#include "DXDevice.h"
#include "DXUtils.h"

namespace Lemon::DX
{
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


    }

    DXDevice::~DXDevice()
    {

    }
} // Lemon