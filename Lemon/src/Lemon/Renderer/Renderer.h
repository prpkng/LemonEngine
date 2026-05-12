#pragma once

#include "Backends/DX/Resources/DXTexture.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include "Lemon/Window.h"
#include "RHI/Interfaces/ICommandQueue.h"
#include "RHI/Interfaces/ISwapchain.h"
#include "Resources/ResourceManager.h"
#include "Resources/ResourcePool.h"
#include <Lemon/Core.h>
#include "RHI/Interfaces/IDevice.h"

namespace Lemon
{

class Renderer
{
public:
    LEMON_API static Renderer& Instance();

    void Init(const std::unique_ptr<Window>& wnd, RHI::IDevice::Desc deviceDesc);

    [[nodiscard]] ResourceManager& GetResources() { return m_ResourceManager; }

    [[nodiscard]] std::shared_ptr<RHI::IDevice> GetDevice() const { return m_Device; }
    [[nodiscard]] std::shared_ptr<RHI::ISwapchain> GetSwapchain() const { return swapchain; }
    [[nodiscard]] std::shared_ptr<RHI::ICommandQueue> GetGraphicsQueue() const { return graphicsQueue; }



    // === DEBUG ====
    [[nodiscard]] std::string GetStats() const;
    void PrintStats() const;
private:
    std::shared_ptr<RHI::IDevice> m_Device;
    std::shared_ptr<RHI::ISwapchain>    swapchain;
    std::shared_ptr<RHI::ICommandQueue> graphicsQueue;

    ResourceManager m_ResourceManager;
};

} // namespace Lemon