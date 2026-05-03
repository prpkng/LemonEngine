#pragma once

#include "../API/Helpers.h"
#include "Lemon/Renderer/Backends/DX/DXDevice.h"
#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"
#include <Lemon/Renderer/RHI/Interfaces/ICommandList.h>
#include <Lemon/Renderer/RHI/Interfaces/ICommandQueue.h>
#include <memory>

namespace Lemon::DX
{
using namespace RHI;
class DXCommandList final : public ICommandList
{
public:

    DXCommandList(std::shared_ptr<DXDevice>         device,
                  ComPtr<ID3D12GraphicsCommandList> cmdList,
                  ComPtr<ID3D12CommandAllocator>    allocator,
                  QueueType                         queueType);

    void Begin() override;

    void End() override;
    void BindPipeline(std::shared_ptr<IPipeline> pipeline) override;
    void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
    void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) override;

    void PushConstants(ShaderStage stage, u32 slot, std::span<const std::byte> data, u32 offsetIn32BitWords) override;

    void SetPrimitiveTopology(PrimitiveTopology topology) override;

    // === Barriers ===
    void TransitionResource(void* resource, ResourceState before, ResourceState after) override;
    void TransitionTexture(ITexture* texture, ResourceState after) override;
    void TransitionTexture(ITexture* texture, ResourceState before, ResourceState after) override;

    // === Viewport / Scissor ===
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const ScissorRect& scissor) override;

    void BindTexture(u32 slot, const ITextureView* view) override;

    void SetRenderTargets(std::vector<const ITextureView*> renderTargetViews,
                          const ITextureView*              depthStencilView) override;
    using ICommandList::SetRenderTargets;

    // === Clear ===
    void ClearRenderTarget(const ITextureView* renderTarget, const std::array<float, 4>& color) override;
    void ClearDepthStencil(const ITextureView* dsv, float depth, uint8_t stencil) override;
    using ICommandList::ClearDepthStencil;

    // === Buffers ===
    void BindVertexBuffer(VertexBufferView buffer) override;
    void BindVertexBuffers(std::span<VertexBufferView> buffers) override;
    void BindIndexBuffer(IndexBufferView buffer) override;
    // -----------------------------------------------------------------------
    // Internal access for DX12CommandQueue
    // -----------------------------------------------------------------------

    [[nodiscard]] ID3D12GraphicsCommandList*     GetHandle() const noexcept { return m_CmdList.Get(); }
    [[nodiscard]] ComPtr<ID3D12CommandAllocator> ReleaseAllocator() { return std::move(m_CmdAllocator); }
    [[nodiscard]] QueueType                      GetQueueType() const override { return m_QueueType; }

private:
    void AssertRecording() const
    {
        LM_CORE_ASSERT(m_IsRecording, "Command list is not in a recodring state. Call Begin() first.")
    }
    bool TransitionTextureInternal(void* resource, RHI::ResourceState before, RHI::ResourceState after);

    std::shared_ptr<DXDevice>         m_Device;
    ComPtr<ID3D12GraphicsCommandList> m_CmdList;
    ComPtr<ID3D12CommandAllocator>    m_CmdAllocator;
    QueueType                         m_QueueType;
    bool                              m_IsRecording = false;
    bool                              m_WasClosed   = false;
};
} // namespace Lemon::DX
