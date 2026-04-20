#pragma once
#include "../../Buffers/VertexBuffer.h"
#include "DXBuffer.h"

namespace Lemon::DX
{
    class DXVertexBuffer : public RHI::VertexBuffer
    {
    public:
        DXVertexBuffer(DXDevice* device, const Desc& desc);

        const std::shared_ptr<RHI::Buffer> GetBuffer() const override { return m_Buffer; }
        const D3D12_VERTEX_BUFFER_VIEW* GetBufferView() const { return &m_VertexBufferView; }
    private:
        std::shared_ptr<RHI::Buffer> m_Buffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    };
} // Lemon::DX

