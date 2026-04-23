
#include "DXDevice.h"

#include "Resources/DXBuffer.h"
#include "Pipelines/DXPipeline.h"
#include "Pipelines/DXShader.h"
#include "API/Helpers.h"
#include "API/DXGraphicsPSODesc.h"
#include "API/DXPSO.h"
#include "API/DXRootSignatureDesc.h"
#include "API/Helpers.h"
#include "Commands/DXCommandQueue.h"
#include "Resources/DXSwapchain.h"

namespace Lemon::DX
{
    std::shared_ptr<RHI::IBuffer> DXDevice::CreateBuffer(const RHI::IBuffer::Desc& desc)
    {
        return std::make_shared<DXBuffer>(shared_from_this(), desc);
    }

    std::shared_ptr<RHI::IVertexBuffer> DXDevice::CreateVertexBuffer(const RHI::IVertexBuffer::Desc& desc)
    {
        return std::make_shared<DXVertexBuffer>(shared_from_this(), desc);
    }

    std::shared_ptr<RHI::IIndexBuffer> DXDevice::CreateIndexBuffer(const RHI::IIndexBuffer::Desc& desc)
    {
        return std::make_shared<DXIndexBuffer>(shared_from_this(), desc);
    }

    std::shared_ptr<RHI::IPipeline> DXDevice::CreatePipeline(const RHI::IPipeline::Desc& desc)
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        for (const auto& vertexAttribute : desc.inputLayout)
        {
            inputLayout.push_back({
                vertexAttribute.semanticName.c_str(),
                vertexAttribute.semanticIndex,
                Convert::ToFormat(vertexAttribute.format),
                vertexAttribute.binding,
                vertexAttribute.offset,
                vertexAttribute.inputRate == RHI::InputRate::PerVertex
                    ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
                    : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,
                0
            });
        }

        DXRootSignatureDesc rootDesc;
        for (auto rootParam : desc.rootParameters)
        {
            switch (rootParam.type)
            {
            case RHI::RootParamType::Constants:
                rootDesc.AddConstant(rootParam.count, rootParam.shaderReg, rootParam.space, Convert::ToVisibility(rootParam.visibility));
                break;
            case RHI::RootParamType::ConstantBufferView:
                rootDesc.AddCBV(rootParam.shaderReg, rootParam.space, Convert::ToVisibility(rootParam.visibility));
                break;
            case RHI::RootParamType::ShaderResourceView:
                rootDesc.AddSRV(rootParam.shaderReg, rootParam.space, Convert::ToVisibility(rootParam.visibility));
                break;
            case RHI::RootParamType::UnorderedAccessView:
                //TODO
                break;
            default: ;
            }
        }

        DXShader vertexShader(L"shader.hlsl", RHI::ShaderStage::Vertex);
        DXShader pixelShader(L"shader.hlsl", RHI::ShaderStage::Pixel);

        DXGraphicsPSODesc psoDesc;
        psoDesc
            .SetVertexShader(vertexShader.GetBytecode(), vertexShader.GetLength())
            .SetPixelShader(pixelShader.GetBytecode(), pixelShader.GetLength())
            .SetInputLayout(std::move(inputLayout))
            .SetTopology(Convert::ToTopologyType(desc.topology))

            .SetBlendDesc(desc.blendState)
            .SetDepthStencilDesc(desc.depthStencilState)
            .SetRasterizerDesc(desc.rasterizerState)

            .SetSampleDesc(desc.sampleState.count, desc.sampleState.quality)
            .SetDepthStencilFormat(Convert::ToFormat(desc.depthStencilFormat))
            ;


        for (int i = 0; i < desc.renderTargetFormats.size(); ++i)
        {
            psoDesc.SetRenderTargetFormat(Convert::ToFormat(desc.renderTargetFormats[i]), i);
        }

        return std::make_shared<DXPipeline>(DXPipelineStateObject::Create(m_Handle, rootDesc, psoDesc));
    }

    std::shared_ptr<RHI::ICommandQueue> DXDevice::CreateCommandQueue(RHI::QueueType type)
    {
        return std::make_shared<DXCommandQueue>(shared_from_this(), type);
    }

    std::shared_ptr<RHI::ISwapchain> DXDevice::CreateSwapchain(const std::shared_ptr<RHI::ICommandQueue>& cmdQueue, const RHI::ISwapchain::Desc& desc)
    {
        return std::make_shared<DXSwapchain>(shared_from_this(), std::dynamic_pointer_cast<DXCommandQueue>(cmdQueue), desc);
    }

    DXDevice::DXDevice(const Desc& desc) : IDevice()
    {
        if (desc.enableDebugLayer)
        {
            ComPtr<ID3D12Debug> debugController;
            CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), "Failed to get debug interface");
            debugController->EnableDebugLayer();

            ComPtr<ID3D12Debug1> debug1;
            debugController.As(&debug1);
            debug1->SetEnableGPUBasedValidation(TRUE);

        }

        CHECK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Handle)),
            "Failed to create DX device");

        if (desc.enableDebugLayer)
        {
            ComPtr<ID3D12InfoQueue> infoQueue;
            CHECK(m_Handle->QueryInterface(IID_PPV_ARGS(&infoQueue)), "Failed to retrieve info queue");
            CHECK(infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE), "Failed to set break on severity for DX debug layer");
        }
    }

    DXDevice::~DXDevice() = default;
} // Lemon