#pragma once
#include "API/DXPSO.h"
#include "Lemon/Renderer/Pipelines/IShaderPipeline.h"
#include "API/DXUtils.h"

namespace Lemon::DX
{
    class DXPipeline final : public RHI::IPipeline
    {
    public:
        explicit DXPipeline(DXPipelineStateObject pso) : m_PsoWrapper(std::move(pso)) {}

        [[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_PsoWrapper.GetRootSignature(); }
        [[nodiscard]] ComPtr<ID3D12PipelineState> GetPSO() const noexcept { return m_PsoWrapper.GetPSO(); }

    private:
        DXPipelineStateObject m_PsoWrapper;
    };
}
