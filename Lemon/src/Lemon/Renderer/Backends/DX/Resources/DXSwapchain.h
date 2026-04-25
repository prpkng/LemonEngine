#pragma once

#include "../API/Helpers.h"
#include "../Commands/DXCommandQueue.h"
#include "../DXDevice.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <Lemon/Renderer/RHI/Interfaces/ISwapchain.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <memory>
#include <vector>


namespace Lemon::DX {

class DXSwapchain : public RHI::ISwapchain {
  public:
    DXSwapchain(const std::shared_ptr<DXDevice>& device,
                const std::shared_ptr<DXCommandQueue>& commandQueue,
                const Desc& desc);
    ~DXSwapchain() override;

    u32 AcquireNextBackbuffer() override;
    void Present(u32 swapInterval) override;
    void Resize(u32 width, u32 height) override;

    //TODO a bit hacky in atp, move this to ISwapchain as soon as we have a proper ITexture and ITexture view interface
    [[nodiscard]] RHI::ITextureView* GetBackbufferView(u32 index) const override {
        return m_RenderTargetViews[index].get();
    }
    [[nodiscard]] std::shared_ptr<RHI::ITexture> GetBackbuffer(u32 index) const override {
        return m_RenderTargets[index];
    }
    [[nodiscard]] u32 GetBackbufferCount() const override { return static_cast<u32>(m_RenderTargets.size()); }
    [[nodiscard]] RHI::Format GetBackbufferFormat() const override { return m_Format; }

    void ResetBackbufferStates();
  private:
    void InitRenderTargets(const std::shared_ptr<DXDevice>& device, u32 bufferCount);
    RHI::Format m_Format;
    Desc m_Desc;
    ComPtr<IDXGISwapChain3> m_Swapchain = nullptr;
    std::vector<std::shared_ptr<RHI::ITexture>> m_RenderTargets{};
    std::vector<std::unique_ptr<RHI::ITextureView>> m_RenderTargetViews{};
};

} // namespace Lemon::DX