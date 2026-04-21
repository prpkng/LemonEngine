#pragma once
#include <lmpch.h>

#include "Buffer.h"
#include <Lemon/Renderer/RHITypes.h>

namespace Lemon::RHI
{

    struct VertexElement
    {
        std::string name;
        ElementType type;
        uint32_t offset;
    };

    struct VertexLayout
    {
        std::vector<VertexElement> elements;
        uint32_t stride;
        InputRate inputRate = InputRate::PerVertex;
    };

    class VertexBuffer
    {
    public:
        struct Desc
        {
            Buffer::Desc bufferDesc;
            VertexLayout layout;
            uint32_t binding;
        };
        explicit VertexBuffer(const Desc& desc) : m_Layout(desc.layout), m_Binding(desc.binding) {}
        virtual ~VertexBuffer() = default;

        virtual const std::shared_ptr<Buffer> GetBuffer() const = 0;
        const VertexLayout& GetLayout() const { return m_Layout; };
        const uint32_t GetBinding() const { return m_Binding; };
    private:
        VertexLayout m_Layout;
        uint32_t m_Binding;

    };


    inline uint32_t GetVertexElementSize(ElementType type)
    {
        switch (type)
        {
            // ===== FLOAT =====
        case ElementType::Float:  return 4;
        case ElementType::Float2: return 8;
        case ElementType::Float3: return 12;
        case ElementType::Float4: return 16;

            // ===== SIGNED INT =====
        case ElementType::Int:  return 4;
        case ElementType::Int2: return 8;
        case ElementType::Int3: return 12;
        case ElementType::Int4: return 16;

            // ===== UNSIGNED INT =====
        case ElementType::Uint:  return 4;
        case ElementType::Uint2: return 8;
        case ElementType::Uint3: return 12;
        case ElementType::Uint4: return 16;

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


}
