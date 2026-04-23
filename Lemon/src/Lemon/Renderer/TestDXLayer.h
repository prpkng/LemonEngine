#pragma once

#include <Lemon/Layer.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Lemon/Window.h"
#include "Platform/WindowsWindow.h"
#include "Backends/DX/DXDevice.h"
#include "Backends/DX/Resources/DXBuffer.h"
#include "Backends/DX/Resources/DXSwapchain.h"

#include "Backends/DX/API/DXPSO.h"
#include "Backends/DX/API/DXRootSignatureDesc.h"
#include "Backends/DX/Commands/DXCommandQueue.h"

namespace Lemon::DX
{
    class DXPipeline;
}

class TestDXLayer : public Lemon::Layer {
public:

    void InitShaderPipeline(const std::shared_ptr<Lemon::DX::DXDevice>& device);

    void InitBuffers(const std::shared_ptr<Lemon::DX::DXDevice>& device);

    explicit TestDXLayer(const std::unique_ptr<Lemon::Window>& wnd);
    ~TestDXLayer() override;

    void OnUpdate() override;

private:
    Lemon::WindowsWindow* window;

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    uint32_t frameIndex = 0;
    std::array<uint64_t, MAX_FRAMES_IN_FLIGHT> frameFenceValues = {};

    std::shared_ptr<Lemon::DX::DXSwapchain> swapchain;

    std::shared_ptr<Lemon::DX::DXPipeline> pipeline;

    std::shared_ptr<Lemon::DX::DXCommandQueue> graphicsQueue;

    std::shared_ptr<Lemon::DX::DXVertexBuffer> vertexBuffer;

    std::shared_ptr<Lemon::DX::DXIndexBuffer> indexBuffer;
};



