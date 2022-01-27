
#ifndef LS_UTILS_LRU_CACHE_IMPL_HPP
#define LS_UTILS_LRU_CACHE_IMPL_HPP

#include <utility> // std::move

namespace ls
{
namespace utils
{



/*--------------------------------------
 * Constructor
--------------------------------------*/
template <typename T, size_t cacheSize>
inline LRUCache<T, cacheSize>::LRUCache() noexcept
{
    clear();
}



/*--------------------------------------
 * Get an ID for a key
--------------------------------------*/
template <typename T, size_t cacheSize>
inline size_t LRUCache<T, cacheSize>::get_index_for_key(size_t key, bool incrementCounts) noexcept
{
    size_t ret = CACHE_MISS;
    size_t cacheIndex = 0;//capacity()-1;

    for (size_t i = 0; i < capacity(); ++i)
    {
        if (mCacheIds[i] == key)
        {
            ret = i;
        }

        if (mCacheCounts[cacheIndex] < mCacheCounts[i])
        {
            cacheIndex = i;
        }
    }

    if (ret == CACHE_MISS)
    {
        ret = cacheIndex;
    }

    if (incrementCounts)
    {
        for (size_t i = capacity(); i--;)
        {
            if (mCacheCounts[i] != CACHE_MISS)
            {
                ++mCacheCounts[i];
            }
        }

        mCacheCounts[ret] = 0;
    }

     return ret;
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline const T* LRUCache<T, cacheSize>::query(size_t key) const noexcept
{
    const T* ret = nullptr;

    for (size_t i = capacity(); i--;)
    {
        if (mCacheIds[i] == key)
        {
            ret = &mData[i];
            break;
        }
    }

    return ret;
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T* LRUCache<T, cacheSize>::query(size_t key) noexcept
{
    T* ret = nullptr;

    for (size_t i = capacity(); i--;)
    {
        if (mCacheIds[i] == key)
        {
            ret = &mData[i];
            break;
        }
    }

    return ret;
}



/*--------------------------------------
 * Query the cache or update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T& LRUCache<T, cacheSize>::update(size_t key, const UpdateFunc& updater) noexcept
{
    size_t i = get_index_for_key(key, true);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
        mData[i] = std::move(updater(key));
    }

    return mData[i];
}



/*--------------------------------------
 * Query the cache or update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T* LRUCache<T, cacheSize>::query_or_update(size_t key, T** out, const UpdateFunc& updater) noexcept
{
    T* ret = nullptr;
    size_t i = get_index_for_key(key, true);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
        mData[i] = std::move(updater(key));
    }
    else
    {
        ret = &mData[i];
    }

    *out = mData + i;

    return ret;
}



/*--------------------------------------
 * Insert an object
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void LRUCache<T, cacheSize>::insert(size_t key, const T& val) noexcept
{
    size_t i = get_index_for_key(key, true);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
    }

    mData[i] = val;
}



/*--------------------------------------
 * Emplace an object
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void LRUCache<T, cacheSize>::emplace(size_t key, T&& val) noexcept
{
    size_t i = get_index_for_key(key, true);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
    }

    mData[i] = std::move(val);
}



/*--------------------------------------
 * Element indexing (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
template <typename index_type>
inline const T& LRUCache<T, cacheSize>::operator[](index_type index) const noexcept
{
    return mData[index];
}



/*--------------------------------------
 * Element indexing
--------------------------------------*/
template <typename T, size_t cacheSize>
template <typename index_type>
inline T& LRUCache<T, cacheSize>::operator[](index_type index) noexcept
{
    return mData[index];
}



/*--------------------------------------
 * Clear the cache
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void LRUCache<T, cacheSize>::clear() noexcept
{
    for (size_t& index : mCacheIds)
    {
        index = CACHE_MISS;
    }

    for (size_t& count : mCacheCounts)
    {
        count = CACHE_MISS;
    }
}



/*--------------------------------------
 * Total Capacity
--------------------------------------*/
template <typename T, size_t cacheSize>
constexpr size_t LRUCache<T, cacheSize>::capacity() const noexcept
{
    return cacheSize;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_LRU_CACHE_IMPL_HPP */
