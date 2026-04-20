#pragma once
#include <lmpch.h>

#include "DXDevice.h"
#include "Lemon/Renderer/Buffers/IndexBuffer.h"

namespace Lemon::DX
{
    class DXIndexBuffer : public RHI::IndexBuffer {
    public:
        DXIndexBuffer(DXDevice* device, const Desc& desc);

        const std::shared_ptr<RHI::Buffer> GetBuffer() const override { return m_Buffer; }
        const D3D12_INDEX_BUFFER_VIEW* GetBufferView() const { return &m_IndexBufferView; }
    private:
        std::shared_ptr<RHI::Buffer> m_Buffer;
        D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
    };
}
