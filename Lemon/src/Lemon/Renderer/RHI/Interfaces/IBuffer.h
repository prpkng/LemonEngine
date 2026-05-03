#pragma once
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <cstddef>
#include <span>

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
            
            Desc() {}

            // template <typename T>
            // Desc(BufferUsage usage, MemoryUsage memoryUsage, const std::vector<T>& initialData, bool allowResize = false) :
            //     usage(usage), memoryUsage(memoryUsage), size(initialData.size() * sizeof(T)), initialData(initialData.data()), allowResize(allowResize) {}
            Desc(BufferUsage usage, MemoryUsage memoryUsage, std::span<const std::byte> initialData, bool allowResize = false) :
                usage(usage), memoryUsage(memoryUsage), size(initialData.size_bytes()), initialData(initialData.data()), allowResize(allowResize) {}

            Desc(BufferUsage usage, MemoryUsage memoryUsage, size_t size, const void* initialData = nullptr, bool allowResize = false) :
                usage(usage), memoryUsage(memoryUsage), size(size), initialData(initialData), allowResize(allowResize) {}
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

    struct VertexBufferView {
        std::shared_ptr<IBuffer> buffer;
        size_t offset{};
        size_t stride{};
        size_t size{};
    };

    struct IndexBufferView {
        std::shared_ptr<IBuffer> buffer;

        size_t offset{};
        size_t size{};
        ElementType indexType{};
    };

}
