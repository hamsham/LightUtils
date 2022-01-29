
#ifndef LS_UTILS_LRU_CACHE_HPP
#define LS_UTILS_LRU_CACHE_HPP

#include <cstdlib> // size_t

namespace ls
{
namespace utils
{


/**
 * @brief Least-recently-used Cache
 *
 * This cache type will store only the data which was used most recently. Each
 * time the cache is accessed, a counter will increment to monitor what data
 * was unused and will eject the most infrequently accessed data to replace it
 * with new, uncached, data.
 */
template <typename T, size_t cacheSize = 16>
class LRUCache
{
    static_assert(cacheSize != 0, "Cache objects must have a nonzero capacity.");

  public:
    static constexpr size_t CACHE_SIZE = cacheSize;

    static constexpr size_t CACHE_MISS = ~(size_t)0;

  private:
    size_t mLruId;

    size_t mCacheCounts[CACHE_SIZE];

    size_t mCacheIds[CACHE_SIZE];

    T mData[CACHE_SIZE];

    size_t _search_index(size_t key) const noexcept;

    size_t _update_index(size_t key) noexcept;

  public:
    LRUCache() noexcept;

    const T* query(size_t key) const noexcept;

    T* query(size_t key) noexcept;

    template <class UpdateFunc>
    T& update(size_t key, UpdateFunc&& updater) noexcept;

    template <class UpdateFunc>
    T& query_or_update(size_t key, UpdateFunc&& updater) noexcept;

    T& insert(size_t key, const T& val) noexcept;

    T& insert(size_t key, T&& val) noexcept;

    template <typename... Args>
    T& emplace(size_t key, Args&&... args) noexcept;

    const T& operator[](size_t index) const noexcept;

    T& operator[](size_t index) noexcept;

    void clear() noexcept;

    constexpr size_t capacity() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/LRUCacheImpl.hpp"

#endif /* LS_UTILS_LRU_CACHE_HPP */
