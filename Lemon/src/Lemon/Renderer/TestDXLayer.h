#pragma once

#include <lmpch.h>

#include <Lemon/Layer.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Lemon/Window.h"
#include "Platform/WindowsWindow.h"

inline std::string HrToString(HRESULT hr)
{
    switch (hr)
    {
        // Common success
        case S_OK: return "S_OK";

        // DXGI errors
        case DXGI_ERROR_DEVICE_REMOVED: return "DXGI_ERROR_DEVICE_REMOVED";
        case DXGI_ERROR_DEVICE_HUNG: return "DXGI_ERROR_DEVICE_HUNG";
        case DXGI_ERROR_DEVICE_RESET: return "DXGI_ERROR_DEVICE_RESET";
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
        case DXGI_ERROR_INVALID_CALL: return "DXGI_ERROR_INVALID_CALL";
        case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI_ERROR_WAS_STILL_DRAWING";
        case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
        case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
        case DXGI_ERROR_NOT_FOUND: return "DXGI_ERROR_NOT_FOUND";
        case DXGI_ERROR_MORE_DATA: return "DXGI_ERROR_MORE_DATA";
        case DXGI_ERROR_UNSUPPORTED: return "DXGI_ERROR_UNSUPPORTED";

        // D3D12 specific
        case D3D12_ERROR_ADAPTER_NOT_FOUND: return "D3D12_ERROR_ADAPTER_NOT_FOUND";
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";

        // Generic HRESULTs
        case E_FAIL: return "E_FAIL";
        case E_INVALIDARG: return "E_INVALIDARG";
        case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
        case E_NOTIMPL: return "E_NOTIMPL";
        case E_POINTER: return "E_POINTER";

        default:
            break;
    }

    // Fallback: use Windows to get message
    char* msgBuf = nullptr;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuf,
        0,
        nullptr);

    std::ostringstream oss;

    if (size && msgBuf)
    {
        oss << msgBuf;
        LocalFree(msgBuf);
    }
    else
    {
        oss << "Unknown HRESULT";
    }

    // Append hex code (VERY useful)
    oss << " (0x" << std::hex << hr << ")";

    return oss.str();
}

class TestDXLayer : public Lemon::Layer {
public:
    ID3D12Device *InitDevice();

    void InitCommandQueue(ID3D12Device *device);

    void InitSwapchain(HWND hwnd);

    void InitRenderTargets(ID3D12Device *device);

    void InitSync(ID3D12Device *device);

    void InitShaderPipeline(ID3D12Device *device);

    void InitBuffers(ID3D12Device *device);

    explicit TestDXLayer(std::unique_ptr<Lemon::Window>& wnd);
    ~TestDXLayer() override;

    void OnUpdate() override;

    inline void ThrowIfFailed(HRESULT hr, std::string_view msg)
    {
        if (FAILED(hr))
            throw std::runtime_error(std::string(msg) + ": " + HrToString(hr));
    }
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



