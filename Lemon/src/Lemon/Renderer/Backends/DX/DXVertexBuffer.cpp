//
// Created by guiej on 20/04/2026.
//

#include "DXVertexBuffer.h"

namespace Lemon
{
    namespace DX
    {
        DXVertexBuffer::DXVertexBuffer(DXDevice* device, const Desc& desc) : VertexBuffer(desc)
        {
            m_Buffer = device->CreateBuffer(desc.bufferDesc);

            auto bufferAsDX = std::dynamic_pointer_cast<DXBuffer>(m_Buffer);
            m_VertexBufferView = {};
            m_VertexBufferView.BufferLocation = bufferAsDX->GetVirtualAddress();
            m_VertexBufferView.SizeInBytes = m_Buffer->GetSize();
            m_VertexBufferView.StrideInBytes = desc.layout.stride;
        }
    } // DX
} // Lemon