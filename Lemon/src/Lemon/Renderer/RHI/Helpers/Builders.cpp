#include "Builders.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <vector>

namespace Lemon::RHI
{
VertexLayoutBuilder::VertexLayoutBuilder() : m_AccumulatedOffset(0), m_TotalStride(0) {}

VertexLayoutBuilder& VertexLayoutBuilder::WithElement(Semantic semantic, ElementType type, const bool addToAccumulation)
{
    m_Elements.emplace_back(VertexElement{semantic, type, m_AccumulatedOffset});
    const size_t size = GetVertexElementSize(type);
    if (addToAccumulation)
        m_AccumulatedOffset += size;
    m_TotalStride += size;
    return *this;
}

VertexLayout VertexLayoutBuilder::Build(const InputRate inputRate)
{
    return VertexLayout{std::move(m_Elements), m_TotalStride, inputRate};
}

InputLayoutBuilder::InputLayoutBuilder() : m_ElementCount(0), m_AccumulatedOffset(0) {}

InputLayoutBuilder& InputLayoutBuilder::WithElement(Semantic semantic, ElementType type, u32 binding, const InputRate inputRate)
{
    auto attribute = VertexAttribute{
        .semantic  = semantic,
        .format    = type,
        .binding   = binding,
        .offset    = m_AccumulatedOffset,
        .inputRate = inputRate,
        .location  = m_ElementCount,
    };
    attribute.BuildSemanticName();
    
    m_Attributes.emplace_back(attribute);
    const size_t size = GetVertexElementSize(type);
    m_AccumulatedOffset += size;
    m_ElementCount++;
    return *this;
}

std::vector<VertexAttribute> InputLayoutBuilder::Build() { return std::move(m_Attributes); }

u32 GetVertexElementSize(const ElementType type)
{
    switch (type) {
    // ===== FLOAT =====
    case ElementType::Float:
        return 4;
    case ElementType::Float2:
        return 8;
    case ElementType::Float3:
        return 12;
    case ElementType::Float4:
        return 16;

    // ===== SIGNED INT =====
    case ElementType::Int:
        return 4;
    case ElementType::Int2:
        return 8;
    case ElementType::Int3:
        return 12;
    case ElementType::Int4:
        return 16;

    // ===== UNSIGNED INT =====
    case ElementType::Uint:
        return 4;
    case ElementType::Uint2:
        return 8;
    case ElementType::Uint3:
        return 12;
    case ElementType::Uint4:
        return 16;

    // ===== 8-bit (4 components) =====
    case ElementType::Byte4:
    case ElementType::Byte4N:
    case ElementType::Ubyte4:
    case ElementType::Ubyte4N:
        return 4; // 4 * 1 byte

    // ===== 16-bit (2 components) =====
    case ElementType::Short2:
    case ElementType::Short2N:
        return 4; // 2 * 2 bytes

    // ===== 16-bit (4 components) =====
    case ElementType::Short4:
    case ElementType::Short4N:
        return 8; // 4 * 2 bytes

    default:
        assert(false && "Unknown ElementType");
        return 0;
    }
}
} // namespace Lemon::RHI
