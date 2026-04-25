#pragma once

#include <Lemon/Layer.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Backends/DX/DXDevice.h"
#include "Backends/DX/Resources/DXBuffer.h"
#include "Backends/DX/Resources/DXSwapchain.h"
#include "Backends/DX/Resources/DXTexture.h"
#include "Lemon/Window.h"
#include "Platform/WindowsWindow.h"

#include "Backends/DX/API/DXPSO.h"
#include "Backends/DX/API/DXRootSignatureDesc.h"
#include "Backends/DX/Commands/DXCommandQueue.h"

// #include <ResourceUploadBatch.h>
// #include <DescriptorHeap.h>
// #include <DirectXHelpers.h>

namespace Lemon::DX
{
class DXPipeline;
}

class TestDXLayer : public Lemon::Layer
{
public:
    void CreateTexture(const std::shared_ptr<Lemon::DX::DXDevice>&       device,
                       const std::shared_ptr<Lemon::DX::DXCommandQueue>& graphicsQueue);

    void InitShaderPipeline(const std::shared_ptr<Lemon::DX::DXDevice>& device);

    void InitBuffers(const std::shared_ptr<Lemon::DX::DXDevice>& device);

    explicit TestDXLayer(const std::unique_ptr<Lemon::Window>& wnd);
    ~TestDXLayer() override;

    void OnUpdate() override;

private:
    Lemon::WindowsWindow* window;

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    uint32_t                                   frameIndex       = 0;
    std::array<uint64_t, MAX_FRAMES_IN_FLIGHT> frameFenceValues = {};

    std::shared_ptr<Lemon::DX::DXDevice>       device;
    std::shared_ptr<Lemon::DX::DXSwapchain>    swapchain;
    std::shared_ptr<Lemon::DX::DXPipeline>     pipeline;
    std::shared_ptr<Lemon::DX::DXCommandQueue> graphicsQueue;
    std::shared_ptr<Lemon::DX::DXVertexBuffer> vertexBuffer;
    std::shared_ptr<Lemon::DX::DXIndexBuffer>  indexBuffer;

    std::shared_ptr<Lemon::DX::DXTexture>     texture;
    std::shared_ptr<Lemon::DX::DXTextureView> textureView;
};
