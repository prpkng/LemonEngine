#pragma once

#include "Renderer.h"
#include "Lemon/Renderer/Backends/DX/DXDevice.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include "Platform/WindowsWindow.h"
#include "RHI/Interfaces/ISwapchain.h"
#include <utility>

namespace Lemon
{

Renderer& Renderer::Instance()
{
    static Renderer instance{};

    return instance;
}

void Renderer::Init(const std::unique_ptr<Window>& wnd, RHI::IDevice::Desc deviceDesc)
{
    auto                   window = dynamic_cast<Lemon::WindowsWindow*>(wnd.get());
    const SDL_PropertiesID props  = SDL_GetWindowProperties(window->m_Handle);
    const auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    LM_CORE_ASSERT(hwnd, "Failed to retrieve HWND from SDL!");

    LM_INFO("Initializing renderer");
    deviceDesc.nativeWindowPtr = hwnd;
    m_Device                   = std::make_shared<DX::DXDevice>(deviceDesc);

    graphicsQueue = m_Device->GetDefaultGraphicsQueue();

    auto swapChainDesc = RHI::ISwapchain::Desc{.windowHandle = hwnd,
                                               .width        = window->GetWidth(),
                                               .height       = window->GetHeight(),
                                               .bufferCount  = 2,
                                               .format       = RHI::Format::RGBA8_UNORM};
    swapchain          = m_Device->CreateSwapchain(graphicsQueue, swapChainDesc);
}

TextureHandle Renderer::CreateTexture(const RHI::ITexture::Desc& desc) {
    // auto handle = m_TextureDescPool.alloc(desc);

    DX::DXTexture texture = *reinterpret_cast<DX::DXTexture*>(m_Device->CreateTexture(desc));
    return m_TextureNativePool.insertOrDie(std::move(texture));
}

TextureHandle Renderer::LoadTexture(std::string_view path, RHI::Format format, u32 mipLevels) {
    // auto handle = m_TextureDescPool.alloc(path, format, mipLevels);
    DX::DXTexture texture = *reinterpret_cast<DX::DXTexture*>(m_Device->LoadTexture(path, format, mipLevels));
    return m_TextureNativePool.insertOrDie(std::move(texture));
}

std::string Renderer::GetStats() const {
    std::stringstream ss;
    ss << "\n=== RESOURCE POOLS ===\n";
    m_TextureNativePool.forEach([&ss](TextureHandle handle, const DX::DXTexture* texture) {
        ss << fmt::format("Texture [{0}]: \"{1}\" {2}x{3}x{4}\n", (u32)handle.index, texture->GetDesc().debugName, texture->GetDesc().width, texture->GetDesc().height, texture->GetDesc().arraySize);
    });

    return ss.str();
}

void Renderer::PrintStats() const {
    LM_INFO("{}", GetStats());
}

} // namespace Lemon