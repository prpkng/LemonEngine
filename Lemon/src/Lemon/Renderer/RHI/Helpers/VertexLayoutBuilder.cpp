#include "VertexLayoutBuilder.h"

namespace Lemon::RHI
{
    VertexLayoutBuilder::VertexLayoutBuilder() : m_AccumulatedOffset(0), m_TotalStride(0)
    {
    }

    VertexLayoutBuilder& VertexLayoutBuilder::WithElement(const std::string& name, ElementType type,
                                                          const bool addToAccumulation)
    {
        m_Elements.emplace_back(VertexElement {name, type, m_AccumulatedOffset});
        const size_t size = GetVertexElementSize(type);
        if (addToAccumulation) m_AccumulatedOffset += size;
        m_TotalStride += size;
        return *this;
    }

    VertexLayout VertexLayoutBuilder::Build(const InputRate inputRate)
    {
        return VertexLayout{
            std::move(m_Elements),
            m_TotalStride,
            inputRate
        };
    }
}
