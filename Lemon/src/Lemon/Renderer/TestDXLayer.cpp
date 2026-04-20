#include "TestDXLayer.h"
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

#include "Platform/WindowsWindow.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_video.h"

#define CHECK(x, msg) {HRESULT hr = x; if (hr != S_OK) logHRError(hr, msg); }

std::string HrToString(HRESULT hr)
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

void logHRError(HRESULT hr, std::string_view msg) {
    auto errorMsg = HrToString(hr);
    LM_CORE_FATAL("{0}: ERROR {1}", msg, errorMsg);
}

void setBlendState(D3D12_BLEND_DESC &blendDesc) {
    blendDesc = {};

    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {};
    defaultRenderTargetBlendDesc.BlendEnable = FALSE;
    defaultRenderTargetBlendDesc.LogicOpEnable = FALSE;
    defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    defaultRenderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
        blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
    }
}

void setRasterizerState(D3D12_RASTERIZER_DESC &rasterizerDesc) {
    rasterizerDesc = {};

    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

void setDepthStencilState(D3D12_DEPTH_STENCIL_DESC &depthStencilDesc) {
    depthStencilDesc = {};

    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

    depthStencilDesc.StencilEnable = FALSE;
    depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
}

TestDXLayer::TestDXLayer(std::unique_ptr<Lemon::Window>& wnd) : Layer("Test DX Layer") {

    window = static_cast<Lemon::WindowsWindow*>(wnd.get());

    SDL_PropertiesID props = SDL_GetWindowProperties(window->m_Handle);
    HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    LM_CORE_ASSERT(hwnd, "Failed to retrieve HWND from SDL!");

    // Create a rendering device
    ID3D12Device* device = nullptr;
    CHECK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "Failed to create device");

    // The command queue decides which order the command lists should execute. In our case, only one command list exists.
    commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CHECK(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)), "Failed to create command queue");

    // The command allocator is used to allocate memory on the GPU for commands
    commandAllocator = nullptr;
    CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)), "Failed to create command allocator");
    CHECK(commandAllocator->Reset(), "Failed to reset command allocator");

    // The command list is used to store a list of commands we wish to execute on the GPU
    commandList = nullptr;
    CHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)), "Failed to create command list");
    CHECK(commandList->Close(), "Failed to close command list");

    // Helper factory to create the swap chain
    IDXGIFactory4* factory = nullptr;
    CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "Failed to create DXGI factory");

    // Create the swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {
        .BufferDesc = DXGI_MODE_DESC {
            .Width = wnd->GetWidth(),
            .Height = wnd->GetHeight(),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM
        },
        .SampleDesc = DXGI_SAMPLE_DESC { .Count = 1},
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .OutputWindow = hwnd,
        .Windowed = TRUE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    };

    IDXGISwapChain* tempSwapChain = nullptr;
    CHECK(factory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain), "Failed to create swap chain");

    // Cast swap chain to IDXGISwapChain3 to leverage the lastest features
    swapChain = {};
    CHECK(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)), "Failed to cast swap chain");
    tempSwapChain->Release();
    tempSwapChain = nullptr;

    // Memory descriptor heap to store render target views (RTV). Descriptor describes how to interperate resource memory
    rtvHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    CHECK(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)), "Failed to create descriptor heap");

    renderTargets[2];

    rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < 2; i++) {
            CHECK(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])), "Failed to get swapchain buffer");

            device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
            rtvHandle.ptr += rtvIncrementSize;
        }
    }

    // Fence is used to synchronize the CPU with the GPU, so they don't touch the same memory at the same time
    fence = nullptr;
    fenceValue = 0;
    CHECK(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Failed to create fence");

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    //Root signature is like have many object buffers and textures we want to use when drawing.
    //For our rotating triangle, we only need a single constant that is going to be our angle

    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameters[0].Constants.Num32BitValues = 1;
    rootParameters[0].Constants.ShaderRegister = 0;
    rootParameters[0].Constants.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    rootSignature = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters = _countof(rootParameters),
        .pParameters = rootParameters,
        .NumStaticSamplers = 0,
        .pStaticSamplers = nullptr,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    };

    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    CHECK(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob), "Failed to serialize root signature")
    CHECK(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "Failed to create root signature")

    if (signatureBlob) {
        signatureBlob->Release();
        signatureBlob = nullptr;
    }

    if (errorBlob) {
        errorBlob->Release();
        errorBlob = nullptr;
    }

    ID3DBlob* vertexShader = nullptr;
    ID3DBlob* pixelShader = nullptr;
    CHECK(D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "VSMain",
        "vs_5_0", 0, 0, &vertexShader, nullptr), "Failed to compile vertex shader");
    CHECK(D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0",
        0, 0, &pixelShader, nullptr), "Failed to compile pixel shader");

    // Pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature;

    psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();

    psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

    setBlendState(psoDesc.BlendState);

    psoDesc.SampleMask = UINT_MAX;

    setRasterizerState(psoDesc.RasterizerState);
    setDepthStencilState(psoDesc.DepthStencilState);

    psoDesc.InputLayout.pInputElementDescs = nullptr;
    psoDesc.InputLayout.NumElements = 0;

    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    pipelineState = nullptr;
    CHECK(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)), "Failed to create pipeline state");

    vertexShader->Release();
    vertexShader = nullptr;
    pixelShader->Release();
    pixelShader = nullptr;


}

TestDXLayer::~TestDXLayer() {
}

void TestDXLayer::OnUpdate() {
    static UINT triangleAngle = 0;

    CHECK(commandAllocator->Reset(), "Failed to reset command allocator");

    // Record commands to draw a triangle
    CHECK(commandList->Reset(commandAllocator, nullptr), "Failed to reset command list");

    UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[backBufferIndex];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * rtvIncrementSize;

    // Clear the render target
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Set viewport and scissor
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()), 0.0f, 1.0f};
    D3D12_RECT scissorRect = {0, 0, LONG_MAX, LONG_MAX };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->SetGraphicsRootSignature(rootSignature);
    commandList->SetPipelineState(pipelineState);

    commandList->SetGraphicsRoot32BitConstant(0, triangleAngle, 0);
    commandList->DrawInstanced(3, 1, 0, 0);

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTargets[backBufferIndex];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);
    }

    CHECK(commandList->Close(), "Failed to close command list");

    ID3D12CommandList* commandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(1, commandLists);

    CHECK(swapChain->Present(1, 0), "Failed to present swap chain");

    //Wait on the CPU for the GPU frame to finish
    const UINT64 currentFenceValue = ++fenceValue;
    CHECK(commandQueue->Signal(fence, currentFenceValue), "Failed to signal the fence");

    if (fence->GetCompletedValue() < currentFenceValue) {
        CHECK(fence->SetEventOnCompletion(currentFenceValue, fenceEvent), "Failed to set fence completion");
        WaitForSingleObject(fenceEvent, INFINITE);
    }



    triangleAngle++;


}