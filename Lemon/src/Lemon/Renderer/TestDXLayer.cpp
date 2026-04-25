#include <lmpch.h>

#include "TestDXLayer.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

#include "Backends/DX/API/Helpers.h"
#include "Backends/DX/Commands/DXCommandQueue.h"
#include "Backends/DX/DXDevice.h"
#include "Backends/DX/Resources/DXBuffer.h"
#include "Backends/DX/Resources/DXTexture.h"
#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"
#include "Platform/WindowsWindow.h"
#include "RHI/Helpers/Builders.h"
#include "RHI/Interfaces/ICommandList.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_video.h"
#include <cmath>
#include <memory>
#include <numbers>

#include "Backends/DX/API/Helpers.h"
#include "Backends/DX/Commands/DXCommandList.h"
#include "Backends/DX/Pipelines/DXPipeline.h"
#include "SDL3/SDL_timer.h"
#include "d3dx12_barriers.h"
#include "d3dx12_core.h"
#include "d3dx12_resource_helpers.h"
#include "d3dx12_root_signature.h"
#include "dxgiformat.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #include <DescriptorHeap.h>
// #include <DirectXHelpers.h>
// #include <ResourceUploadBatch.h>

using namespace Lemon::RHI;
using namespace Lemon::DX;

struct Vertex {
    float position[2];
    float color[3];
    float uv[2];

    [[nodiscard]] std::string ToString() const {
        return fmt::format("pos: {0:.2f}x{1:.2f} color: R:{2} G:{3} B:{4}", position[0], position[1], color[0],
                           color[1], color[2]);
    }
};

void logHRError(HRESULT hr, std::string_view msg) {
    auto errorMsg = HrToString(hr);
    LM_CORE_FATAL("{0}: ERROR {1}", msg, errorMsg);
}

enum Descriptors { Texture, Count };

void TestDXLayer::CreateTexture(const std::shared_ptr<DXDevice>&       device,
                                const std::shared_ptr<DXCommandQueue>& graphicsQueue) {
    ITexture::Desc texDesc(1024, 1024, Format::RGBA8_UNORM);

    texture = std::dynamic_pointer_cast<DXTexture>(device->CreateTexture(texDesc));


    int       width, height, channels;
    const u8* data = stbi_load("assets/test.png", &width, &height, &channels, 4);

    LM_CORE_ASSERT(data != nullptr, "Failed to load texture image!");

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = data;
    textureData.RowPitch = width * channels;
    textureData.SlicePitch = textureData.RowPitch * height;

    ComPtr<ID3D12Resource> uploadBuffer;
    {
        const CD3DX12_HEAP_PROPERTIES heapProps{D3D12_HEAP_TYPE_UPLOAD};
        const auto                    uploadBufferSize = GetRequiredIntermediateSize(texture->GetResource(), 0, 1);

        const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        CHECK(device->GetHandle()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                           D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                           IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())),
              "Failed to create upload buffer for texture");
    }

    auto cmdList = graphicsQueue->GetCommandList().release();

    auto* dxCmdList = dynamic_cast<DXCommandList*>(cmdList);
    dxCmdList->Begin();

    // write commands to copy data to upload texture (copying each subresource)
    UpdateSubresources(dxCmdList->GetHandle(), texture->GetResource(), uploadBuffer.Get(), 0, 0,
                       1, // Count
                       &textureData);

    // write commands to transition texture to texture state
    cmdList->TransitionResource(texture->GetResource(), ResourceState::CopyDest, ResourceState::ShaderResource);

    dxCmdList->End();

    const u64 frameDone = graphicsQueue->SubmitSingle(*cmdList);

    // Wait for the GPU to finish executing the command list
    graphicsQueue->CpuWaitForValue(frameDone);

    textureView = std::unique_ptr<DXTextureView>(dynamic_cast<DXTextureView*>(texture->CreateSRV().release()));
}

void TestDXLayer::InitShaderPipeline(const std::shared_ptr<DXDevice>& device) {

    IPipeline::Desc desc{};
    desc.vertexShaderPath = "assets/shader.hlsl";
    desc.pixelShaderPath = "assets/shader.hlsl";
    desc.renderTargetFormats = {Format::RGBA8_UNORM};
    desc.blendState.blendEnable = true;
    desc.inputLayout = InputLayoutBuilder()
                           .WithElement("POSITION", ElementType::Float2)
                           .WithElement("COLOR", ElementType::Float3)
                           .WithElement("TEXCOORD", ElementType::Float2)
                           .Build();
    desc.rootParameters = {RootParameter(RootParamType::Constants, 1, 0, 0, ShaderStage::All),
                           RootParameter(RootParamType::Constants, 1, 1, 0, ShaderStage::Vertex),
                           RootParameter(RootParamType::ShaderResourceView, 1, 0, 0, ShaderStage::Pixel)};

    desc.staticSamplers = {StaticSamplerDesc(Filter::Linear, AddressMode::Wrap, 0, 0, ShaderStage::Pixel)};

    pipeline = std::dynamic_pointer_cast<DXPipeline>(device->CreatePipeline(desc));
}

constexpr int SIDE_COUNT = 8;

void TestDXLayer::InitBuffers(const std::shared_ptr<DXDevice>& dxDevice) {
    auto device = dxDevice->GetHandle();

    std::vector<Vertex> vertices = {};
    vertices.push_back({
        {0.0f, 0.0f},
        {0.25f, 0.25f, 0.25f},
        {0.5f, 0.5f}
    });
    std::vector<uint16_t> indices = {0};
    const float           incr = 2 * std::numbers::pi_v<float> / SIDE_COUNT;
    for (int i = 0; i < SIDE_COUNT; i++) {
        const float cos = cosf(-incr * (float)i);
        const float sin = sinf(-incr * (float)i);
        Vertex vertex = {
            {cos / 1.5f, sin / 1.5f},
            {1.0f, 1.0f, 1.0f},
            {(cos + 1.0f) / 2.0f, (sin + 1.0f) / 2.0f}
        };
        vertices.push_back(vertex);
        // int next = i + 1;
        // if (next >= SIDE_COUNT)
            // next = 0;
        indices.push_back(i + 1);
        // indices.push_back(0);
        // indices.push_back(next + 1);
    }
    indices.push_back(1);

    // std::vector<Vertex> vertices = {
    //     { {-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f }},
    //     {  {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f }},
    //     {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
    //     { {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f }},
    // };

    // std::vector<u16> indices = {0, 1, 2, 1, 3, 2};


    IBuffer::Desc vertexDesc(BufferUsage::Vertex, MemoryUsage::CPU_TO_GPU, vertices);

    auto layout = VertexLayoutBuilder()
                      .WithElement("POSITION", ElementType::Float2)
                      .WithElement("COLOR", ElementType::Float3)
                      .WithElement("TEXCOORD", ElementType::Float2)
                      .Build();

    vertexBuffer = std::dynamic_pointer_cast<DXVertexBuffer>(dxDevice->CreateVertexBuffer(vertexDesc, layout));

    IBuffer::Desc indexDesc(BufferUsage::Index, MemoryUsage::CPU_TO_GPU, indices);

    indexBuffer = std::dynamic_pointer_cast<DXIndexBuffer>(dxDevice->CreateIndexBuffer(indexDesc, ElementType::Ushort));
}

TestDXLayer::TestDXLayer(const std::unique_ptr<Lemon::Window>& wnd) : Layer("Test DX Layer") {

    window = dynamic_cast<Lemon::WindowsWindow*>(wnd.get());

    const SDL_PropertiesID props = SDL_GetWindowProperties(window->m_Handle);
    const auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    LM_CORE_ASSERT(hwnd, "Failed to retrieve HWND from SDL!");

    DXDevice::Desc desc{};
    desc.enableDebugLayer = true;
    desc.initialWidth = window->GetWidth();
    desc.initialHeight = window->GetHeight();
    desc.nativeWindowPtr = hwnd;
    
    device = std::make_shared<DXDevice>(desc);

    graphicsQueue = std::dynamic_pointer_cast<DXCommandQueue>(device->CreateCommandQueue(QueueType::Graphics));

    auto swapChainDesc = ISwapchain::Desc{.windowHandle = hwnd,
                                          .width = window->GetWidth(),
                                          .height = window->GetHeight(),
                                          .bufferCount = 2,
                                          .format = Format::RGBA8_UNORM};
    swapchain = std::dynamic_pointer_cast<DXSwapchain>(device->CreateSwapchain(graphicsQueue, swapChainDesc));

    InitShaderPipeline(device);

    InitBuffers(device);

    CreateTexture(device, graphicsQueue);
}

TestDXLayer::~TestDXLayer() = default;

void TestDXLayer::OnUpdate() {
    static UINT triangleAngle = 0;
    static UINT triangleColor = 0;
    const float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;

    // --- Throttle: wait on the OLDEST frame slot before reusing it ---
    // On frames 0 and 1 this value is 0, so CpuWaitForValue returns
    // immediately. From frame 2 onwards, this blocks only if the GPU is still
    // on that slot.
    const uint64_t waitValue = frameFenceValues[frameIndex];
    if (waitValue > 0)
        graphicsQueue->CpuWaitForValue(waitValue);

    auto                 cmdList = graphicsQueue->GetCommandList().release();
    const DXCommandList* dxCmdList = dynamic_cast<DXCommandList*>(cmdList);
    cmdList->Begin();

    const UINT backBufferIndex = swapchain->AcquireNextBackbuffer();

    // Transition the backBuffer to the render target state
    cmdList->TransitionResource(swapchain->GetBackbuffer(backBufferIndex), ResourceState::Present,
                                ResourceState::RenderTarget);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapchain->GetBackbufferView(backBufferIndex);

    // Clear the render target
    cmdList->ClearRenderTarget(&rtvHandle, {0.0f, 0.2f, 0.4f, 1.0f});

    // Set viewport and scissor
    cmdList->SetViewport(
        {0.0f, 0.0f, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()), 0.0f, 0.0f});
    cmdList->SetScissor({0, 0, LONG_MAX, LONG_MAX});

    dxCmdList->GetHandle()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    ID3D12DescriptorHeap* heaps[] = { device->m_SrvHeap->GetHeap() };
    dxCmdList->GetHandle()->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

    cmdList->SetPrimitiveTopology(PrimitiveTopology::TriangleFan);
    cmdList->BindPipeline(pipeline);

    dxCmdList->GetHandle()->SetGraphicsRootDescriptorTable(2, textureView->GetGPUHandle());

    cmdList->PushConstants(ShaderStage::Vertex, 0, &time, 4, 0);
    cmdList->PushConstants(ShaderStage::Vertex, 1, &triangleAngle, 4, 0);
    cmdList->BindVertexBuffer(vertexBuffer);
    cmdList->BindIndexBuffer(indexBuffer);
    cmdList->DrawIndexed(SIDE_COUNT + 2, 1, 0, 0, 0);

    // Transition the backBuffer to the present state
    cmdList->TransitionResource(swapchain->GetBackbuffer(backBufferIndex), ResourceState::RenderTarget,
                                ResourceState::Present);

    cmdList->End();

    const u64 frameDone = graphicsQueue->SubmitSingle(*cmdList);

    swapchain->Present(1);

    // Wait on the CPU for the GPU frame to finish
    graphicsQueue->CpuWaitForValue(frameDone - 1);

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    triangleAngle += 2;
    triangleColor = (triangleColor + 1) % 255;

    delete cmdList;
}
