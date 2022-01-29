
#ifndef LS_UTILS_LRU_8_WAY_CACHE_IMPL_HPP
#define LS_UTILS_LRU_8_WAY_CACHE_IMPL_HPP

#include <utility> // std::move, std::forward

#include "lightsky/setup/Api.h" // LS_INLINE
#include "lightsky/setup/Compiler.h"
#include "lightsky/setup/Arch.h"

#include "lightsky/utils/Assertions.h"

#if defined(LS_ARCH_X86)
    #include <immintrin.h>
#elif defined(LS_ARCH_ARM)
    #include <arm_neon.h>
#endif



namespace ls
{
namespace utils
{

/*-----------------------------------------------------------------------------
 * 8-way LRU
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Determine if there's a key within the container
--------------------------------------*/
template <typename T>
inline LS_INLINE int32_t LRU8WayCache<T>::_lookup_index_for_key(const uint32_t* map, uint32_t key) noexcept
{
    #if defined(LS_X86_SSE4_1)
        const __m128i lo       = _mm_loadu_si128((const __m128i*)map);
        const __m128i hi       = _mm_loadu_si128((const __m128i*)map + 1);
        const __m128i val      = _mm_set1_epi32((int)key);
        const __m128i loMask   = _mm_cmpeq_epi32(val, lo);
        const __m128i hiMask   = _mm_cmpeq_epi32(val, hi);
        const __m128i shortMax = _mm_cmpeq_epi32(val, val);
        const __m128i mask16   = _mm_packs_epi32(loMask, hiMask);
        const __m128i mask     = _mm_minpos_epu16(_mm_sub_epi16(shortMax, mask16));
        return -_mm_test_all_zeros(mask16, mask16) | _mm_extract_epi16(mask, 1);

    #elif defined(LS_X86_SSE2)
        constexpr uint32_t indexArray[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        __m128i loIndices = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&indexArray[0]));
        __m128i hiIndices = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&indexArray[4]));
        __m128i val       = _mm_set1_epi32(key);
        __m128i lo        = _mm_loadu_si128(reinterpret_cast<const __m128i*>(map));
        __m128i hi        = _mm_loadu_si128(reinterpret_cast<const __m128i*>(map+4));
        __m128i loMask    = _mm_and_si128(loIndices, _mm_cmpeq_epi32(val, lo));
        __m128i hiMask    = _mm_and_si128(hiIndices, _mm_cmpeq_epi32(val, hi));
        __m128i mask4     = _mm_or_si128(loMask, hiMask);
        __m128i mask2     = _mm_or_si128(mask4, _mm_unpackhi_epi64(mask4, mask4));
        __m128i mask      = _mm_or_si128(mask2, _mm_shuffle_epi32(mask2, 0xB1));
        return _mm_cvtsi128_si32(mask) -1;

    #elif defined(LS_ARCH_AARCH64)
        constexpr uint32_t indexArray[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        uint32x4_t loIndices = vld1q_u32(&indexArray[0]);
        uint32x4_t hiIndices = vld1q_u32(&indexArray[4]);
        uint32x4_t val       = vdupq_n_u32(key);
        uint32x4_t lo        = vld1q_u32(map);
        uint32x4_t hi        = vld1q_u32(map+4);
        uint32x4_t loMask    = vandq_u32(loIndices, vceqq_s32(val, lo));
        uint32x4_t hiMask    = vandq_u32(hiIndices, vceqq_s32(val, hi));
        uint32x4_t mask4     = vorrq_u32(loMask, hiMask);
        int32_t ret          = vmaxvq_s32(vreinterpretq_s32_u32(mask4));
        return ret - 1;

    #elif defined(LS_ARCH_ARM)
        constexpr uint32_t indexArray[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        uint32x4_t loIndices = vld1q_u32(&indexArray[0]);
        uint32x4_t hiIndices = vld1q_u32(&indexArray[4]);
        uint32x4_t val       = vdupq_n_u32(key);
        uint32x4_t lo        = vld1q_u32(map);
        uint32x4_t hi        = vld1q_u32(map+4);
        uint32x4_t loMask    = vandq_u32(loIndices, vceqq_s32(val, lo));
        uint32x4_t hiMask    = vandq_u32(hiIndices, vceqq_s32(val, hi));
        uint32x4_t mask4     = vorrq_u32(loMask, hiMask);
        uint32x2_t mask2     = vorr_u32(vget_low_u32(mask4), vget_high_u32(mask4));
        uint32_t mask        = vget_lane_u32(mask2, 0) | vget_lane_u32(mask2, 1);
        return (int32_t)mask -1;

    #else
        const int32_t k0 = -(key == map[0]) & 1;
        const int32_t k1 = -(key == map[1]) & 2;
        const int32_t k2 = -(key == map[2]) & 3;
        const int32_t k3 = -(key == map[3]) & 4;
        const int32_t k4 = -(key == map[4]) & 5;
        const int32_t k5 = -(key == map[5]) & 6;
        const int32_t k6 = -(key =std::forward= map[6]) & 7;
        const int32_t k7 = -(key == map[7]) & 8;
        return (k0|k1|k2|k3|k4|k5|k6|k7) - 1;

    #endif
}



/*--------------------------------------
 * Find the index of the least-recently used value
--------------------------------------*/
template <typename T>
inline LS_INLINE unsigned LRU8WayCache<T>::_count_trailing_zero_bits(uint64_t n) noexcept
{
    #if defined(LS_COMPILER_GNU)
        return (unsigned)__builtin_ctzll(n);

    #elif defined(LS_COMPILER_MSC)
        unsigned long ret;
        #if LS_OS_WINDOWS == 64
            return (_BitScanForward64(&ret, (unsigned long long)n) ? (unsigned)ret : 64u);
        #else
            return (_BitScanForward(&ret, (unsigned long)n) ? (unsigned)ret : 64u);
        #endif

    #else
        unsigned ret = 0ull;
        while (!(n & 1ull))
        {
            n >>= 1ull;
            ++ret;
        }
        return ret;

    #endif
}



/*--------------------------------------
 * Update the least-recently used value
--------------------------------------*/
template <typename T>
inline LS_INLINE void LRU8WayCache<T>::_update_lru_index(uint64_t key0To8) noexcept
{
    LS_DEBUG_ASSERT(key0To8 < 8);
    mRows[key0To8] = 0xFF;
    mCols &= ~(0x0101010101010101ull << key0To8);
}



/*--------------------------------------
 * Find the least-recently used value
--------------------------------------*/
template <typename T>
inline LS_INLINE unsigned LRU8WayCache<T>::_get_lru_index() noexcept
{
    const uint64_t n = (mCols-0x0101010101010101ull) & ~mCols & 0x8080808080808080ull;
    return _count_trailing_zero_bits(n) >> 3;
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
template <typename T>
LRU8WayCache<T>::LRU8WayCache() noexcept :
    mCols{},
    mKeys{CACHE_MISS, CACHE_MISS, CACHE_MISS, CACHE_MISS, CACHE_MISS, CACHE_MISS, CACHE_MISS, CACHE_MISS},
    mData{}
{}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T>
inline const T* LRU8WayCache<T>::query(uint32_t key) const noexcept
{
    int index = _lookup_index_for_key(mKeys, key);
    return (index >= 0) ? &mData[index] : nullptr;
}



/*--------------------------------------
 * Query the cache for data
--------------------------------------*/
template <typename T>
inline T* LRU8WayCache<T>::query(uint32_t key) noexcept
{
    int index = _lookup_index_for_key(mKeys, key);
    return (index >= 0) ? &mData[index] : nullptr;
}



/*--------------------------------------
 * Query the cache and update with new data
--------------------------------------*/
template <typename T>
template <class UpdateFunc>
inline T& LRU8WayCache<T>::update(uint32_t key, UpdateFunc&& updater) noexcept
{
    int32_t index = _lookup_index_for_key(mKeys, key);
    if (index < 0)
    {
        index = (int32_t)_get_lru_index();
        mKeys[index] = key;
    }

    _update_lru_index(index);

    updater(key, mData[index]);
    return mData[index];
}



/*--------------------------------------
 * Query the cache or update with new data
--------------------------------------*/
template <typename T>
template <class UpdateFunc>
inline T& LRU8WayCache<T>::query_or_update(uint32_t key, UpdateFunc&& updater) noexcept
{
    int32_t index = _lookup_index_for_key(mKeys, key);

    if (index < 0)
    {
        index = (int32_t)_get_lru_index();
        mKeys[index] = key;
        updater(key, mData[index]);
    }

    _update_lru_index(index);

    return mData[index];
}



/*--------------------------------------
 * Insert an object
--------------------------------------*/
template <typename T>
inline T& LRU8WayCache<T>::insert(uint32_t key, const T& val) noexcept
{
    int32_t index = _lookup_index_for_key(mKeys, key);

    if (index < 0)
    {
        index = (int32_t)_get_lru_index();
        mKeys[index] = key;
    }

    _update_lru_index(index);

    mData[index] = val;
    return mData[index];
}



/*--------------------------------------
 * Insert an object (r-value)
--------------------------------------*/
template <typename T>
inline T& LRU8WayCache<T>::insert(uint32_t key, T&& val) noexcept
{
    int32_t index = _lookup_index_for_key(mKeys, key);

    if (index < 0)
    {
        index = (int32_t)_get_lru_index();
        mKeys[index] = key;
    }

    _update_lru_index(index);

    mData[index] = std::move(val);
    return mData[index];
}



/*--------------------------------------
 * Construct an object in-place
--------------------------------------*/
template <typename T>
template <typename... Args>
inline T& LRU8WayCache<T>::emplace(uint32_t key, Args&&... args) noexcept
{
    int32_t index = _lookup_index_for_key(mKeys, key);

    if (index < 0)
    {
        index = (int32_t)_get_lru_index();
        mKeys[index] = key;
    }

    _update_lru_index(index);

    mData[index] = T{std::forward<Args>(args)...};
    return mData[index];
}



/*--------------------------------------
 * Index an object (const)
--------------------------------------*/
template <typename T>
inline const T& LRU8WayCache<T>::operator[](uint32_t index) const noexcept
{
    LS_DEBUG_ASSERT(index < 8);
    return mData[index];
}



/*--------------------------------------
 * Index an object
--------------------------------------*/
template <typename T>
inline T& LRU8WayCache<T>::operator[](uint32_t index) noexcept
{
    LS_DEBUG_ASSERT(index < 8);
    return mData[index];
}



/*--------------------------------------
 * Clear all keys and object in *this.
--------------------------------------*/
template <typename T>
inline void LRU8WayCache<T>::clear() noexcept
{
    mCols = 0;

    for (uint32_t i = 0; i < CACHE_SIZE; ++i)
    {
        mKeys[i] = (uint32_t)CACHE_MISS;
    }
}



/*--------------------------------------
 * Get the container capacity
--------------------------------------*/
template <typename T>
constexpr uint32_t LRU8WayCache<T>::capacity() const noexcept
{
    return CACHE_SIZE;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_LRU_8_WAY_CACHE_IMPL_HPP */
