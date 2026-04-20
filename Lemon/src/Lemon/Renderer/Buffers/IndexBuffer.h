#pragma once
#include <lmpch.h>

#include "Buffer.h"


namespace Lemon::RHI
{
    class IndexBuffer
    {
    public:
        struct Desc
        {
            Buffer::Desc bufferDesc;
            uint32_t binding;
            VertexElementType indexType = VertexElementType::Uint;
        };
        explicit IndexBuffer(const Desc& desc) : m_Binding(desc.binding), m_IndexType(desc.indexType) {}
        virtual ~IndexBuffer() = default;

        virtual const std::shared_ptr<Buffer> GetBuffer() const = 0;
        VertexElementType GetIndexType() const { return m_IndexType; }
        uint32_t GetBinding() const { return m_Binding; };
    protected:
        uint32_t m_Binding;
        VertexElementType m_IndexType;
    };

}
