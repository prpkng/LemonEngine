//
// Created by guiej on 20/04/2026.
//

#include "DXIndexBuffer.h"

#include "API/DXUtils.h"
#include "DXBuffer.h"

namespace Lemon::DX
{
    DXIndexBuffer::DXIndexBuffer(DXDevice* device, const Desc& desc) : IndexBuffer(desc)
    {
        m_Buffer = device->CreateBuffer(desc.bufferDesc);

        auto bufferAsDX = std::dynamic_pointer_cast<DXBuffer>(m_Buffer);
        m_IndexBufferView = {};
        m_IndexBufferView.BufferLocation = bufferAsDX->GetVirtualAddress();
        m_IndexBufferView.SizeInBytes = m_Buffer->GetSize();
        m_IndexBufferView.Format = TranslateElementTypeToFormat(m_IndexType);
    }
}
