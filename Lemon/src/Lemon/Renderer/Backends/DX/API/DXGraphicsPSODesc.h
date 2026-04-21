#pragma once

#include <d3dx12.h>
#include <d3d12.h>

#include "Lemon/Renderer/RHI/Types/RHITypes.h"

namespace Lemon::DX
{
    class DXGraphicsPSODesc
    {
    public:
        DXGraphicsPSODesc();

        // --- Shaders ---
        DXGraphicsPSODesc& SetVertexShader(const void* bytecode, size_t length);

        DXGraphicsPSODesc& SetPixelShader(const void* bytecode, size_t length);

        // --- Input layout ---
        // Takes ownership of the layout array for the lifetime of this desc.
        DXGraphicsPSODesc& SetInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC> layout);

        // --- Render targets ---
        DXGraphicsPSODesc& SetRenderTargetFormat(DXGI_FORMAT fmt, UINT slot = 0);

        DXGraphicsPSODesc& SetDepthStencilFormat(DXGI_FORMAT fmt);

        // --- Full setters ---

        DXGraphicsPSODesc& SetRasterizerDesc(const RHI::RasterizerState& rasterizerDesc);
        DXGraphicsPSODesc& SetBlendDesc(const RHI::BlendState& blendDesc);
        DXGraphicsPSODesc& SetDepthStencilDesc(const RHI::DepthStencilState& depthStencilDesc);

        // --- Topology ---
        DXGraphicsPSODesc& SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);

        // --- MSAA ---
        DXGraphicsPSODesc& SetSampleDesc(UINT count, UINT quality = 0);

        // --- Escape hatch: raw access when you need something niche ---
        D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetHandle() { return m_Handle; }

        [[nodiscard]] const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetHandle() const { return m_Handle; }

    private:
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Handle = {};
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
    };
}
