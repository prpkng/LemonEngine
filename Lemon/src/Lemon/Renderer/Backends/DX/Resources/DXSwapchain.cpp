#include "DXSwapchain.h"
#include "DXTexture.h"
#include "Lemon/Renderer/Backends/DX/API/Helpers.h"
#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"
#include "d3d12.h"
#include "spdlog/fmt/bundled/format.h"
#include <dxgi.h>
#include <spdlog/fmt/xchar.h>

namespace Lemon::DX
{

DXSwapchain::DXSwapchain(const std::shared_ptr<DXDevice>&       device,
                         const std::shared_ptr<DXCommandQueue>& commandQueue,
                         const Desc&                            desc)
    : m_Format(desc.format), m_Desc(desc)
{
    ComPtr<IDXGIFactory4> factory;
    CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "Failed to create DXGI factory");

    // Create the swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    swapChainDesc.BufferDesc.Width   = desc.width;
    swapChainDesc.BufferDesc.Height  = desc.height;
    swapChainDesc.BufferDesc.Format  = Convert::ToFormat(desc.format);
    swapChainDesc.BufferCount        = desc.bufferCount;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow       = static_cast<HWND>(desc.windowHandle);
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Windowed           = TRUE;

    IDXGISwapChain* tempSwapChain = nullptr;
    CHECK(factory->CreateSwapChain(commandQueue->GetHandle().Get(), &swapChainDesc, &tempSwapChain),
          "Failed to create swap chain");

    // Cast swap chain to IDXGISwapChain3 to leverage the lastest features
    CHECK(tempSwapChain->QueryInterface(IID_PPV_ARGS(&m_Swapchain)), "Failed to cast swap chain");
    tempSwapChain->Release();
    tempSwapChain = nullptr;

    InitRenderTargets(device, desc.bufferCount);
}

void DXSwapchain::InitRenderTargets(const std::shared_ptr<DXDevice>& device, const u32 bufferCount)
{
    for (u32 i = 0; i < bufferCount; i++) {

        // Retrieve the existing backbuffer texture
        ComPtr<ID3D12Resource> backbuffer;
        CHECK(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)), "Failed to get swapchain backbuffer");

        backbuffer->SetName(std::wstring(fmt::format(L"Swapchain Backbuffer {0}", i)).c_str());

        // Build a texture desc that matches the backbuffer DXGI created
        auto desc           = RHI::ITexture::Desc(m_Desc.width, m_Desc.height, m_Desc.format);
        desc.isRenderTarget = true;
        desc.initialState   = RHI::ResourceState::Present;
        desc.debugName      = fmt::format("Swapchain Backbuffer {0}", i);

        /// Pass everything to DXTexture
        auto texture = std::make_shared<DXTexture>(device->GetHandle(),
                                                   std::move(backbuffer), // the REAL swapchain resource
                                                   device->m_SrvHeap.get(), device->m_RtvHeap.get(),
                                                   device->m_DsvHeap.get(), desc);

        m_RenderTargets.push_back(texture);
        m_RenderTargetViews.push_back(texture->CreateRTV());
    }
}

DXSwapchain::~DXSwapchain() {}

u32 DXSwapchain::AcquireNextBackbuffer() { return m_Swapchain->GetCurrentBackBufferIndex(); }

void DXSwapchain::Present(u32 swapInterval)
{

    CHECK(m_Swapchain->Present(swapInterval, 0), "Failed to present swap chain");
}

void DXSwapchain::ResetBackbufferStates()
{
    for (auto& texture : m_RenderTargets) {
        auto* dxTexture = dynamic_cast<DXTexture*>(texture.get());
        dxTexture->SetCurrentState(ResourceState::Present);
    }
}

void DXSwapchain::Resize(u32 width, u32 height) {} // TODO

} // namespace Lemon::DX
