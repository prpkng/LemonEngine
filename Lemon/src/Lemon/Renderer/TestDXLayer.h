#pragma once

#include <Lemon/Layer.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Lemon/Window.h"
#include "Platform/WindowsWindow.h"
#include "Backends/DX/DXDevice.h"
#include "Backends/DX/Resources/DXBuffer.h"

#include "Backends/DX/API/DXPSO.h"
#include "Backends/DX/API/DXRootSignatureDesc.h"
#include "Backends/DX/Commands/DXCommandQueue.h"

namespace Lemon::DX
{
    class DXPipeline;
}

class TestDXLayer : public Lemon::Layer {
public:
    void InitCommandQueue(ComPtr<ID3D12Device> device);

    void InitSwapchain(HWND hwnd);

    void InitRenderTargets(ComPtr<ID3D12Device> device);

    void InitSync(ComPtr<ID3D12Device> device);

    void InitShaderPipeline(const std::shared_ptr<Lemon::DX::DXDevice>& device);

    void InitBuffers(const std::shared_ptr<Lemon::DX::DXDevice>& device);

    explicit TestDXLayer(const std::unique_ptr<Lemon::Window>& wnd);
    ~TestDXLayer() override;

    void OnUpdate() override;

private:
    Lemon::WindowsWindow* window;

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    uint32_t frameIndex = 0;
    std::array<uint64_t, MAX_FRAMES_IN_FLIGHT> frameFenceValues = {};


    std::shared_ptr<Lemon::DX::DXPipeline> pipeline;

    std::shared_ptr<Lemon::DX::DXCommandQueue> graphicsQueue;

    // ID3D12CommandQueue* commandQueue;
    // ID3D12CommandAllocator* commandAllocator;
    // ID3D12GraphicsCommandList* commandList;
    IDXGISwapChain3* swapChain;
    ID3D12Resource* renderTargets[2];
    ID3D12DescriptorHeap* rtvHeap;
    UINT rtvIncrementSize;

    ID3D12Fence* fence;
    UINT64 fenceValue;
    HANDLE fenceEvent;

    std::shared_ptr<Lemon::DX::DXVertexBuffer> vertexBuffer;

    std::shared_ptr<Lemon::DX::DXIndexBuffer> indexBuffer;
};



