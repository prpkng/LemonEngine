#include "DXGraphicsPSODesc.h"

#include <cstdint>
#include <algorithm>
#include <Lemon/Renderer/RHI/Interfaces/IShader.h>
#include "Helpers.h"
#include "d3d12.h"

namespace Lemon::DX
{
    DXGraphicsPSODesc::DXGraphicsPSODesc()
    {
        // Safe, draw-ready defaults
        m_Handle.SampleMask = UINT_MAX;
        m_Handle.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        m_Handle.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        m_Handle.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        m_Handle.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        m_Handle.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_Handle.SampleDesc = {1, 0};
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetVertexShader(const void* bytecode, const size_t length)
    {
        m_Handle.VS = {bytecode, length};
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetPixelShader(const void* bytecode, const size_t length)
    {
        m_Handle.PS = {bytecode, length};
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetInputLayout(
        std::vector<D3D12_INPUT_ELEMENT_DESC> layout)
    {
        m_InputLayout = std::move(layout);
        m_Handle.InputLayout = {m_InputLayout.data(), (UINT)m_InputLayout.size()};
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetRenderTargetFormat(const DXGI_FORMAT fmt, const UINT slot)
    {
        m_Handle.RTVFormats[slot] = fmt;
        m_Handle.NumRenderTargets = std::max(m_Handle.NumRenderTargets, slot + 1);
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetDepthStencilFormat(const DXGI_FORMAT fmt)
    {
        m_Handle.DSVFormat = fmt;
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetRasterizerDesc(const RHI::RasterizerState& desc)
    {
        m_Handle.RasterizerState.CullMode = Convert::ToCullMode(desc.cullMode);
        m_Handle.RasterizerState.FillMode = desc.fillMode == RHI::FillMode::Solid ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
        m_Handle.RasterizerState.DepthClipEnable = desc.depthClipEnable;
        m_Handle.RasterizerState.FrontCounterClockwise = desc.frontAsCCW;
        m_Handle.RasterizerState.DepthBias = desc.depthBias;
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetBlendDesc(const RHI::BlendState& blendDesc)
    {
        m_Handle.BlendState.AlphaToCoverageEnable = false;
        m_Handle.BlendState.RenderTarget[0].BlendEnable = blendDesc.blendEnable;
        m_Handle.BlendState.RenderTarget[0].SrcBlend = Convert::ToBlendType(blendDesc.srcBlend);
        m_Handle.BlendState.RenderTarget[0].DestBlend = Convert::ToBlendType(blendDesc.dstBlend);
        m_Handle.BlendState.RenderTarget[0].BlendOp = Convert::ToBlendOp(blendDesc.blendOp);
        m_Handle.BlendState.RenderTarget[0].SrcBlendAlpha = Convert::ToBlendType(blendDesc.srcBlendAlpha);
        m_Handle.BlendState.RenderTarget[0].DestBlendAlpha = Convert::ToBlendType(blendDesc.dstBlendAlpha);
        m_Handle.BlendState.RenderTarget[0].BlendOpAlpha = Convert::ToBlendOp(blendDesc.blendOpAlpha);
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetDepthStencilDesc(const RHI::DepthStencilState& desc)
    {
        m_Handle.DepthStencilState.DepthEnable = desc.depthEnable;
        m_Handle.DepthStencilState.DepthWriteMask = desc.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        m_Handle.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        m_Handle.DepthStencilState.StencilEnable = desc.stencilEnable;
        return *this;
    }


    DXGraphicsPSODesc& DXGraphicsPSODesc::SetTopology(const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
    {
        m_Handle.PrimitiveTopologyType = topology;
        return *this;
    }

    DXGraphicsPSODesc& DXGraphicsPSODesc::SetSampleDesc(const UINT count, const UINT quality)
    {
        m_Handle.SampleDesc = {count, quality};
        return *this;
    }
}
