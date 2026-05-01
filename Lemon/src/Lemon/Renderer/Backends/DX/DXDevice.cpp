#include "DXDevice.h"

#include "API/DXGraphicsPSODesc.h"
#include "API/DXPSO.h"
#include "API/DXRootSignatureDesc.h"
#include "API/Helpers.h"
#include "Commands/DXCommandQueue.h"
#include "Lemon/Renderer/Backends/DX/API/Helpers.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"
#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"
#include "Pipelines/DXPipeline.h"
#include "Pipelines/DXShader.h"
#include "Resources/DXBuffer.h"
#include "Resources/DXSwapchain.h"
#include "Resources/DXTexture.h"
#include "Resources/DXUploadContext.h"
#include "d3d12.h"
#include <codecvt>
#include <memory>

namespace Lemon::DX
{

DXDevice::DXDevice(const Desc& desc) : IDevice()
{
    if (desc.enableDebugLayer) {
        ComPtr<ID3D12Debug> debugController;
        CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), "Failed to get debug interface");
        debugController->EnableDebugLayer();

        ComPtr<ID3D12Debug1> debug1;
        debugController.As(&debug1);
        debug1->SetEnableGPUBasedValidation(TRUE);
    }

    CHECK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Handle)), "Failed to create DX device");

    if (desc.enableDebugLayer) {
        ComPtr<ID3D12InfoQueue> infoQueue;
        CHECK(m_Handle->QueryInterface(IID_PPV_ARGS(&infoQueue)), "Failed to retrieve info queue");
        CHECK(infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE),
              "Failed to set break on severity for DX debug layer");
    }

    // Create heaps
    m_SrvHeap = std::make_unique<DXDescriptorHeap>(m_Handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                                   1024, // max textures + CBV (//TODO increase this later)
                                                   true);
    m_RtvHeap = std::make_unique<DXDescriptorHeap>(m_Handle, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64, false);
    m_DsvHeap = std::make_unique<DXDescriptorHeap>(m_Handle, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 16, false);
}

DXDevice::~DXDevice() = default;
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

std::wstring StringToWString(const std::string& str)
{
    std::wstring wstr;
    size_t       size;
    wstr.resize(str.length());
    mbstowcs_s(&size, &wstr[0], wstr.size() + 1, str.c_str(), str.size());
    return wstr;
}

std::string WStringToString(const std::wstring& wstr)
{
    std::string str;
    size_t      size;
    str.resize(wstr.length());
    wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
    return str;
}

std::shared_ptr<RHI::IPipeline> DXDevice::CreatePipeline(const RHI::IPipeline::Desc& desc)
{
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
    for (const auto& vertexAttribute : desc.inputLayout) {
        inputLayout.push_back(
            {vertexAttribute.semanticName.c_str(), vertexAttribute.semanticIndex,
             Convert::ToFormat(vertexAttribute.format), vertexAttribute.binding, vertexAttribute.offset,
             vertexAttribute.inputRate == RHI::InputRate::PerVertex ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
                                                                    : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,
             0});
    }

    DXRootSignatureDesc rootDesc;
    for (auto rootParam : desc.rootParameters) {
        switch (rootParam.type) {
        case RHI::RootParamType::Constants:
            rootDesc.AddConstant(rootParam.count, rootParam.shaderReg, rootParam.space,
                                 Convert::ToVisibility(rootParam.visibility));
            break;
        case RHI::RootParamType::ConstantBufferView:
            rootDesc.AddCBV(rootParam.shaderReg, rootParam.space, Convert::ToVisibility(rootParam.visibility));
            break;
        case RHI::RootParamType::ShaderResourceView:
            rootDesc.AddSRVTable(rootParam.shaderReg, rootParam.count, rootParam.space,
                                 Convert::ToVisibility(rootParam.visibility));
            break;
        case RHI::RootParamType::UnorderedAccessView:
            // TODO
            break;
        default:;
        }
    }

    for (auto staticSampler : desc.staticSamplers) {
        CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Init(staticSampler.shaderRegister, Convert::ToFilter(staticSampler.filter),
                         Convert::ToAddressMode(staticSampler.addressU), Convert::ToAddressMode(staticSampler.addressV),
                         Convert::ToAddressMode(staticSampler.addressW));
        samplerDesc.ShaderVisibility = Convert::ToVisibility(staticSampler.visibility);
        samplerDesc.RegisterSpace    = staticSampler.space;
        rootDesc.AddStaticSampler(samplerDesc);
    }

    DXShader vertexShader(StringToWString(desc.vertexShaderPath), RHI::ShaderStage::Vertex);
    DXShader pixelShader(StringToWString(desc.pixelShaderPath), RHI::ShaderStage::Pixel);

    DXGraphicsPSODesc psoDesc;
    psoDesc.SetVertexShader(vertexShader.GetBytecode(), vertexShader.GetLength())
        .SetPixelShader(pixelShader.GetBytecode(), pixelShader.GetLength())
        .SetInputLayout(std::move(inputLayout))
        .SetTopology(Convert::ToTopologyType(desc.topology))

        .SetBlendDesc(desc.blendState)
        .SetDepthStencilDesc(desc.depthStencilState)
        .SetRasterizerDesc(desc.rasterizerState)

        .SetSampleDesc(desc.sampleState.count, desc.sampleState.quality)
        .SetDepthStencilFormat(Convert::ToFormat(desc.depthStencilFormat));

    for (int i = 0; i < desc.renderTargetFormats.size(); ++i) {
        psoDesc.SetRenderTargetFormat(Convert::ToFormat(desc.renderTargetFormats[i]), i);
    }

    return std::make_shared<DXPipeline>(DXPipelineStateObject::Create(m_Handle, rootDesc, psoDesc));
}

std::shared_ptr<RHI::ICommandQueue> DXDevice::CreateCommandQueue(RHI::QueueType type)
{
    return std::make_shared<DXCommandQueue>(shared_from_this(), type);
}

std::shared_ptr<RHI::ISwapchain> DXDevice::CreateSwapchain(const std::shared_ptr<RHI::ICommandQueue>& cmdQueue,
                                                           const RHI::ISwapchain::Desc&               desc)
{
    return std::make_shared<DXSwapchain>(shared_from_this(), std::dynamic_pointer_cast<DXCommandQueue>(cmdQueue), desc);
}

static D3D12_RESOURCE_FLAGS BuildResourceFlags(const RHI::ITexture::Desc& desc) noexcept
{
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if (desc.isRenderTarget)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    if (desc.isDepthStencil)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
                 D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE; // remove this if you want to sample the depth later

    if (desc.allowUnorderedAccess)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return flags;
}

std::shared_ptr<RHI::ITexture> DXDevice::CreateTexture(RHI::ITexture::Desc desc)
{
    // Build the D3D12 resource description
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Alignment           = 0;
    textureDesc.Width               = desc.width;
    textureDesc.Height              = desc.height;
    textureDesc.DepthOrArraySize    = static_cast<UINT16>(desc.arraySize);
    textureDesc.MipLevels           = static_cast<UINT16>(desc.arraySize);
    textureDesc.Format              = Convert::ToFormat(desc.format);
    textureDesc.SampleDesc          = {1, 0};
    textureDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags               = BuildResourceFlags(desc);

    // Determine initial state and clear value
    std::optional<D3D12_CLEAR_VALUE> clearValue;

    if (desc.initialState == RHI::ResourceState::Common && desc.isRenderTarget) {
        desc.initialState = RHI::ResourceState::RenderTarget;
        clearValue   = D3D12_CLEAR_VALUE{
            .Format = Convert::ToFormat(desc.format),
            .Color  = {0.0f, 0.0f, 0.0f, 1.0f},
        };
    } else if (desc.initialState == RHI::ResourceState::Common && desc.isDepthStencil) {
        desc.initialState = RHI::ResourceState::DepthWrite;
        clearValue   = D3D12_CLEAR_VALUE{
            .Format       = Convert::ToFormat(desc.format),
            .DepthStencil = {1.0f, 0},
        };
    }

    // Allocate GPU memory and create the resource
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    ComPtr<ID3D12Resource> resource;
    CHECK(GetHandle()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, Convert::ToResourceState(desc.initialState),
                                               clearValue.has_value() ? &clearValue.value() : nullptr,
                                               IID_PPV_ARGS(&resource)),
          "Failed to create texture resource");

    // Set debug name if available
    if (!desc.debugName.empty()) {
        std::wstring wideName(desc.debugName.begin(), desc.debugName.end());
        resource->SetName(wideName.c_str());
    }

    /// Pass everything to DXTexture
    return std::make_shared<DXTexture>(GetHandle(), std::move(resource), m_SrvHeap.get(), m_RtvHeap.get(),
                                       m_DsvHeap.get(), desc);
}

std::shared_ptr<RHI::IUploadContext> DXDevice::CreateUploadContext() {
    return std::make_shared<DXUploadContext>(shared_from_this());
}


std::shared_ptr<RHI::ICommandQueue> DXDevice::GetDefaultCopyQueue() {
    if (m_DefaultCopyQueue != nullptr) return m_DefaultCopyQueue;

    m_DefaultCopyQueue = CreateCommandQueue(RHI::QueueType::Copy);
    return m_DefaultCopyQueue;
}

std::shared_ptr<RHI::ICommandQueue> DXDevice::GetDefaultGraphicsQueue() {
    if (m_DefaultGraphicsQueue != nullptr) return m_DefaultGraphicsQueue;

    m_DefaultGraphicsQueue = CreateCommandQueue(RHI::QueueType::Graphics);
    return m_DefaultGraphicsQueue;
}


} // namespace Lemon::DX