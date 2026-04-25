#include "DXTexture.h"

#include <utility>

#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include "d3d12.h"
#include <utility>

using namespace Lemon::RHI;

namespace Lemon::DX
{

DXTextureView::DXTextureView(const RHI::ITexture&           texture,
                             const RHI::ITextureView::Desc& desc,
                             DXDescriptorHeap::Allocation   allocation)
    : m_Texture(texture), m_Desc(desc), m_Allocation(allocation)
{
    // Inherit format from parent texture if not specified
    if (m_Desc.format == RHI::Format::Unknown)
        m_Desc.format = texture.GetDesc().format;
}

DXTexture::DXTexture(ComPtr<ID3D12Device>   device,
                     ComPtr<ID3D12Resource> resource,
                     DXDescriptorHeap*      srvUavHeap,
                     DXDescriptorHeap*      rtvHeap,
                     DXDescriptorHeap*      dsvHeap,
                     const ITexture::Desc&  desc)
    : m_Device(std::move(device)), m_Resource(std::move(resource)), m_SrvUavHeap(srvUavHeap), m_RtvHeap(rtvHeap),
      m_DsvHeap(dsvHeap), m_Desc(desc)
{
}

std::unique_ptr<RHI::ITextureView> DXTexture::CreateView(const RHI::ITextureView::Desc& desc)
{
    const Format fmt      = desc.format == Format::Unknown ? m_Desc.format : desc.format;
    const u32    mipCount = desc.mipCount == ~0u ? m_Desc.mipLevels - desc.baseMip : desc.mipCount;

    switch (desc.type) {

    case TextureViewType::ShaderResource:
        return CreateSRVInternal(desc, fmt, mipCount);
    case TextureViewType::RenderTarget:
        return CreateRTVInternal(desc, fmt);
    case TextureViewType::DepthStencil:
        return CreateDSVInternal(desc, fmt);
    case TextureViewType::UnorderedAccess:
        return CreateUAVInternal(desc, fmt);
    default:
        LM_CORE_ASSERT(false, "Unknown view type");
        break;
    }
}

std::unique_ptr<RHI::ITextureView>
DXTexture::CreateSRVInternal(const RHI::ITextureView::Desc& desc, RHI::Format fmt, u32 mipCount)
{
    auto allocation = m_SrvUavHeap->Allocate();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = Convert::ToFormat(fmt);
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip       = desc.baseMip;
    srvDesc.Texture2D.MipLevels             = mipCount;
    srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;

    m_Device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, allocation.cpu);
    return std::make_unique<DXTextureView>(*this, desc, allocation);
}

std::unique_ptr<RHI::ITextureView> DXTexture::CreateRTVInternal(const RHI::ITextureView::Desc& desc, RHI::Format fmt)
{
    auto allocation = m_SrvUavHeap->Allocate();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format                        = Convert::ToFormat(fmt);
    rtvDesc.ViewDimension                 = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice            = desc.baseMip;

    m_Device->CreateRenderTargetView(m_Resource.Get(), &rtvDesc, allocation.cpu);
    return std::make_unique<DXTextureView>(*this, desc, allocation);
}

std::unique_ptr<RHI::ITextureView> DXTexture::CreateDSVInternal(const RHI::ITextureView::Desc& desc, RHI::Format fmt)
{
    auto allocation = m_SrvUavHeap->Allocate();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format                        = Convert::ToFormat(fmt);
    dsvDesc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice            = desc.baseMip;
    dsvDesc.Flags                         = D3D12_DSV_FLAG_NONE;

    m_Device->CreateDepthStencilView(m_Resource.Get(), &dsvDesc, allocation.cpu);
    return std::make_unique<DXTextureView>(*this, desc, allocation);
}

std::unique_ptr<RHI::ITextureView> DXTexture::CreateUAVInternal(const RHI::ITextureView::Desc& desc, RHI::Format fmt)
{
    auto allocation = m_SrvUavHeap->Allocate();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format                           = Convert::ToFormat(fmt);
    uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice               = desc.baseMip;

    m_Device->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &uavDesc, allocation.cpu);
    return std::make_unique<DXTextureView>(*this, desc, allocation);
}

} // namespace Lemon::DX