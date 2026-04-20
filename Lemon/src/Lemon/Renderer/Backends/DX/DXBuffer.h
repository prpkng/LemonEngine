#pragma once
#include "../../Buffers/Buffer.h"
#include "DXDevice.h"

namespace Lemon::DX
{
    class DXBuffer : public RHI::Buffer
    {
    public:
        DXBuffer(const DXDevice* device, const Desc& desc);

        void* Map() override;
        void Unmap() override;

        void Update(const void* data, size_t size, size_t offset) override;

        ComPtr<ID3D12Resource> GetHandle() const {return m_Handle; }
        D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const {return m_GpuVA; }
    private:
        D3D12_GPU_VIRTUAL_ADDRESS m_GpuVA;
        ComPtr<ID3D12Resource> m_Handle;
    };
}

inline D3D12_HEAP_TYPE TranslateHeapType(Lemon::RHI::MemoryUsage usage)
{
    switch (usage)
    {
    case Lemon::RHI::MemoryUsage::GPU_ONLY:   return D3D12_HEAP_TYPE_DEFAULT;
    case Lemon::RHI::MemoryUsage::CPU_To_GPU: return D3D12_HEAP_TYPE_UPLOAD;
    case Lemon::RHI::MemoryUsage::GPU_To_CPU: return D3D12_HEAP_TYPE_READBACK;
    }
    return D3D12_HEAP_TYPE_DEFAULT;
}

inline D3D12_RESOURCE_STATES TranslateInitialState(Lemon::RHI::MemoryUsage usage)
{
    switch (usage)
    {
    case Lemon::RHI::MemoryUsage::GPU_ONLY:   return D3D12_RESOURCE_STATE_COMMON;
    case Lemon::RHI::MemoryUsage::CPU_To_GPU: return D3D12_RESOURCE_STATE_GENERIC_READ;
    case Lemon::RHI::MemoryUsage::GPU_To_CPU: return D3D12_RESOURCE_STATE_COPY_DEST;
    }
    return D3D12_RESOURCE_STATE_COMMON;
}
