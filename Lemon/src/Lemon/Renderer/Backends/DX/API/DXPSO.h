#pragma once

#include <d3d12.h>

#include "DXGraphicsPSODesc.h"
#include "DXRootSignatureDesc.h"
#include "DXUtils.h"

namespace Lemon::DX
{
    class DXPipelineStateObject
    {
    public:
        static DXPipelineStateObject Create(const ComPtr<ID3D12Device>& device,
                                            const DXRootSignatureDesc& rootDesc,
                                            DXGraphicsPSODesc& psoDesc);

        [[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }
        [[nodiscard]] ComPtr<ID3D12PipelineState> GetPSO() const { return m_PipelineState; }
    private:
        ComPtr<ID3D12RootSignature> m_RootSignature;
        ComPtr<ID3D12PipelineState> m_PipelineState;
    };
}
