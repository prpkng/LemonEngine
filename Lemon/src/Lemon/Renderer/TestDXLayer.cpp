#include <cstddef>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <lmpch.h>

#include "TestDXLayer.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_sdl3.h>

#include "Backends/DX/API/Helpers.h"
#include "Backends/DX/Commands/DXCommandQueue.h"
#include "Backends/DX/DXDevice.h"
#include "Backends/DX/Resources/DXBuffer.h"
#include "Backends/DX/Resources/DXTexture.h"
#include "Lemon/Renderer/RHI/Helpers/Builders.h"
#include "Lemon/Renderer/RHI/Interfaces/IDevice.h"
#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"
#include "MeshLoader.h"
#include "Platform/WindowsWindow.h"
#include "RHI/Helpers/Builders.h"
#include "RHI/Interfaces/IBuffer.h"
#include "RHI/Interfaces/ICommandList.h"
#include "Renderer.h"
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
#include "dxgiformat.h"
#include "imgui.h"
#include "magic_enum/magic_enum.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define LINALG_FORWARD_COMPATIBLE
#include <linalg.h>

using namespace linalg::aliases;

// #include <DescriptorHeap.h>
// #include <DirectXHelpers.h>
// #include <ResourceUploadBatch.h>

using namespace Lemon;
using namespace Lemon::RHI;
using namespace Lemon::DX;
#define PI 3.14159

void TestDXLayer::InitShaderPipeline(const std::shared_ptr<DXDevice>& device)
{
    
    IPipeline::Desc desc{};
    desc.vertexShaderPath       = "assets/shader.hlsl";
    desc.pixelShaderPath        = "assets/shader.hlsl";
    desc.renderTargetFormats    = {Format::RGBA8_UNORM};
    desc.blendState.blendEnable = true;
    desc.inputLayout            = InputLayoutBuilder()
                                      .WithElement(Semantic::Position, ElementType::Float3)
                                      .WithElement(Semantic::Normal, ElementType::Float3)
                                      .WithElement(Semantic::TexCoord0, ElementType::Float2)
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

static size_t indexCount = 0;


void TestDXLayer::InitBuffers(const std::shared_ptr<DXDevice>& dxDevice)
{
    LM_INFO("Loading mesh");
    auto mesh  = loadMeshes("assets/monkey.fbx")[0];
    

    LM_INFO("Mesh loaded!");
    indexCount = mesh.indices.size();
    u32 i = 0;
    for (auto (attr) : mesh.attributes) {
        IBuffer::Desc vertexDesc(BufferUsage::Vertex, MemoryUsage::CPU_TO_GPU, std::as_bytes(std::span(attr.second.data)));
        
        auto buffer = dxDevice->CreateBuffer(vertexDesc);
        
        vertexBuffers[i] = VertexBufferView {
        buffer,
            0,
            GetVertexElementSize(attr.second.format),
            buffer->GetSize()
        };
        i++;
    }

    // IBuffer::Desc vertexDesc(BufferUsage::Vertex, MemoryUsage::CPU_TO_GPU, std::as_bytes(std::span(mesh.vertices)));

    // auto layout = VertexLayoutBuilder()
    //                   .WithElement("POSITION", ElementType::Float3)
    //                   .WithElement("NORMAL", ElementType::Float3)
    //                   .WithElement("TEXCOORD", ElementType::Float2)
    //                   .Build();

    // vertexBuffer = std::dynamic_pointer_cast<DXVertexBuffer>(dxDevice->CreateVertexBuffer(vertexDesc, layout));

    IBuffer::Desc indexDesc(BufferUsage::Index, MemoryUsage::CPU_TO_GPU, std::as_bytes(std::span(mesh.indices)));

    auto buffer = dxDevice->CreateBuffer(indexDesc);

    indexBuffer = IndexBufferView {
        buffer,
        0,
        buffer->GetSize(),
        ElementType::Uint
    };
    
}

TestDXLayer::TestDXLayer(const std::unique_ptr<Lemon::Window>& wnd) : Layer("Test DX Layer")
{
    window = dynamic_cast<Lemon::WindowsWindow*>(wnd.get());

    IDevice::Desc desc{};
    desc.enableDebugLayer = true;
    desc.initialWidth     = window->GetWidth();
    desc.initialHeight    = window->GetHeight();
    Renderer::Instance().Init(wnd, desc);

    device          = std::dynamic_pointer_cast<DXDevice>(Renderer::Instance().GetDevice());
    graphicsQueue   = std::dynamic_pointer_cast<DXCommandQueue>(Renderer::Instance().GetGraphicsQueue());
    swapchain       = std::dynamic_pointer_cast<DXSwapchain>(Renderer::Instance().GetSwapchain());

    ITexture::Desc depthDesc{};
    depthDesc.width          = window->GetWidth();
    depthDesc.height         = window->GetHeight();
    depthDesc.format         = Format::D32_FLOAT;
    depthDesc.isDepthStencil = true;
    depthDesc.debugName      = "DepthBuffer";

    depthTextureHandle = Renderer::Instance().CreateTexture(depthDesc);
    auto depthTexture = Renderer::Instance().GetNativeTexture(depthTextureHandle);
    depthTextureView =
        std::shared_ptr<DXTextureView>(dynamic_cast<DXTextureView*>(depthTexture->CreateDSV().release()));

    InitShaderPipeline(device);

    InitBuffers(device);

    
    textureHandle = Renderer::Instance().LoadTexture("assets/test.png");
    // texture = dynamic_cast<DXTexture*>(device->LoadTexture("assets/test.png"));

    auto texture = Renderer::Instance().GetNativeTexture(textureHandle);

    textureView = std::unique_ptr<DXTextureView>(dynamic_cast<DXTextureView*>(texture->CreateSRV().release()));

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    [[maybe_unused]] ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplSDL3_InitForD3D(window->m_Handle);

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device                  = device->m_Handle.Get();
    initInfo.CommandQueue            = graphicsQueue->GetHandle().Get();
    initInfo.NumFramesInFlight       = MAX_FRAMES_IN_FLIGHT;
    initInfo.RTVFormat               = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat               = DXGI_FORMAT_UNKNOWN;
    initInfo.UserData = device.get();
    // Allocating srv descriptors (for textures) is up to the application
    // (curent version of the imgui backend will only allocate one descriptor)


    auto allocation = device->m_SrvHeap->Allocate();

    initInfo.SrvDescriptorHeap    = device->m_SrvHeap->GetHeap();
    initInfo.LegacySingleSrvCpuDescriptor = allocation.cpu;
    initInfo.LegacySingleSrvGpuDescriptor = allocation.gpu;
    // initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
    //                                    D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
    //     auto dx = reinterpret_cast<DXDevice*>(info->UserData);
    //     auto allocation = dx->m_SrvHeap->Allocate();
    //     out_cpu_handle = &allocation.cpu;
    //     out_gpu_handle = &allocation.gpu;
    // };
    // initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
    //                                   D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
    //     // return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle);
    // };

    ImGui_ImplDX12_Init(&initInfo);

    ImGui::StyleColorsDark();

    Renderer::Instance().PrintStats();
}

float4 from_euler(float pitch, float yaw, float roll)
{
    float4 qx = linalg::rotation_quat(float3(1, 0, 0), fmodf(pitch, PI * 2));
    float4 qy = linalg::rotation_quat(float3(0, 1, 0), fmodf(yaw, PI * 2));
    float4 qz = linalg::rotation_quat(float3(0, 0, 1), fmodf(roll, PI * 2));

    // Apply in order: roll first, then pitch, then yaw (YXZ order)
    return linalg::qmul(qy, linalg::qmul(qx, qz));
}

TestDXLayer::~TestDXLayer() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void TestDXLayer::OnUpdate()
{
    static UINT   triangleAngle = 0;
    static UINT   triangleColor = 0;
    static float  lastTime;
    const float   time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    static float2 lastMouse, mousePos, rot{0, -PI / 6};

    float dt = lastTime - time;
    lastTime = time;

    u32    buttons    = SDL_GetMouseState(&mousePos.x, &mousePos.y);
    float2 deltaMouse = lastMouse - mousePos;
    lastMouse         = mousePos;

    if (buttons & SDL_BUTTON_LMASK) {
        rot.x -= deltaMouse.x / 10.0f * (PI / 180.0f);
        rot.y += deltaMouse.y / 10.0f * (PI / 180.0f);
    }

    // == IMGUI ==

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();


    // ===========


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

    float4x4 modelMatrix = linalg::pose_matrix(linalg::rotation_quat(float3(0, 1, 0), time / 2), float3(0, 0, 0));

    float3 cameraPos = float3(0, 2, -4);
    float4 cameraRot = from_euler(-rot.y, rot.x, 0.0);
    // float3 cameraPos = float3(4, 0, 0);

    float3   forward    = linalg::qzdir(cameraRot);
    float4x4 viewMatrix = linalg::lookat_matrix(cameraPos, cameraPos + forward, float3(0, 1, 0), linalg::pos_z);
    float4x4 projMatrix =
        linalg::perspective_matrix(3.141524f / 4.0f, 16.f / 9.f, .01f, 100.f, linalg::pos_z, linalg::zero_to_one);
    std::vector<float4x4> matrices = {modelMatrix, viewMatrix, projMatrix};
    cmdList->PushConstants(ShaderStage::Vertex, 0, std::as_bytes(std::span(&time, 1)), 0);
    cmdList->PushConstants(ShaderStage::Vertex, 1, std::as_bytes(std::span(matrices)), 0);

    cmdList->PushConstants(ShaderStage::Vertex, 2, std::as_bytes(std::span(&triangleAngle, 1)), 0);
    cmdList->BindVertexBuffers(vertexBuffers);
    // cmdList->BindVertexBuffer(vertexBuffers[1]);
    // cmdList->BindVertexBuffer(vertexBuffers[2]);
    cmdList->BindIndexBuffer(indexBuffer);

    cmdList->DrawIndexed(indexCount, 1, 0, 0, 0);


    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), static_cast<DXCommandList*>(cmdList.get())->GetHandle());
    
    // Transition the backBuffer to the present state
    cmdList->TransitionTexture(swapchain->GetBackbuffer(backBufferIndex).get(), ResourceState::Present);

    cmdList->End();

    frameFenceValues[frameIndex] = graphicsQueue->SubmitSingle(*cmdList);

    swapchain->Present(1);
    swapchain->ResetBackbufferStates();

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    triangleAngle += 2;
    triangleColor = (triangleColor + 1) % 255;
}
