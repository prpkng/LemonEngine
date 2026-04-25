#pragma once

#include "../API/Helpers.h"
#include "Lemon/Renderer/Backends/DX/API/DXDescriptorHeap.h"
#include "Lemon/Renderer/Backends/DX/Commands/DXCommandList.h"
#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include "d3d12.h"

#include <Lemon/Renderer/RHI/Interfaces/ITexture.h>

namespace Lemon::DX
{

class DXTextureView final : public RHI::ITextureView
{
public:
    DXTextureView(const RHI::ITexture&           texture,
                  const RHI::ITextureView::Desc& desc,
                  DXDescriptorHeap::Allocation   allocation);

    [[nodiscard]] RHI::TextureViewType           GetType() const override { return m_Desc.type; }
    [[nodiscard]] const RHI::ITextureView::Desc& GetDesc() const override { return m_Desc; }
    [[nodiscard]] const RHI::ITexture&           GetTexture() const override { return m_Texture; }

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_Allocation.cpu; }
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_Allocation.gpu; }

private:
    const RHI::ITexture&         m_Texture;
    RHI::ITextureView::Desc      m_Desc;
    DXDescriptorHeap::Allocation m_Allocation;
};

class DXTexture final : public RHI::ITexture
{
public:
    DXTexture(const DXTexture&)            = default;
    DXTexture(DXTexture&&)                 = default;
    DXTexture& operator=(const DXTexture&) = default;
    DXTexture& operator=(DXTexture&&)      = default;

    DXTexture(ComPtr<ID3D12Device>   device,
              ComPtr<ID3D12Resource> resource,
              DXDescriptorHeap*      srvUavHeap,
              DXDescriptorHeap*      rtvHeap,
              DXDescriptorHeap*      dsvHeap,
              ITexture::Desc         desc);

    [[nodiscard]] const Desc& GetDesc() const override { return m_Desc; }
    [[nodiscard]] u32         GetWidth() const override { return m_Desc.width; }
    [[nodiscard]] u32         GetHeight() const override { return m_Desc.height; }
    [[nodiscard]] u32         GetMipLevels() const override { return m_Desc.mipLevels; }

    [[nodiscard]] RHI::ResourceState GetCurrentState() const override { return m_CurrentState; }

    [[nodiscard]] std::unique_ptr<RHI::ITextureView> CreateView(const RHI::ITextureView::Desc& desc) override;

    [[nodiscard]] ID3D12Resource* GetResource() const { return m_Resource.Get(); }

    void SetCurrentState(ResourceState state) { m_CurrentState = state; }
private:
    [[nodiscard]] std::unique_ptr<RHI::ITextureView>
    CreateSRVInternal(const RHI::ITextureView::Desc& desc, RHI::Format fmt, u32 mipCount);
    [[nodiscard]] std::unique_ptr<RHI::ITextureView> CreateRTVInternal(const RHI::ITextureView::Desc& desc,
                                                                       RHI::Format                    fmt);
    [[nodiscard]] std::unique_ptr<RHI::ITextureView> CreateDSVInternal(const RHI::ITextureView::Desc& desc,
                                                                       RHI::Format                    fmt);
    [[nodiscard]] std::unique_ptr<RHI::ITextureView> CreateUAVInternal(const RHI::ITextureView::Desc& desc,
                                                                       RHI::Format                    fmt);

    ComPtr<ID3D12Device>   m_Device;
    ComPtr<ID3D12Resource> m_Resource;
    DXDescriptorHeap*      m_SrvUavHeap;
    DXDescriptorHeap*      m_RtvHeap;
    DXDescriptorHeap*      m_DsvHeap;
    RHI::ITexture::Desc    m_Desc;
    RHI::ResourceState     m_CurrentState;
};
} // namespace Lemon::DX