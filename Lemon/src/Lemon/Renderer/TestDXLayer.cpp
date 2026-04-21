#include "TestDXLayer.h"
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include "Backends/DX/DXBuffer.h"
#include "Backends/DX/DXDevice.h"
#include "Backends/DX/API/DXUtils.h"
#include "Helpers/VertexLayoutBuilder.h"
#include "Platform/WindowsWindow.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_video.h"
#include <cmath>
#include <numbers>

#include "Backends/DX/API/DXPSO.h"


using namespace Lemon::RHI;
using namespace Lemon::DX;

struct Vertex {
    float position[2];
    float color[3];


     std::string ToString() const {
        return fmt::format("pos: {0:.2f}x{1:.2f} color: R:{2} G:{3} B:{4}", position[0], position[1],
            color[0], color[1], color[2]);
    }
};

void logHRError(HRESULT hr, std::string_view msg) {
    auto errorMsg = HrToString(hr);
    LM_CORE_FATAL("{0}: ERROR {1}", msg, errorMsg);
}

void TestDXLayer::InitCommandQueue(ComPtr<ID3D12Device> device) {
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
}

void TestDXLayer::InitSwapchain(HWND hwnd) {
    // Helper factory to create the swap chain
    IDXGIFactory4* factory = nullptr;
    CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "Failed to create DXGI factory");

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
    CHECK(factory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain), "Failed to create swap chain");

    // Cast swap chain to IDXGISwapChain3 to leverage the lastest features
    swapChain = {};
    CHECK(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)), "Failed to cast swap chain");
    tempSwapChain->Release();
    tempSwapChain = nullptr;
}

void TestDXLayer::InitRenderTargets(ComPtr<ID3D12Device> device) {
    // Memory descriptor heap to store render target views (RTV). Descriptor describes how to interperate resource memory
    rtvHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    CHECK(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)), "Failed to create descriptor heap");

    rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < 2; i++) {
            CHECK(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])), "Failed to get swapchain buffer");

            device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
            rtvHandle.ptr += rtvIncrementSize;
        }
    }
}

void TestDXLayer::InitSync(ComPtr<ID3D12Device> device) {
    // Fence is used to synchronize the CPU with the GPU, so they don't touch the same memory at the same time
    fence = nullptr;
    fenceValue = 0;
    CHECK(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Failed to create fence");

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void TestDXLayer::InitShaderPipeline(ComPtr<ID3D12Device> device) {
    // //Root signature is like have many object buffers and textures we want to use when drawing.
    // //For our rotating triangle, we only need a single constant that is going to be our angle
    // IPipeline::Desc desc{};
    // desc.vertexShaderPath = "shader.hlsl";
    // desc.pixelShaderPath = "shader.hlsl";
    // desc.renderTargetFormats = { Format::RGBA8_UNORM };
    // desc.inputLayout = {
    //     VertexAttribute { "POSITION", 0, ElementType::Float2, 0, 0, InputRate::PerVertex, 0 },
    //     VertexAttribute { "COLOR",    0, ElementType::Float3, 0, 8, InputRate::PerVertex, 0 }
    // };
    // desc.rootParameters = {
    //     RootParameter { RootParamType::Constants, 1, 0, 0, ShaderStage::Vertex }
    // };


    DXShader vertexShader(L"shader.hlsl", ShaderStage::Vertex);
    DXShader pixelShader(L"shader.hlsl", ShaderStage::Pixel);

    DXRootSignatureDesc rootDesc;
    rootDesc.AddConstant(1, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    DXGraphicsPSODesc psoDesc;
    psoDesc.SetVertexShader(vertexShader.GetBytecode(), vertexShader.GetLength())
           .SetPixelShader(pixelShader.GetBytecode(), pixelShader.GetLength())
           .SetInputLayout(std::move(inputLayout))
           .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

    pso = DXPipelineStateObject::Create(device, rootDesc, psoDesc);
}

constexpr int SIDE_COUNT = 6;

void TestDXLayer::InitBuffers(const std::unique_ptr<DXDevice>& dxDevice) {
    auto device = dxDevice->GetHandle();

    std::vector<Vertex> vertices ={};
    vertices.push_back({{0.0f, 0.0f}, {0.25f, 0.25f, 0.25f}});
    std::vector<uint16_t> indices = {};
    const float incr = 2 * std::numbers::pi_v<float> / SIDE_COUNT;
    for (int i = 0; i < SIDE_COUNT; i++)
    {
        Vertex vertex = {
            { cosf(incr * i)/1.5f, sinf(incr * i)/1.5f },
            {1.0f, 1.0f, 1.0f}
        };
        vertices.push_back(vertex);
        int next = i+1;
        if (next >= SIDE_COUNT) next = 0;
        indices.push_back(i+1);
        indices.push_back(0);
        indices.push_back(next+1);
    }

    Buffer::Desc desc{};
    desc.memoryUsage = MemoryUsage::CPU_To_GPU;
    desc.initialData = vertices.data();
    desc.size = vertices.size() * sizeof(Vertex);
    desc.usage = BufferUsage::Vertex;

    VertexBuffer::Desc vertexDesc {};
    vertexDesc.bufferDesc = desc;
    vertexDesc.layout = VertexLayoutBuilder()
                    .WithElement("POSITION", ElementType::Float2)
                    .WithElement("COLOR", ElementType::Float3)
                    .Build();

    vertexBuffer = std::dynamic_pointer_cast<DXVertexBuffer>(dxDevice->CreateVertexBuffer(vertexDesc));

    desc.initialData = indices.data();
    desc.size = indices.size() * sizeof(uint16_t);
    desc.usage = BufferUsage::Index;

    IndexBuffer::Desc indexDesc = {
        .bufferDesc = desc,
        .indexType = ElementType::Ushort,
    };

    auto idxBuffer = dxDevice->CreateBuffer(desc);
    indexBuffer = std::dynamic_pointer_cast<DXIndexBuffer>(dxDevice->CreateIndexBuffer(indexDesc));
}

TestDXLayer::TestDXLayer(std::unique_ptr<Lemon::Window>& wnd) : Layer("Test DX Layer") {

    window = static_cast<Lemon::WindowsWindow*>(wnd.get());

    SDL_PropertiesID props = SDL_GetWindowProperties(window->m_Handle);
    HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    LM_CORE_ASSERT(hwnd, "Failed to retrieve HWND from SDL!");

    DXDevice::Desc desc {};
    desc.enableDebugLayer = true;
    desc.initialWidth = window->GetWidth();
    desc.initialHeight = window->GetHeight();
    desc.nativeWindowPtr = hwnd;
    const auto device = std::make_unique<DXDevice>(desc);

    InitCommandQueue(device->m_Handle);

    InitSwapchain(hwnd);

    InitRenderTargets(device->m_Handle);

    InitSync(device->m_Handle);

    InitShaderPipeline(device->m_Handle);

    InitBuffers(device);
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
    commandList->SetGraphicsRootSignature(pso.GetRootSignature().Get());
    commandList->SetPipelineState(pso.GetPSO().Get());

    commandList->SetGraphicsRoot32BitConstant(0, triangleAngle, 0);
    commandList->IASetVertexBuffers(0, 1, vertexBuffer->GetBufferView());
    commandList->IASetIndexBuffer(indexBuffer->GetBufferView());
    commandList->DrawIndexedInstanced(SIDE_COUNT*3, 1, 0, 0, 0);

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