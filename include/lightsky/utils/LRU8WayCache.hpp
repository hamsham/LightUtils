
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

    static unsigned _count_trailing_zero_bits(uint64_t x) noexcept;

    void _update_lru_index(uint64_t key0To8) noexcept;

    unsigned _get_lru_index() noexcept;

  private: // instance data
    union
    {
        uint8_t mRows[CACHE_SIZE];
        uint64_t mCols;
    };

    uint32_t mKeys[CACHE_SIZE];

    T mData[CACHE_SIZE];

  public:
    LRU8WayCache() noexcept;

    const T* query(uint32_t key) const noexcept;

    T* query(uint32_t key) noexcept;

    template <class UpdateFunc>
    T& update(uint32_t key, const UpdateFunc& updater) noexcept;

    template <class UpdateFunc>
    T* query_or_update(uint32_t key, T** out, const UpdateFunc& updater) noexcept;

    void insert(uint32_t key, const T& val) noexcept;

    void emplace(uint32_t key, T&& val) noexcept;

    const T& operator[](uint32_t index) const noexcept;

    T& operator[](uint32_t index) noexcept;

    void clear() noexcept;

    constexpr uint32_t capacity() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/LRU8WayCacheImpl.hpp"

#endif /* LS_UTILS_LRU_8_WAY_CACHE_HPP */
