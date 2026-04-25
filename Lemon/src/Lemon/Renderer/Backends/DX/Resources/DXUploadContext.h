#pragma once

#include "../API/Helpers.h"
#include "Lemon/Renderer/Backends/DX/DXDevice.h"
#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "Lemon/Renderer/RHI/Interfaces/ICommandQueue.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"
#include "d3d12.h"
#include <Lemon/Renderer/RHI/Interfaces/IUploadContext.h>
#include <memory>

struct PendingUpload {
    ComPtr<ID3D12Resource> stagingBuffer; // kept alive until GPU is done
};

namespace Lemon::DX
{

class DXUploadContext final : public RHI::IUploadContext
{
public:
    DXUploadContext(std::shared_ptr<DXDevice> device);

    void UploadTexture(RHI::ITexture& dest, std::span<const std::byte> pixels, u32 srcRowPitch) override;

    void UploadBuffer(RHI::IBuffer& dest, std::span<const std::byte> data) override;

    void              Flush() override;
    [[nodiscard]] u64 FlushAsync() override;

    [[nodiscard]] bool IsComplete(u64 fenceValue) const override;

    [[nodiscard]] u64 GetPendingStagingBytes() const override;

private:
    std::shared_ptr<DXDevice>           m_Device;
    std::shared_ptr<RHI::ICommandQueue> m_CopyQueue;
    std::shared_ptr<RHI::ICommandQueue> m_GraphicsQueue;
    std::shared_ptr<RHI::ICommandList>  m_CmdList;
    std::vector<PendingUpload>          m_PendingUploads;
    std::vector<RHI::ITexture*>             m_PendingTransitions;
    u64                                 m_PendingStagingBytes = 0;
    u64                                 m_LastFenceValue      = 0;
};

} // namespace Lemon::DX