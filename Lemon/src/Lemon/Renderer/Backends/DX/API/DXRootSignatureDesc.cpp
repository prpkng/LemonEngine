#include "DXRootSignatureDesc.h"
#include "d3dx12_root_signature.h"

namespace Lemon::DX {

DXRootSignatureDesc& DXRootSignatureDesc::AddConstant(const UINT                    num32BitValues,
                                                      const UINT                    shaderRegister,
                                                      const UINT                    space,
                                                      const D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER p{};
    p.InitAsConstants(num32BitValues, shaderRegister, space, visibility);
    parameters.push_back(p);
    return *this;
}

DXRootSignatureDesc&
DXRootSignatureDesc::AddCBV(const UINT shaderRegister, const UINT space, const D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER p{};
    p.InitAsConstantBufferView(shaderRegister, space, visibility);
    parameters.push_back(p);
    return *this;
}

DXRootSignatureDesc& DXRootSignatureDesc::AddSRVTable(const UINT                    shaderRegister,
                                                      const UINT                    count,
                                                      const UINT                    space,
                                                      const D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_DESCRIPTOR_RANGE& srvRange = descriptorRanges.emplace_back();
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, count, shaderRegister, space);

    CD3DX12_ROOT_PARAMETER p{};
    p.InitAsDescriptorTable(1, &srvRange, visibility);
    parameters.push_back(p);
    return *this;
}

DXRootSignatureDesc& DXRootSignatureDesc::AddStaticSampler(const CD3DX12_STATIC_SAMPLER_DESC& samplerDesc) {
    staticSamplers.push_back(samplerDesc);
    return *this;
}

} // namespace Lemon::DX