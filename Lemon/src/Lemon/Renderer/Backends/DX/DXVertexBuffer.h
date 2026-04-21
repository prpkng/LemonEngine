#pragma once

#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "DXBuffer.h"

namespace Lemon::DX
{
    class DXVertexBuffer : public RHI::VertexBuffer
    {
    public:
        DXVertexBuffer(DXDevice* device, const Desc& desc);

        const std::shared_ptr<RHI::IBuffer> GetBuffer() const override { return m_Buffer; }
        const D3D12_VERTEX_BUFFER_VIEW* GetBufferView() const { return &m_VertexBufferView; }
    private:
        std::shared_ptr<RHI::IBuffer> m_Buffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    };
} // Lemon::DX

