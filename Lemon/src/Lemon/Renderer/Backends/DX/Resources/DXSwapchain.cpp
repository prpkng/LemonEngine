#include "DXSwapchain.h"
#include "Lemon/Renderer/Backends/DX/API/Helpers.h"
#include "d3d12.h"
#include <dxgi.h>

namespace Lemon::DX {

DXSwapchain::DXSwapchain(const std::shared_ptr<DXDevice>& device,
                         const std::shared_ptr<DXCommandQueue>& commandQueue,
                         const Desc& desc) : m_Format(desc.format) {
    ComPtr<IDXGIFactory4> factory;
    CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&factory)),
          "Failed to create DXGI factory");

    // Create the swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    swapChainDesc.BufferDesc.Width = desc.width;
    swapChainDesc.BufferDesc.Height = desc.height;
    swapChainDesc.BufferDesc.Format = Convert::ToFormat(desc.format);
    swapChainDesc.BufferCount = desc.bufferCount;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = static_cast<HWND>(desc.windowHandle);
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Windowed = TRUE;

    IDXGISwapChain* tempSwapChain = nullptr;
    CHECK(factory->CreateSwapChain(commandQueue->GetHandle().Get(),
                                   &swapChainDesc, &tempSwapChain),
          "Failed to create swap chain");

    // Cast swap chain to IDXGISwapChain3 to leverage the lastest features
    CHECK(tempSwapChain->QueryInterface(IID_PPV_ARGS(&m_Swapchain)),
          "Failed to cast swap chain");
    tempSwapChain->Release();
    tempSwapChain = nullptr;

    InitRenderTargets(device->GetHandle(), desc.bufferCount);
}

void DXSwapchain::InitRenderTargets(const ComPtr<ID3D12Device>& device, const u32 bufferCount) {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        .NumDescriptors = static_cast<UINT>(GetBackbufferCount()),
    };
    CHECK(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)),
          "Failed to create RTV descriptor heap");
    rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    m_RenderTargets.resize(GetBackbufferCount());

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (u32 i = 0; i < GetBackbufferCount(); i++) {
        CHECK(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i])), "Failed to get swapchain buffer");

        device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvIncrementSize;
    }
}

DXSwapchain::~DXSwapchain() {}

u32 DXSwapchain::AcquireNextBackbuffer() {
    return m_Swapchain->GetCurrentBackBufferIndex();
}

void DXSwapchain::Present(u32 swapInterval) {
    
    CHECK(m_Swapchain->Present(swapInterval, 0), "Failed to present swap chain");
}

void DXSwapchain::Resize(u32 width, u32 height) {} //TODO

} // namespace Lemon::DX
