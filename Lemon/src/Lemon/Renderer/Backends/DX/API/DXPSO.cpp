#include "DXPSO.h"

namespace Lemon::DX
{
    DXPipelineStateObject DXPipelineStateObject::Create(const ComPtr<ID3D12Device>& device,
        const DXRootSignatureDesc& rootDesc, DXGraphicsPSODesc& psoDesc)
    {
        DXPipelineStateObject result = {};

        // Build root signature
        CD3DX12_ROOT_SIGNATURE_DESC rsDesc{};
        rsDesc.Init(
            (UINT)rootDesc.parameters.size(),
            rootDesc.parameters.data(),
            (UINT)rootDesc.staticSamplers.size(),
            rootDesc.staticSamplers.empty() ? nullptr : rootDesc.staticSamplers.data(),
            rootDesc.flags);

        ComPtr<ID3DBlob> sigBlob, errBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                         &sigBlob, &errBlob);

        if (FAILED(hr))
        {
            std::string errorMessage(static_cast<const char*>(errBlob->GetBufferPointer()), errBlob->GetBufferSize());
            LM_CORE_FATAL("Failed to serialize root signature! \n{0}", errorMessage);
        }

        CHECK(device->CreateRootSignature(0,
                                          sigBlob->GetBufferPointer(),
                                          sigBlob->GetBufferSize(),
                                          IID_PPV_ARGS(&result.m_RootSignature)),
              "Failed to create root signature");

        // Wire root signature into the PSO desc, then create
        psoDesc.GetHandle().pRootSignature = result.m_RootSignature.Get();

        CHECK(device->CreateGraphicsPipelineState(&psoDesc.GetHandle(), IID_PPV_ARGS(&result.m_PipelineState)),
            "Failed to create pipeline state");

        return result;
    }
}
