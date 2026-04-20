#pragma once
#include <wrl/client.h>

#include "d3d12.h"

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

        DXDevice(const Desc& desc);
        ~DXDevice();

        ComPtr<ID3D12Device> m_Handle;
    private:
    };
} // Lemon::DX
