
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
inline size_t LRUCache<T, cacheSize>::_search_index(size_t key) const noexcept
{
    for (size_t i = 0; i < cacheSize; ++i)
    {
        if (key == mCacheIds[i])
        {
            return i;
        }
    }

    return CACHE_MISS;
}



/*--------------------------------------
 * Get an ID for a key and update the cache counts
--------------------------------------*/
template <typename T, size_t cacheSize>
size_t LRUCache<T, cacheSize>::_update_index(size_t key) noexcept
{
    size_t keyIndex = _search_index(key);
    if (keyIndex == CACHE_MISS)
    {
        keyIndex = mLruId;
    }
    else if (mCacheCounts[keyIndex] == (cacheSize-1))
    {
        // already the most-used element, no need to update anything
        return keyIndex;
    }

    size_t oldCount = mCacheCounts[keyIndex];
    mCacheCounts[keyIndex] = cacheSize;

    for (size_t i = 0; i < cacheSize; ++i)
    {
        if (oldCount < mCacheCounts[i])
        {
            --mCacheCounts[i];
        }
    }

    // scan for LRU element
    size_t lruId = 0;
    for (size_t i = 0; i < cacheSize; ++i)
    {
        if (mCacheCounts[lruId] > mCacheCounts[i])
        {
            lruId = i;
        }
    }

    mLruId = lruId;

    return keyIndex;
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline const T* LRUCache<T, cacheSize>::query(size_t key) const noexcept
{
    size_t i = _search_index(key);
    return i != CACHE_MISS ? &mData[i] : nullptr;
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T* LRUCache<T, cacheSize>::query(size_t key) noexcept
{
    size_t i = _search_index(key);
    return i != CACHE_MISS ? &mData[i] : nullptr;
}



/*--------------------------------------
 * Query the cache and update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T& LRUCache<T, cacheSize>::update(size_t key, UpdateFunc&& updater) noexcept
{
    size_t i = _update_index(key);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
    }

    updater(key, mData[i]);
    return mData[i];
}



/*--------------------------------------
 * Query the cache or update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T& LRUCache<T, cacheSize>::query_or_update(size_t key, UpdateFunc&& updater) noexcept
{
    size_t i = _update_index(key);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
        updater(key, mData[i]);
    }

    return mData[i];
}



/*--------------------------------------
 * Insert an object
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T& LRUCache<T, cacheSize>::insert(size_t key, const T& val) noexcept
{
    size_t i = _update_index(key);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
    }

    mData[i] = val;
    return mData[i];
}



/*--------------------------------------
 * Insert an object (r-value)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T& LRUCache<T, cacheSize>::insert(size_t key, T&& val) noexcept
{
    size_t i = _update_index(key);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
    }

    mData[i] = std::move(val);
    return mData[i];
}



/*--------------------------------------
 * Emplace an object
--------------------------------------*/
template <typename T, size_t cacheSize>
template <typename... Args>
inline T& LRUCache<T, cacheSize>::emplace(size_t key, Args&&... args) noexcept
{
    size_t i = _update_index(key);

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
    }

    mData[i] = T{std::forward<Args>(args)...};
    return mData[i];
}



/*--------------------------------------
 * Element indexing (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline const T& LRUCache<T, cacheSize>::operator[](size_t index) const noexcept
{
    return mData[index];
}



/*--------------------------------------
 * Element indexing
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T& LRUCache<T, cacheSize>::operator[](size_t index) noexcept
{
    return mData[index];
}



/*--------------------------------------
 * Clear the cache
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void LRUCache<T, cacheSize>::clear() noexcept
{
    mLruId = 0;

    for (size_t i = 0; i < cacheSize; ++i)
    {
        mCacheCounts[i] = i;
    }

    for (size_t& index : mCacheIds)
    {
        index = CACHE_MISS;
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
