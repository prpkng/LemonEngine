#pragma once
#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "../DXDevice.h"

namespace Lemon::DX
{
    class DXBuffer : public RHI::IBuffer
    {
    public:
        DXBuffer(std::shared_ptr<DXDevice> device, const Desc& desc);

        void* Map() override;
        void Unmap() override;

        void Update(const void* data, size_t size, size_t offset) override;

        [[nodiscard]] ComPtr<ID3D12Resource> GetHandle() const { return m_Handle; }
        [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const { return m_GpuVA; }

        

    private:
        D3D12_GPU_VIRTUAL_ADDRESS m_GpuVA;
        ComPtr<ID3D12Resource> m_Handle;
    };
}
