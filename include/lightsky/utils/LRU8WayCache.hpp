
#ifndef LS_UTILS_LRU_8_WAY_CACHE_HPP
#define LS_UTILS_LRU_8_WAY_CACHE_HPP

#include <cstdlib> // size_t
#include <cstdint>

namespace ls
{
namespace utils
{


/**
 * @brief Least-recently-used Cache, specialized to 8 elements.
 *
 * This cache type will store only the data which was used most recently. Each
 * time the cache is accessed, a counter will increment to monitor what data
 * was unused and will eject the most infrequently accessed data to replace it
 * with new, uncached, data.
 */
template <typename T>
class LRU8WayCache
{
  private: // static data
    enum : uint32_t
    {
        CACHE_SIZE = 8,
        CACHE_MISS = 0xFFFFFFFF
    };

    static int32_t _lookup_index_for_key(const uint32_t* map, uint32_t key) noexcept;

    static unsigned _count_trailing_zero_bits(uint64_t n) noexcept;

    void _update_lru_index(uint64_t key0To8) noexcept;

    unsigned _get_lru_index() const noexcept;

  private: // instance data
    alignas(alignof(uint32_t)*CACHE_SIZE) uint32_t mKeys[CACHE_SIZE];

    union alignas(alignof(uint64_t)*(CACHE_SIZE/2))
    {
        uint8_t mRows[CACHE_SIZE];
        uint64_t mCols;
    };

    T mData[CACHE_SIZE];

  public:
    LRU8WayCache() noexcept;

    const T* query(uint32_t key) const noexcept;

    T* query(uint32_t key) noexcept;

    template <class UpdateFunc>
    T& update(uint32_t key, UpdateFunc&& updater) noexcept;

    template <class UpdateFunc>
    T& query_or_update(uint32_t key, UpdateFunc&& updater) noexcept;

    T& insert(uint32_t key, const T& val) noexcept;

    T& insert(uint32_t key, T&& val) noexcept;

    template <typename... Args>
    T& emplace(uint32_t key, Args&&... args) noexcept;

    const T& operator[](uint32_t index) const noexcept;

    T& operator[](uint32_t index) noexcept;

    void clear() noexcept;

    constexpr uint32_t capacity() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/LRU8WayCacheImpl.hpp"

#endif /* LS_UTILS_LRU_8_WAY_CACHE_HPP */
