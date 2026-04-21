#pragma once

#include <Lemon/Core.h>

#include "IPipeline.h"
#include "IBuffer.h"
#include <span>
#include <cstddef>

#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"

namespace Lemon::RHI
{

    /// \brief The main interface for GPU command recording
    ///
    ///\code
    /// DX12: ID3D12GraphicsCommandList  (created open, Close() before submit)
    /// Vulkan: VkCommandBuffer          (vkBeginCommandBuffer / vkEndCommandBuffer)
    ///\endcode
    ///
    ///The backing allocator (DX12) / pool (Vulkan) is managed by ICommandQueue internally. Callers never touch it
    struct ICommandList
    {
        virtual ~ICommandList() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void BindPipeline(std::shared_ptr<IPipeline> pipeline) = 0;

        virtual void Draw(
            u32 vertexCount,
            u32 instanceCount, /// Default: 1
            u32 firstVertex,   /// Default: 0
            u32 firstInstance  /// Default: 0
            ) = 0;

        virtual void DrawIndexed(
            u32 indexCount,
            u32 instanceCount, /// Default: 1
            u32 firstIndex,    /// Default: 0
            i32 vertexOffset,  /// Default: 0
            u32 firstInstance  /// Default: 0
            ) = 0;

        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;

        // === Barriers ===
        virtual void TransitionResource(void* resource,
                                        ResourceState before,
                                        ResourceState after) = 0;

        // === Viewport / Scissor ===
        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetScissor(const ScissorRect& scissor) = 0;

        // === Clear ===
        virtual void ClearRenderTarget(void* renderTarget, const std::array<float, 4>& color) = 0;

        // === Buffers ===
        virtual void BindVertexBuffer(std::shared_ptr<IVertexBuffer> buffer) = 0;
        virtual void BindIndexBuffer(std::shared_ptr<IIndexBuffer> buffer) = 0;

        // === Push constants / Root constants ===

        ///
        /// \code maps to SetGraphicsRoot32BitConstants | vkCmdPushConstants \endcode
        virtual void PushConstants(
            ShaderStage stage,
            const void* data,
            size_t dataSize,
            u32 offsetIn32BitWords
            ) = 0;

        // === BARRIERS ===

        // virtual void TransitionTexture();

        [[nodiscard]] virtual QueueType GetQueueType() const = 0;
    };

}
