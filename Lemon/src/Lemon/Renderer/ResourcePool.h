#pragma once

#include <Lemon/Core.h>
#include <concepts>
#include <cstddef>
#include <expected>
#include <memory>
#include <source_location>
#include <type_traits>

namespace Lemon
{

template <typename Tag> struct ResourceHandle {
    u32 index : 20;
    u32 version : 11;
    u32 valid : 1; // explicit null operator

    [[nodiscard]] bool isNull() const noexcept { return !valid; }
    [[nodiscard]] bool isValid() const noexcept { return valid; }

    auto operator<=>(const ResourceHandle&) const = default;

    [[nodiscard]] static constexpr ResourceHandle null() noexcept { return {0, 0, 0}; }
};

struct TextureTag {
};
struct BufferTag {
};
struct PipelineTag {
};

using TextureHandle  = ResourceHandle<TextureTag>;
using BufferHandle   = ResourceHandle<BufferTag>;
using PipelineHandle = ResourceHandle<PipelineTag>;

static_assert(std::is_trivially_copyable_v<TextureHandle>);
static_assert(sizeof(TextureHandle) == 4);

template <typename T> struct alignas(T) ResourceSlot {
    std::byte data[sizeof(T)];

    // Construct T in-place
    template <typename... Args> T* construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        return ::new (data) T(std::forward<Args>(args)...);
    }

    // Destroy T in-place
    void destroy() noexcept(std::is_nothrow_destructible_v<T>) { std::destroy_at(ptr()); }

    [[nodiscard]] T*       ptr() noexcept { return std::launder(reinterpret_cast<T*>(data)); }
    [[nodiscard]] const T* ptr() const noexcept { return std::launder(reinterpret_cast<const T*>(data)); }
};

// ============ MEMORY POOL =========

enum class PoolError {
    Exhausted,   // no free slots
    StaleHandle, // version mismatch
    NullHandle,  // called get() on a null handle
    DoubleFree,  // free() called on already-freed slot
};

template <typename T, typename HandleTag, size_t Capacity> class ResourcePool
{
public:
    using Handle = ResourceHandle<HandleTag>;

    ResourcePool()
    {
        m_Slots.fill({});
        m_Versions.fill(0);
        m_Occupancy.fill(false);

        // Index 0 is the null slot - never allocated
        m_FreeList.reserve(Capacity - 1);
        for (u32 i = Capacity - 1; i >= 1; --i)
            m_FreeList.push_back(i);
    }

    // Non-copyable / movable
    ResourcePool(const ResourcePool&)            = delete;
    ResourcePool& operator=(const ResourcePool&) = delete;
    ResourcePool(ResourcePool&&)                 = delete;
    ResourcePool& operator=(ResourcePool&&)      = delete;

    ~ResourcePool() {}

    // === Allocation =======================

    // template <typename... Args>
    // [[nodiscard]] std::expected<Handle, PoolError>
    // alloc(T object)
    // {
    //     if (m_FreeList.empty())
    //         return std::unexpected(PoolError::Exhausted);

    //     const u32 idx = m_FreeList.back();
    //     m_FreeList.pop_back();

    //     m_Slots[idx] = std::move(object);
    //     m_Occupancy[idx] = true;

    //     return Handle{idx, m_Versions[idx], 1};
    // }

    // template <typename... Args>
    // [[nodiscard]] Handle allocOrDie(T object, std::source_location loc = std::source_location::current())
    // {
    //     auto result = alloc(std::move(object));
    //     if (!result) [[unlikely]]
    //         LM_CORE_FATAL("ResourcePool exhausasted at {}:{}", loc.file_name(), loc.line());

    //     return result.value();
    // }

    template <typename... Args> [[nodiscard]] std::expected<Handle, PoolError> insert(T&& value)
    {
        if (m_FreeList.empty())
            return std::unexpected(PoolError::Exhausted);

        const u32 idx = m_FreeList.back();
        m_FreeList.pop_back();

        void* ptr = m_Slots[idx].data;
        new (ptr) T(std::move(value));

        m_Occupancy[idx] = true;

        return Handle{idx, m_Versions[idx], 1};
    }

    template <typename... Args>
    [[nodiscard]] Handle insertOrDie(T&& value, std::source_location loc = std::source_location::current())
    {
        auto result = insert(std::move(value));
        if (!result.has_value()) { [[unlikely]]
            LM_CORE_FATAL("ResourcePool exhausasted at {}:{}", loc.file_name(), loc.line());
        }

        return result.value();
    }

    // === Freeing =======================

    [[nodiscard]] std::expected<void, PoolError> free(Handle handle) noexcept
    {
        if (handle.isNull())
            return std::unexpected(PoolError::NullHandle);

        if (!validate(handle))
            return std::unexpected(PoolError::StaleHandle);

        if (!m_Occupancy[handle.index]) [[unlikely]]
            return std::unexpected(PoolError::DoubleFree);

        // m_Slots[handle.index].destroy();
        std::destroy_at(&m_Slots[handle.index]);
        handle.valid              = false;
        m_Occupancy[handle.index] = false;
        m_Versions[handle.index]++; // Invalidade previous copies of this handle
        m_FreeList.push_back(handle.index);

        return {};
    }

    // === Querying =======================

    /// Safe - validates generation and returns nullptr on stale/null handles
    [[nodiscard]] T* get(Handle handle) noexcept
    {
        if (!validate(handle))
            return nullptr;

        return ptr(handle.index);
    }

    /// Safe - validates generation and returns nullptr on stale/null handles
    [[nodiscard]] const T* getConst(Handle handle) const noexcept
    {
        if (!validate(handle))
            return nullptr;

        return ptr(handle.index);
    }

    [[nodiscard]] std::expected<T*, PoolError> tryGet(Handle handle) noexcept
    {
        if (handle.isNull())
            return std::unexpected(PoolError::NullHandle);
        if (!validate(handle))
            return std::unexpected(PoolError::StaleHandle);

        return &m_Slots[handle.index];
    }

    [[nodiscard]] std::expected<const T*, PoolError> tryGet(Handle handle) const noexcept
    {
        if (handle.isNull())
            return std::unexpected(PoolError::NullHandle);
        if (!validate(handle))
            return std::unexpected(PoolError::StaleHandle);

        return &m_Slots[handle.index];
    }

    [[nodiscard]] T& getUnchecked(Handle handle) noexcept
    {
        LM_CORE_ASSERT(validate(handle));
        return &m_Slots[handle.index];
    }

    // === Iteration =======================

    template <std::invocable<Handle, T*> Fn> void forEach(Fn&& fn) noexcept
    {
        for (u32 i = 0; i < Capacity; ++i) {
            if (!m_Occupancy[i])
                continue;

            Handle handle{i, m_Versions[i], 1};
            std::invoke(fn, handle, ptr(i));
        }
    }

    template <std::invocable<Handle, const T*> Fn> void forEach(Fn&& fn) const noexcept
    {
        for (u32 i = 0; i < Capacity; ++i) {
            if (!m_Occupancy[i])
                continue;

            Handle handle{i, m_Versions[i], 1};
            std::invoke(fn, handle, ptr(i));
        }
    }

    // === Stats =======================

    [[nodiscard]] size_t capacity() const noexcept { return Capacity; }
    [[nodiscard]] size_t available() const noexcept { return m_FreeList.size(); }
    [[nodiscard]] size_t used() const noexcept { return Capacity - 1 - m_FreeList.size(); }

private:
    [[nodiscard]] bool validate(Handle handle) const noexcept
    {
        return handle.valid && handle.index > 0 && handle.index < Capacity &&
               handle.version == m_Versions[handle.index];
    }

    T*       ptr(u32 index) { return std::launder(reinterpret_cast<T*>(m_Slots[index].data)); }
    const T* ptr(u32 index) const { return std::launder(reinterpret_cast<const T*>(m_Slots[index].data)); }

    struct Slot {
        alignas(T) std::byte data[sizeof(T)];
    };

    std::array<Slot, Capacity> m_Slots;     // Contiguous, no heap allocations
    std::array<u32, Capacity>  m_Versions;  // Generational counters (for dangling safety)
    std::array<bool, Capacity> m_Occupancy; // Alive / dead per slot
    std::vector<u32>           m_FreeList;  // Recycled indices
};

} // namespace Lemon