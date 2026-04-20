#pragma once
#include <lmpch.h>

#include "Buffer.h"


namespace Lemon::RHI
{

    enum class VertexElementType
    {
        Float, Float2, Float3, Float4,
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Byte4, Byte4N, Ubyte4, Ubyte4N,
        Short2, Short2N,
        Short4, Short4N,
    };

    enum class InputRate
    {
        PerVertex,
        PerInstance
    };

    struct VertexElement
    {
        std::string name;
        VertexElementType type;
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


    inline uint32_t GetVertexElementSize(VertexElementType type)
    {
        switch (type)
        {
            // ===== FLOAT =====
        case VertexElementType::Float:  return 4;
        case VertexElementType::Float2: return 8;
        case VertexElementType::Float3: return 12;
        case VertexElementType::Float4: return 16;

            // ===== SIGNED INT =====
        case VertexElementType::Int:  return 4;
        case VertexElementType::Int2: return 8;
        case VertexElementType::Int3: return 12;
        case VertexElementType::Int4: return 16;

            // ===== UNSIGNED INT =====
        case VertexElementType::Uint:  return 4;
        case VertexElementType::Uint2: return 8;
        case VertexElementType::Uint3: return 12;
        case VertexElementType::Uint4: return 16;

            // ===== 8-bit (4 components) =====
        case VertexElementType::Byte4:
        case VertexElementType::Byte4N:
        case VertexElementType::Ubyte4:
        case VertexElementType::Ubyte4N:
            return 4; // 4 * 1 byte

            // ===== 16-bit (2 components) =====
        case VertexElementType::Short2:
        case VertexElementType::Short2N:
            return 4; // 2 * 2 bytes

            // ===== 16-bit (4 components) =====
        case VertexElementType::Short4:
        case VertexElementType::Short4N:
            return 8; // 4 * 2 bytes

        default:
            assert(false && "Unknown VertexElementType");
            return 0;
        }
    }


}
