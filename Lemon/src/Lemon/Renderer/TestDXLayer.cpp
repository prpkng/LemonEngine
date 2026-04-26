#include <cstddef>
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
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_video.h"
#include <cmath>
#include <memory>
#include <numbers>
#include <span>

#include "Backends/DX/API/Helpers.h"
#include "Backends/DX/Commands/DXCommandList.h"
#include "Backends/DX/Pipelines/DXPipeline.h"
#include "SDL3/SDL_timer.h"

#define LINALG_FORWARD_COMPATIBLE
#include <linalg.h>

using namespace linalg::aliases;

// #include <DescriptorHeap.h>
// #include <DirectXHelpers.h>
// #include <ResourceUploadBatch.h>

using namespace Lemon::RHI;
using namespace Lemon::DX;
#define PI 3.14159
struct Vertex {
    float3 position;
    float3 color;
    float2 uv;

    [[nodiscard]] std::string ToString() const
    {
        return fmt::format("pos: {0:.2f}x{1:.2f} color: R:{2} G:{3} B:{4}", position[0], position[1], color[0],
                           color[1], color[2]);
    }
};

void logHRError(HRESULT hr, std::string_view msg)
{
    auto errorMsg = HrToString(hr);
    LM_CORE_FATAL("{0}: ERROR {1}", msg, errorMsg);
}

enum Descriptors { Texture, Count };

void TestDXLayer::CreateTexture(const std::shared_ptr<DXDevice>&       device,
                                const std::shared_ptr<DXCommandQueue>& graphicsQueue)
{
    texture = std::dynamic_pointer_cast<DXTexture>(device->LoadTexture("assets/test.png"));

    textureView = std::unique_ptr<DXTextureView>(dynamic_cast<DXTextureView*>(texture->CreateSRV().release()));
}

void TestDXLayer::InitShaderPipeline(const std::shared_ptr<DXDevice>& device)
{

    IPipeline::Desc desc{};
    desc.vertexShaderPath       = "assets/shader.hlsl";
    desc.pixelShaderPath        = "assets/shader.hlsl";
    desc.renderTargetFormats    = {Format::RGBA8_UNORM};
    desc.blendState.blendEnable = true;
    desc.inputLayout            = InputLayoutBuilder()
                                      .WithElement("POSITION", ElementType::Float3)
                                      .WithElement("COLOR", ElementType::Float3)
                                      .WithElement("TEXCOORD", ElementType::Float2)
                                      .Build();
    desc.rootParameters         = {RootParameter(RootParamType::Constants, 1, 0, 0, ShaderStage::All),
                                   RootParameter(RootParamType::Constants, 4 * 4 * 3, 0, 1, ShaderStage::Vertex),
                                   RootParameter(RootParamType::Constants, 1, 1, 0, ShaderStage::Vertex),
                                   RootParameter(RootParamType::ShaderResourceView, 1, 0, 0, ShaderStage::Pixel)};

    desc.staticSamplers = {StaticSamplerDesc(Filter::Linear, AddressMode::Wrap, 0, 0, ShaderStage::Pixel)};

    desc.depthStencilState.depthEnable = true;
    desc.depthStencilState.depthWrite  = true;
    desc.depthStencilFormat            = Format::D32_FLOAT;
    pipeline                           = std::dynamic_pointer_cast<DXPipeline>(device->CreatePipeline(desc));
}

constexpr int SIDE_COUNT = 8;

void TestDXLayer::InitBuffers(const std::shared_ptr<DXDevice>& dxDevice)
{
    auto device = dxDevice->GetHandle();

    // std::vector<Vertex> vertices = {};
    // vertices.push_back({float3(0.0f, 0.0f, 0.0f), float3(0.25f, 0.25f, 0.25f), float2(0.5f, 0.5f)});
    // std::vector<uint16_t> indices = {0};
    // const float           incr    = 2 * std::numbers::pi_v<float> / SIDE_COUNT;
    // for (int i = 0; i < SIDE_COUNT; i++) {
    //     const float cos    = cosf(-incr * (float)i);
    //     const float sin    = sinf(-incr * (float)i);
    //     Vertex      vertex = {
    //         float3(cos / 1.5f, sin / 1.5f, 0.f),
    //         float3(1.0f, 1.0f, 1.0f),
    //         float2((cos + 1.0f) / 2.0f, (sin + 1.0f) / 2.0f),
    //     };
    //     vertices.push_back(vertex);
    //     // int next = i + 1;
    //     // if (next >= SIDE_COUNT)
    //     // next = 0;
    //     indices.push_back(i + 1);
    //     // indices.push_back(0);
    //     // indices.push_back(next + 1);
    // }
    // indices.push_back(1);

    std::vector<Vertex> vertices = {
        // Front face (z = +0.5)
        {float3(-0.5f,  0.5f,  0.5f), float3(1, 0, 0), float2(0, 0)},
        { float3(0.5f,  0.5f,  0.5f), float3(1, 0, 0), float2(1, 0)},
        { float3(0.5f, -0.5f,  0.5f), float3(1, 0, 0), float2(1, 1)},
        {float3(-0.5f, -0.5f,  0.5f), float3(1, 0, 0), float2(0, 1)},

        // Back face (z = -0.5)
        { float3(0.5f,  0.5f, -0.5f), float3(0, 1, 0), float2(0, 0)},
        {float3(-0.5f,  0.5f, -0.5f), float3(0, 1, 0), float2(1, 0)},
        {float3(-0.5f, -0.5f, -0.5f), float3(0, 1, 0), float2(1, 1)},
        { float3(0.5f, -0.5f, -0.5f), float3(0, 1, 0), float2(0, 1)},

        // Left face (x = -0.5)
        {float3(-0.5f,  0.5f, -0.5f), float3(0, 0, 1), float2(0, 0)},
        {float3(-0.5f,  0.5f,  0.5f), float3(0, 0, 1), float2(1, 0)},
        {float3(-0.5f, -0.5f,  0.5f), float3(0, 0, 1), float2(1, 1)},
        {float3(-0.5f, -0.5f, -0.5f), float3(0, 0, 1), float2(0, 1)},

        // Right face (x = +0.5)
        { float3(0.5f,  0.5f,  0.5f), float3(1, 1, 0), float2(0, 0)},
        { float3(0.5f,  0.5f, -0.5f), float3(1, 1, 0), float2(1, 0)},
        { float3(0.5f, -0.5f, -0.5f), float3(1, 1, 0), float2(1, 1)},
        { float3(0.5f, -0.5f,  0.5f), float3(1, 1, 0), float2(0, 1)},

        // Top face (y = +0.5)
        {float3(-0.5f,  0.5f, -0.5f), float3(1, 0, 1), float2(0, 0)},
        { float3(0.5f,  0.5f, -0.5f), float3(1, 0, 1), float2(1, 0)},
        { float3(0.5f,  0.5f,  0.5f), float3(1, 0, 1), float2(1, 1)},
        {float3(-0.5f,  0.5f,  0.5f), float3(1, 0, 1), float2(0, 1)},

        // Bottom face (y = -0.5)
        {float3(-0.5f, -0.5f,  0.5f), float3(0, 1, 1), float2(0, 0)},
        { float3(0.5f, -0.5f,  0.5f), float3(0, 1, 1), float2(1, 0)},
        { float3(0.5f, -0.5f, -0.5f), float3(0, 1, 1), float2(1, 1)},
        {float3(-0.5f, -0.5f, -0.5f), float3(0, 1, 1), float2(0, 1)},
    };

    std::vector<u16> indices = {
        0,  1,  2,  0,  2,  3,  // front
        4,  5,  6,  4,  6,  7,  // back
        8,  9,  10, 8,  10, 11, // left
        12, 13, 14, 12, 14, 15, // right
        16, 17, 18, 16, 18, 19, // top
        20, 21, 22, 20, 22, 23, // bottom
    };

    IBuffer::Desc vertexDesc(BufferUsage::Vertex, MemoryUsage::CPU_TO_GPU, std::as_bytes(std::span(vertices)));

    auto layout = VertexLayoutBuilder()
                      .WithElement("POSITION", ElementType::Float3)
                      .WithElement("COLOR", ElementType::Float3)
                      .WithElement("TEXCOORD", ElementType::Float2)
                      .Build();

    vertexBuffer = std::dynamic_pointer_cast<DXVertexBuffer>(dxDevice->CreateVertexBuffer(vertexDesc, layout));

    IBuffer::Desc indexDesc(BufferUsage::Index, MemoryUsage::CPU_TO_GPU, std::as_bytes(std::span(indices)));

    indexBuffer = std::dynamic_pointer_cast<DXIndexBuffer>(dxDevice->CreateIndexBuffer(indexDesc, ElementType::Ushort));
}

TestDXLayer::TestDXLayer(const std::unique_ptr<Lemon::Window>& wnd) : Layer("Test DX Layer")
{

    window = dynamic_cast<Lemon::WindowsWindow*>(wnd.get());

    const SDL_PropertiesID props = SDL_GetWindowProperties(window->m_Handle);
    const auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    LM_CORE_ASSERT(hwnd, "Failed to retrieve HWND from SDL!");

    DXDevice::Desc desc{};
    desc.enableDebugLayer = true;
    desc.initialWidth     = window->GetWidth();
    desc.initialHeight    = window->GetHeight();
    desc.nativeWindowPtr  = hwnd;

    device = std::make_shared<DXDevice>(desc);

    graphicsQueue = std::dynamic_pointer_cast<DXCommandQueue>(device->GetDefaultGraphicsQueue());

    auto swapChainDesc = ISwapchain::Desc{.windowHandle = hwnd,
                                          .width        = window->GetWidth(),
                                          .height       = window->GetHeight(),
                                          .bufferCount  = 2,
                                          .format       = Format::RGBA8_UNORM};
    swapchain          = std::dynamic_pointer_cast<DXSwapchain>(device->CreateSwapchain(graphicsQueue, swapChainDesc));

    ITexture::Desc depthDesc{};
    depthDesc.width          = window->GetWidth();
    depthDesc.height         = window->GetHeight();
    depthDesc.format         = Format::D32_FLOAT;
    depthDesc.isDepthStencil = true;

    depthTexture = std::dynamic_pointer_cast<DXTexture>(device->CreateTexture(depthDesc));
    depthTextureView =
        std::shared_ptr<DXTextureView>(dynamic_cast<DXTextureView*>(depthTexture->CreateDSV().release()));

    InitShaderPipeline(device);

    InitBuffers(device);

    texture = std::dynamic_pointer_cast<DXTexture>(device->LoadTexture("assets/test.png"));

    textureView = std::unique_ptr<DXTextureView>(dynamic_cast<DXTextureView*>(texture->CreateSRV().release()));
}

float4 from_euler(float pitch, float yaw, float roll)
{
    float4 qx = linalg::rotation_quat(float3(1, 0, 0), fmodf(pitch, PI*2));
    float4 qy = linalg::rotation_quat(float3(0, 1, 0), fmodf(yaw, PI*2));
    float4 qz = linalg::rotation_quat(float3(0, 0, 1), fmodf(roll, PI*2));

    // Apply in order: roll first, then pitch, then yaw (YXZ order)
    return linalg::qmul(qy, linalg::qmul(qx, qz));
}

TestDXLayer::~TestDXLayer() = default;

void TestDXLayer::OnUpdate()
{
    static UINT   triangleAngle = 0;
    static UINT   triangleColor = 0;
    static float lastTime;
    const float   time          = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    static float2 lastMouse, mousePos, rot{0, -PI/6};

    float dt = lastTime - time;
    lastTime = time;

    u32    buttons    = SDL_GetMouseState(&mousePos.x, &mousePos.y);
    float2 deltaMouse = lastMouse - mousePos;
    lastMouse         = mousePos;

    if (buttons & SDL_BUTTON_LMASK) {
        rot.x -= deltaMouse.x / 10.0f * (PI / 180.0f);
        rot.y += deltaMouse.y / 10.0f * (PI / 180.0f);
    }


    // --- Throttle: wait on the OLDEST frame slot before reusing it ---
    // On frames 0 and 1 this value is 0, so CpuWaitForValue returns
    // immediately. From frame 2 onwards, this blocks only if the GPU is still
    // on that slot.
    const uint64_t waitValue = frameFenceValues[frameIndex];
    if (waitValue > 0)
        graphicsQueue->CpuWaitForValue(waitValue);

    auto cmdList = graphicsQueue->GetCommandList();
    cmdList->Begin();

    const UINT backBufferIndex = swapchain->AcquireNextBackbuffer();

    // Transition the backBuffer to the render target state
    cmdList->TransitionTexture(swapchain->GetBackbuffer(backBufferIndex).get(), ResourceState::RenderTarget);

    // Clear the render target
    cmdList->ClearRenderTarget(swapchain->GetBackbufferView(backBufferIndex), {0.0f, 0.2f, 0.4f, 1.0f});

    cmdList->ClearDepthStencil(depthTextureView.get(), 1.0f, 0);

    // Set viewport and scissor
    cmdList->SetViewport(
        {0.0f, 0.0f, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()), 0.0f, 1.0f});
    cmdList->SetScissor({0, 0, LONG_MAX, LONG_MAX});

    cmdList->SetRenderTargets({swapchain->GetBackbufferView(backBufferIndex)}, depthTextureView.get());

    cmdList->SetPrimitiveTopology(PrimitiveTopology::TriangleList);
    cmdList->BindPipeline(pipeline);

    cmdList->BindTexture(3, textureView.get());

    float4x4 modelMatrix =
        linalg::pose_matrix(linalg::rotation_quat(float3(0, 1, 0), time/2), float3(0, 0, 0));

    float3 cameraPos = float3(0, 2, -4);
    float4 cameraRot = from_euler(-rot.y, rot.x, 0.0);
    // float3 cameraPos = float3(4, 0, 0);

    float3 forward = linalg::qzdir(cameraRot);
    float4x4 viewMatrix = linalg::lookat_matrix(cameraPos, cameraPos + forward, float3(0, 1, 0), linalg::pos_z);
    float4x4 projMatrix =
        linalg::perspective_matrix(3.141524f / 4.0f, 16.f / 9.f, .01f, 100.f, linalg::pos_z, linalg::zero_to_one);
    std::vector<float4x4> matrices = {modelMatrix, viewMatrix, projMatrix};
    cmdList->PushConstants(ShaderStage::Vertex, 0, std::as_bytes(std::span(&time, 1)), 0);
    cmdList->PushConstants(ShaderStage::Vertex, 1, std::as_bytes(std::span(matrices)), 0);

    cmdList->PushConstants(ShaderStage::Vertex, 2, std::as_bytes(std::span(&triangleAngle, 1)), 0);
    cmdList->BindVertexBuffer(vertexBuffer);
    cmdList->BindIndexBuffer(indexBuffer);
    cmdList->DrawIndexed(6 * 2 * 3, 1, 0, 0, 0);

    // Transition the backBuffer to the present state
    cmdList->TransitionTexture(swapchain->GetBackbuffer(backBufferIndex).get(), ResourceState::Present);

    cmdList->End();

    const u64 frameDone = graphicsQueue->SubmitSingle(*cmdList);

    swapchain->Present(1);
    swapchain->ResetBackbufferStates();

    // Wait on the CPU for the GPU frame to finish
    graphicsQueue->CpuWaitForValue(frameDone - 1);

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    triangleAngle += 2;
    triangleColor = (triangleColor + 1) % 255;
}
