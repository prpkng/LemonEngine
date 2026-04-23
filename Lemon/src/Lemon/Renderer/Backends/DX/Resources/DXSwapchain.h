#pragma once

#include "../API/Helpers.h"
#include "../Commands/DXCommandQueue.h"
#include "../DXDevice.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <Lemon/Renderer/RHI/Interfaces/ISwapchain.h>
#include <d3d12.h>
#include <dxgi1_4.h>
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
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetBackbufferView(u32 index) const {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += index * rtvIncrementSize;

        return rtvHandle;
    }
    [[nodiscard]] void* GetBackbuffer(u32 index) const override {
        return m_RenderTargets[index];
    }
    [[nodiscard]] u32 GetBackbufferCount() const override { return static_cast<u32>(m_RenderTargets.size()); }
    [[nodiscard]] RHI::Format GetBackbufferFormat() const override { return m_Format; }


  private:
    void InitRenderTargets(const ComPtr<ID3D12Device>& device, u32 bufferCount);
    RHI::Format m_Format;
    ComPtr<IDXGISwapChain3> m_Swapchain = nullptr;
    std::vector<ID3D12Resource*> m_RenderTargets{};
    ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
    UINT rtvIncrementSize{};
};

} // namespace Lemon::DX