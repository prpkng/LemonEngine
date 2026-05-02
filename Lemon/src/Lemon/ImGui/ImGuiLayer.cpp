#include "ImGuiLayer.h"
#include "Lemon/Renderer/Backends/DX/Commands/DXCommandList.h"
#include "Lemon/Renderer/Backends/DX/Commands/DXCommandQueue.h"
#include "Lemon/Renderer/Backends/DX/DXDevice.h"
#include "Lemon/Renderer/Backends/DX/Resources/DXSwapchain.h"
#include "Lemon/Renderer/Renderer.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_sdl3.h"
#include "imgui.h"
namespace Lemon
{

ImGuiLayer::ImGuiLayer() {}

ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach()
{

    auto device        = std::dynamic_pointer_cast<DX::DXDevice>(Renderer::Instance().GetDevice());
    auto graphicsQueue = std::dynamic_pointer_cast<DX::DXCommandQueue>(Renderer::Instance().GetGraphicsQueue());
    auto swapchain     = std::dynamic_pointer_cast<DX::DXSwapchain>(Renderer::Instance().GetSwapchain());

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    [[maybe_unused]] ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // ImGui_ImplSDL3_InitForD3D(window->m_Handle);

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device                  = device->m_Handle.Get();
    initInfo.CommandQueue            = graphicsQueue->GetHandle().Get();
    initInfo.NumFramesInFlight       = 2;
    initInfo.RTVFormat               = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat               = DXGI_FORMAT_UNKNOWN;
    initInfo.UserData                = Renderer::Instance().GetDevice().get();

    auto allocation = device->m_SrvHeap->Allocate();

    initInfo.SrvDescriptorHeap            = device->m_SrvHeap->GetHeap();
    initInfo.LegacySingleSrvCpuDescriptor = allocation.cpu;
    initInfo.LegacySingleSrvGpuDescriptor = allocation.gpu;

    ImGui_ImplDX12_Init(&initInfo);

    ImGui::StyleColorsDark();
}

void ImGuiLayer::OnDetach()
{
    ImGui_ImplDX12_Shutdown();
    // ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::OnPreUpdate()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::OnUpdate() {
    ImGui::ShowDemoWindow();
}

void ImGuiLayer::OnPostUpdate() {
    ImGui::Render();
}

void ImGuiLayer::OnPreRender() {}

void ImGuiLayer::OnRender() {
    // ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), static_cast<DX::DXCommandList*>(cmdList.get())->GetHandle());
}

void ImGuiLayer::OnPostRender() {
    
}

void ImGuiLayer::OnEvent(Event& e) {}

} // namespace Lemon