#pragma once

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


    class Buffer
    {
    public:
        struct Desc
        {
            BufferUsage usage;
            MemoryUsage memoryUsage;
            size_t size;
            const void* initialData = nullptr;
            bool allowResize = false;
        };
        explicit Buffer(const Desc& desc) : m_Size(desc.size), m_MemoryUsage(desc.memoryUsage) {}
        virtual ~Buffer() = default;

        // CPU access
        virtual void* Map() = 0;
        virtual void Unmap() = 0;

        // Update (safe abstraction)
        virtual void Update(const void* data, size_t size, size_t offset = 0) = 0;


        size_t GetSize() const { return m_Size; }
        MemoryUsage GetMemoryUsage() const { return m_MemoryUsage; }
    protected:
        size_t m_Size;
        MemoryUsage m_MemoryUsage;
    };
}