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

        ComPtr<ID3D12Resource> GetHandle() const {return m_Handle; }
        D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const {return m_GpuVA; }
    private:
        D3D12_GPU_VIRTUAL_ADDRESS m_GpuVA;
        ComPtr<ID3D12Resource> m_Handle;
    };




    class DXVertexBuffer : public RHI::VertexBuffer
    {
    public:
        DXVertexBuffer(std::shared_ptr<DXDevice> device, const Desc& desc);

        const std::shared_ptr<RHI::IBuffer> GetBuffer() const override { return m_Buffer; }
        const D3D12_VERTEX_BUFFER_VIEW* GetBufferView() const { return &m_VertexBufferView; }
    private:
        std::shared_ptr<RHI::IBuffer> m_Buffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    };

    class DXIndexBuffer : public RHI::IndexBuffer {
    public:
        DXIndexBuffer(std::shared_ptr<DXDevice> device, const Desc& desc);

        const std::shared_ptr<RHI::IBuffer> GetBuffer() const override { return m_Buffer; }
        const D3D12_INDEX_BUFFER_VIEW* GetBufferView() const { return &m_IndexBufferView; }
    private:
        std::shared_ptr<RHI::IBuffer> m_Buffer;
        D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
    };
}