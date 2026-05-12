#pragma once


#include "ResourcePool.h"
#include <expected>
#include <string>
#include <unordered_map>

namespace Lemon {

enum class RegistryError {
    AlreadyExists,
    InvalidHandle,
    NotFound,
    PoolExhausted
};

template <typename T, typename HandleTag, size_t _Capacity, typename Key = std::string>
class ResourceRegistry {
public:
    using Handle = ResourceHandle<HandleTag>;
    
    using Pool = ResourcePool<T, HandleTag, _Capacity>;

    ResourceRegistry() = default;

    ResourceRegistry(const ResourceRegistry&)            = delete;
    ResourceRegistry& operator=(const ResourceRegistry&) = delete;
    ResourceRegistry(ResourceRegistry&&)                 = delete;
    ResourceRegistry& operator=(ResourceRegistry&&)      = delete;

    // ==================================================
    // === CREATE =======================================
    // ==================================================

    [[nodiscard]] std::expected<Handle, RegistryError>
    Insert(Key key, T&& value) {
        if (m_KeyToHandle.contains(key))
            return std::unexpected(RegistryError::AlreadyExists);

        auto result = m_Pool.insert(std::move(value));

        if (!result.has_value())
            return std::unexpected(RegistryError::PoolExhausted);

        Handle handle = result.value();
        m_KeyToHandle.emplace(key, handle);
        return handle;
    }

    
    // ==================================================
    // === GET ==========================================
    // ==================================================

    [[nodiscard]] T* Get(Handle handle) noexcept {
        return m_Pool.get(handle);
    }

    [[nodiscard]] const T* Get(Handle handle) const noexcept {
        return m_Pool.get(handle);
    }
    
    // ==================================================
    // === FIND FROM KEY ================================
    // ==================================================

    [[nodiscard]] T* Find(Key key) noexcept {
        auto it = m_KeyToHandle.find(key);

        if (it == m_KeyToHandle.end())
            return nullptr;

        return Get(it->second);
    }

    [[nodiscard]] const T* Find(Key key) const noexcept {
        auto it = m_KeyToHandle.find(key);
        
        if (it == m_KeyToHandle.end())
            return nullptr;

        return Get(it->second);
    }
    
    // ==================================================
    // === DESTROY ======================================
    // ==================================================

    [[nodiscard]] std::expected<void, RegistryError> Destroy(Handle handle) noexcept {

        if (!Contains(handle))
            return std::unexpected(RegistryError::NotFound);

        m_KeyToHandle.erase(m_KeyToHandle.find(handle.key));
        
        auto result = m_Pool.free(handle);

        if (!result.has_value())
            return std::unexpected(RegistryError::InvalidHandle);

        return {};
    }

    // ==================================================
    // === CONTAINS =====================================
    // ==================================================
    [[nodiscard]]
    bool Contains(Handle handle) const noexcept
    {
        return m_Pool.getConst(handle) != nullptr;
    }

    [[nodiscard]]
    bool Contains(Key key) const noexcept
    {
        return m_KeyToHandle.contains(key);
    }


    // ========================================================
    // === ITERATION ==========================================
    // ========================================================

    template<std::invocable<Handle, T&> Fn>
    void ForEach(Fn&& fn)
    {
        m_Pool.forEach(
            [&](Handle handle, T* resource)
            {
                std::invoke(fn, handle, *resource);
            });
    }

    template<std::invocable<Handle, const T&> Fn>
    void ForEach(Fn&& fn) const
    {
        m_Pool.forEach(
            [&](Handle handle, const T* resource)
            {
                std::invoke(fn, handle, *resource);
            });
    }
     // ========================================================
    // STATS
    // ========================================================

    [[nodiscard]]
    size_t Capacity() const noexcept
    {
        return m_Pool.capacity();
    }

    [[nodiscard]]
    size_t Used() const noexcept
    {
        return m_Pool.used();
    }

    [[nodiscard]]
    size_t Available() const noexcept
    {
        return m_Pool.available();
    }



    ~ResourceRegistry() = default;


private:

    Pool m_Pool;

    std::unordered_map<Key, Handle> m_KeyToHandle;
};

}