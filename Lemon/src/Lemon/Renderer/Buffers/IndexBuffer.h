#pragma once
#include <lmpch.h>

#include "Buffer.h"
#include "Lemon/Renderer/RHITypes.h"


namespace Lemon::RHI
{
    class IndexBuffer
    {
    public:
        struct Desc
        {
            Buffer::Desc bufferDesc;
            uint32_t binding;
            ElementType indexType = ElementType::Uint;
        };
        explicit IndexBuffer(const Desc& desc) : m_Binding(desc.binding), m_IndexType(desc.indexType) {}
        virtual ~IndexBuffer() = default;

        virtual const std::shared_ptr<Buffer> GetBuffer() const = 0;
        ElementType GetIndexType() const { return m_IndexType; }
        uint32_t GetBinding() const { return m_Binding; };
    protected:
        uint32_t m_Binding;
        ElementType m_IndexType;
    };

}
