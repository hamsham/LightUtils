
#ifndef LS_UTILS_INDEXED_CACHE_HPP
#define LS_UTILS_INDEXED_CACHE_HPP

#include <cstdlib> // size_t

namespace ls
{
namespace utils
{



/**
 * @brief Indexed/Hashed cache type
 *
 * This cache hashes incoming data and stores it until some un-hashed data
 * comes in.
 */
template <typename T, size_t cacheSize = 16>
class IndexedCache
{
    static_assert(cacheSize != 0, "Cache objects must have a nonzero capacity.");

  private:
    static constexpr size_t hash_id(size_t key) noexcept;

  public:
    static constexpr size_t CACHE_SIZE = cacheSize;

    static constexpr size_t CACHE_MISS = ~(size_t)0;

  private:
    size_t mCacheIds[CACHE_SIZE];

    T mData[CACHE_SIZE];

  public:
    IndexedCache() noexcept;

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

#include "lightsky/utils/generic/IndexedCacheImpl.hpp"

#endif /* LS_UTILS_INDEXED_CACHE_HPP */
