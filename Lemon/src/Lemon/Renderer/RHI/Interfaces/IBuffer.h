#pragma once
#include "Lemon/Renderer/RHI/Types/RHITypes.h"

namespace Lemon::RHI
{

    enum class BufferUsage
    {
        Vertex,
        Index,
        Uniform,
        Storage,
        Indirect,
        Staging
    };

    enum class MemoryUsage
    {
        GPU_ONLY,
        CPU_To_GPU,
        GPU_To_CPU
    };


    struct IBuffer
    {
        struct Desc
        {
            BufferUsage usage{};
            MemoryUsage memoryUsage{};
            size_t size{};
            const void* initialData = nullptr;
            bool allowResize = false;
        };
        explicit IBuffer(const Desc& desc) : m_Size(desc.size), m_MemoryUsage(desc.memoryUsage) {}
        virtual ~IBuffer() = default;

        // CPU access
        virtual void* Map() = 0;
        virtual void Unmap() = 0;

        // Update (safe abstraction)
        virtual void Update(const void* data, size_t size, size_t offset = 0) = 0;


        [[nodiscard]] constexpr size_t GetSize() const { return m_Size; }
        [[nodiscard]] constexpr MemoryUsage GetMemoryUsage() const { return m_MemoryUsage; }
    protected:
        size_t m_Size;
        MemoryUsage m_MemoryUsage;
    };

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

    struct VertexBuffer
    {
        struct Desc
        {
            IBuffer::Desc bufferDesc{};
            VertexLayout layout{};
            u32 binding{};
        };
        explicit VertexBuffer(const Desc& desc) : m_Layout(desc.layout), m_Binding(desc.binding) {}
        virtual ~VertexBuffer() = default;

        virtual const std::shared_ptr<IBuffer> GetBuffer() const = 0;
        [[nodiscard]] const VertexLayout& GetLayout() const { return m_Layout; };
        [[nodiscard]] constexpr u32 GetBinding() const { return m_Binding; };
    private:
        VertexLayout m_Layout;
        uint32_t m_Binding;

    };

    struct IndexBuffer
    {
        struct Desc
        {
            IBuffer::Desc bufferDesc{};
            u32 binding{};
            ElementType indexType = ElementType::Uint;
        };
        explicit IndexBuffer(const Desc& desc) : m_Binding(desc.binding), m_IndexType(desc.indexType) {}
        virtual ~IndexBuffer() = default;

        virtual const std::shared_ptr<IBuffer> GetBuffer() const = 0;
        [[nodiscard]] constexpr ElementType GetIndexType() const { return m_IndexType; }
        [[nodiscard]] constexpr u32 GetBinding() const { return m_Binding; };
    protected:
        u32 m_Binding;
        ElementType m_IndexType;
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
