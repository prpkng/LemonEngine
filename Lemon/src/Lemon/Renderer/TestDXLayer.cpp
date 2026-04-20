#include "TestDXLayer.h"
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include "Platform/WindowsWindow.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_video.h"

struct Vertex {
    float position[2];
    float color[3];
};

void logHRError(HRESULT hr, std::string_view msg) {
    auto errorMsg = HrToString(hr);
    LM_CORE_FATAL("{0}: ERROR {1}", msg, errorMsg);
}

ID3D12Device * TestDXLayer::InitDevice() {
    // Create a rendering device
    ID3D12Device* device = nullptr;
    ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "Failed to create device");
    return device;
}

void TestDXLayer::InitCommandQueue(ID3D12Device *device) {
    // The command queue decides which order the command lists should execute. In our case, only one command list exists.
    commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)), "Failed to create command queue");

    // The command allocator is used to allocate memory on the GPU for commands
    commandAllocator = nullptr;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)), "Failed to create command allocator");
    ThrowIfFailed(commandAllocator->Reset(), "Failed to reset command allocator");

    // The command list is used to store a list of commands we wish to execute on the GPU
    commandList = nullptr;
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)), "Failed to create command list");
    ThrowIfFailed(commandList->Close(), "Failed to close command list");
}

void TestDXLayer::InitSwapchain(HWND hwnd) {
    // Helper factory to create the swap chain
    IDXGIFactory4* factory = nullptr;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "Failed to create DXGI factory");

    // Create the swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {
        .BufferDesc = DXGI_MODE_DESC {
            .Width = window->GetWidth(),
            .Height = window->GetHeight(),
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
    ThrowIfFailed(factory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain), "Failed to create swap chain");

    // Cast swap chain to IDXGISwapChain3 to leverage the lastest features
    swapChain = {};
    ThrowIfFailed(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)), "Failed to cast swap chain");
    tempSwapChain->Release();
    tempSwapChain = nullptr;
}

void TestDXLayer::InitRenderTargets(ID3D12Device *device) {
    // Memory descriptor heap to store render target views (RTV). Descriptor describes how to interperate resource memory
    rtvHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)), "Failed to create descriptor heap");

    rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < 2; i++) {
            ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])), "Failed to get swapchain buffer");

            device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
            rtvHandle.ptr += rtvIncrementSize;
        }
    }
}

void TestDXLayer::InitSync(ID3D12Device *device) {
    // Fence is used to synchronize the CPU with the GPU, so they don't touch the same memory at the same time
    fence = nullptr;
    fenceValue = 0;
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Failed to create fence");

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void TestDXLayer::InitShaderPipeline(ID3D12Device *device) {
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
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob), "Failed to serialize root signature");
    ThrowIfFailed(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "Failed to create root signature");

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
    ThrowIfFailed(D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "VSMain",
                                     "vs_5_0", 0, 0, &vertexShader, nullptr), "Failed to compile vertex shader");
    ThrowIfFailed(D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0",
                                     0, 0, &pixelShader, nullptr), "Failed to compile pixel shader");

    // Pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature;

    psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();

    psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

    psoDesc.SampleMask = UINT_MAX;

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState      = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    static D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);

    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    pipelineState = nullptr;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)), "Failed to create pipeline state");

    vertexShader->Release();
    vertexShader = nullptr;
    pixelShader->Release();
    pixelShader = nullptr;
}

void TestDXLayer::InitBuffers(ID3D12Device *device) {
    static Vertex quadVertices[] = {
        { { -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } }, // v0 (top-left)
        { {  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } }, // v1 (top-right)
        { { -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } }, // v2 (bottom-left)
        { {  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f } }  // v3 (bottom-right)
    };
    static uint16_t indices[] = {
        0, 1, 2,   // first triangle
        2, 1, 3    // second triangle
    };

    vertexBuffer = nullptr;
    constexpr UINT vertexBufferSize = sizeof(quadVertices);

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = vertexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ThrowIfFailed(device->CreateCommittedResource(
                      &heapProps,
                      D3D12_HEAP_FLAG_NONE,
                      &resourceDesc,
                      D3D12_RESOURCE_STATE_GENERIC_READ,
                      nullptr,
                      IID_PPV_ARGS(&vertexBuffer)
                  ), "Failed to create vertex buffer");

    void* pData = nullptr;
    D3D12_RANGE readRange = {0, 0}; // We won't read from it

    vertexBuffer->Map(0, &readRange, &pData);
    memcpy(pData, quadVertices, sizeof(quadVertices));
    vertexBuffer->Unmap(0, nullptr);

    vertexBufferView = {};
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = vertexBufferSize;
    vertexBufferView.StrideInBytes = sizeof(Vertex);


    indexBuffer = nullptr;

    const UINT indexBufferSize = sizeof(indices);

    heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = indexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ThrowIfFailed(device->CreateCommittedResource(
                      &heapProps,
                      D3D12_HEAP_FLAG_NONE,
                      &resourceDesc,
                      D3D12_RESOURCE_STATE_GENERIC_READ,
                      nullptr,
                      IID_PPV_ARGS(&indexBuffer)
                  ), "Failed to create index buffer");

    pData = nullptr;
    readRange = {0, 0};

    indexBuffer->Map(0, &readRange, &pData);
    memcpy(pData, indices, sizeof(indices));
    indexBuffer->Unmap(0, nullptr);

    indexBufferView = {};
    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = indexBufferSize;
    indexBufferView.Format = DXGI_FORMAT_R16_UINT; // because uint16_t
}

TestDXLayer::TestDXLayer(std::unique_ptr<Lemon::Window>& wnd) : Layer("Test DX Layer") {

    window = static_cast<Lemon::WindowsWindow*>(wnd.get());

    SDL_PropertiesID props = SDL_GetWindowProperties(window->m_Handle);
    HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    LM_CORE_ASSERT(hwnd, "Failed to retrieve HWND from SDL!");

    ID3D12Device *device = InitDevice();

    InitCommandQueue(device);

    InitSwapchain(hwnd);

    InitRenderTargets(device);

    InitSync(device);

    InitShaderPipeline(device);

    InitBuffers(device);
}

TestDXLayer::~TestDXLayer() {
}

void TestDXLayer::OnUpdate() {
    static UINT triangleAngle = 0;

    ThrowIfFailed(commandAllocator->Reset(), "Failed to reset command allocator");

    // Record commands to draw a triangle
    ThrowIfFailed(commandList->Reset(commandAllocator, nullptr), "Failed to reset command list");

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
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetIndexBuffer(&indexBufferView);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

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

    ThrowIfFailed(commandList->Close(), "Failed to close command list");

    ID3D12CommandList* commandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(1, commandLists);

    ThrowIfFailed(swapChain->Present(1, 0), "Failed to present swap chain");

    //Wait on the CPU for the GPU frame to finish
    const UINT64 currentFenceValue = ++fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence, currentFenceValue), "Failed to signal the fence");

    if (fence->GetCompletedValue() < currentFenceValue) {
        ThrowIfFailed(fence->SetEventOnCompletion(currentFenceValue, fenceEvent), "Failed to set fence completion");
        WaitForSingleObject(fenceEvent, INFINITE);
    }



    triangleAngle++;


}