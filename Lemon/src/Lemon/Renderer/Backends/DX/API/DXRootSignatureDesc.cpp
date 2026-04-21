#include "DXRootSignatureDesc.h"

Lemon::DX::DXRootSignatureDesc& Lemon::DX::DXRootSignatureDesc::AddConstant(const UINT num32BitValues, const UINT shaderRegister,
                                                                            const UINT space, const D3D12_SHADER_VISIBILITY visibility)
{
    CD3DX12_ROOT_PARAMETER p{};
    p.InitAsConstants(num32BitValues, shaderRegister, space, visibility);
    parameters.push_back(p);
    return *this;
}

Lemon::DX::DXRootSignatureDesc& Lemon::DX::DXRootSignatureDesc::AddCBV(const UINT shaderRegister, const UINT space,
    const D3D12_SHADER_VISIBILITY visibility)
{
    CD3DX12_ROOT_PARAMETER p{};
    p.InitAsConstantBufferView(shaderRegister, space, visibility);
    parameters.push_back(p);
    return *this;
}

Lemon::DX::DXRootSignatureDesc& Lemon::DX::DXRootSignatureDesc::AddSRV(const UINT shaderRegister, const UINT space,
    const D3D12_SHADER_VISIBILITY visibility)
{
    CD3DX12_ROOT_PARAMETER p{};
    p.InitAsShaderResourceView(shaderRegister, space, visibility);
    parameters.push_back(p);
    return *this;
}
