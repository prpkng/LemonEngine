//
// Created by guiej on 20/04/2026.
//

#include "DXBuffer.h"

#include "API/DXUtils.h"

namespace Lemon::DX
{
    DXBuffer::DXBuffer(const DXDevice* device, const Desc& desc) : IBuffer(desc)
    {
        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = TranslateHeapType(desc.memoryUsage);
        auto initialResourceState = TranslateInitialState(desc.memoryUsage);

        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = desc.size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        CHECK(device->GetHandle()->CreateCommittedResource(
                          &heapProps,
                          D3D12_HEAP_FLAG_NONE,
                          &resourceDesc,
                          initialResourceState,
                          nullptr,
                          IID_PPV_ARGS(&m_Handle)
                      ), "Failed to create buffer");

        if (desc.initialData)
        {
            if (m_MemoryUsage == RHI::MemoryUsage::CPU_TO_GPU)
            {
                void* mapped = DXBuffer::Map();
                memcpy(mapped, desc.initialData, desc.size);
                DXBuffer::Unmap();
            }
            else
            {
                LM_CORE_FATAL("Initial data for GPU_ONLY or GPU_TO_CPU not implemented!");
            }
        }

        m_GpuVA = GetHandle()->GetGPUVirtualAddress();
    }

    void* DXBuffer::Map()
    {
        LM_CORE_ASSERT(m_MemoryUsage != RHI::MemoryUsage::GPU_ONLY, "Cannot map GPU_ONLY buffer");

        void* data = nullptr;
        D3D12_RANGE readRange = {};
        CHECK(m_Handle->Map(0, &readRange, &data), "Failed to map buffer resource");

        return data;
    }

    void DXBuffer::Unmap()
    {
        LM_CORE_ASSERT(m_MemoryUsage != RHI::MemoryUsage::GPU_ONLY, "Cannot unmap GPU_ONLY buffer");


        D3D12_RANGE writtenRange = { 0, m_Size };
        m_Handle->Unmap(0, &writtenRange);
    }

    void DXBuffer::Update(const void* data, size_t size, size_t offset)
    {
        LM_CORE_ASSERT(m_MemoryUsage != RHI::MemoryUsage::GPU_ONLY, "Update is only valid for CPU-to-GPU buffers!");
        LM_CORE_ASSERT(offset + size <= m_Size, "Update size mismatch");

        void* mapped = Map();
        std::memcpy(static_cast<uint8_t*>(mapped) + offset, data, size);
        Unmap();
    }
}
