#pragma once
#include "Lemon/Renderer/RHI/Types/RHITypes.h"

namespace Lemon::RHI
{

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
}
