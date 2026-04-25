#pragma once

#include "../API/Helpers.h"
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
    DXUploadContext(ComPtr<ID3D12Device> device, std::shared_ptr<RHI::ICommandQueue> queue);

    void UploadTexture(RHI::ITexture& dest, std::span<const std::byte> pixels, u32 srcRowPitch) override;

    void UploadBuffer(RHI::IBuffer& dest, std::span<const std::byte> data) override;

    void              Flush() override;
    [[nodiscard]] u64 FlushAsync() override;

    [[nodiscard]] bool IsComplete(u64 fenceValue) const override;

    [[nodiscard]] u64 GetPendingStagingBytes() const override;

private:
    ComPtr<ID3D12Device>                m_Device;
    std::shared_ptr<RHI::ICommandQueue> m_Queue;
    std::shared_ptr<RHI::ICommandList>  m_CmdList;
    std::vector<PendingUpload>          m_PendingUploads;
    u64                                 m_PendingStagingBytes = 0;
    u64                                 m_LastFenceValue      = 0;
};

} // namespace Lemon::DX