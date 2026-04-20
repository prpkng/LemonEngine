#pragma once

#include <lmpch.h>

#include <Lemon/Layer.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Lemon/Window.h"
#include "Platform/WindowsWindow.h"
#include "Backends/DX/DXDevice.h"

class TestDXLayer : public Lemon::Layer {
public:
    void InitCommandQueue(ComPtr<ID3D12Device> device);

    void InitSwapchain(HWND hwnd);

    void InitRenderTargets(ComPtr<ID3D12Device> device);

    void InitSync(ComPtr<ID3D12Device> device);

    void InitShaderPipeline(ComPtr<ID3D12Device> device);

    void InitBuffers(ComPtr<ID3D12Device> device);

    explicit TestDXLayer(std::unique_ptr<Lemon::Window>& wnd);
    ~TestDXLayer() override;

    void OnUpdate() override;

private:
    Lemon::WindowsWindow* window;

    ID3D12RootSignature* rootSignature;
    ID3D12PipelineState* pipelineState;

    ID3D12CommandQueue* commandQueue;
    ID3D12CommandAllocator* commandAllocator;
    ID3D12GraphicsCommandList* commandList;
    IDXGISwapChain3* swapChain;
    ID3D12Resource* renderTargets[2];
    ID3D12DescriptorHeap* rtvHeap;
    UINT rtvIncrementSize;

    ID3D12Fence* fence;
    UINT64 fenceValue;
    HANDLE fenceEvent;

    ID3D12Resource* vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    ID3D12Resource* indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
};



