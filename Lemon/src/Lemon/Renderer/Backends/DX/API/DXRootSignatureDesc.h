#pragma once


#include <d3dx12.h>
#include <d3d12.h>
#include <vector>

#include "Helpers.h"
#include "../Pipelines/DXShader.h"


namespace Lemon::DX
{
    struct DXRootSignatureDesc
    {
        std::vector<CD3DX12_ROOT_PARAMETER> parameters;
        std::vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;

        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        DXRootSignatureDesc& AddConstant(UINT num32BitValues, UINT shaderRegister, UINT space = 0,
                                        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

        DXRootSignatureDesc& AddCBV(UINT shaderRegister, UINT space = 0,
                                        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

        DXRootSignatureDesc& AddSRV(UINT shaderRegister, UINT space = 0,
                                        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

        // Extend with AddDescriptorTable, AddStaticSampler, etc. as needed

    };
}
