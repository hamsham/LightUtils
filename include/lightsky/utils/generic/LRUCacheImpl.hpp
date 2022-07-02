
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
        if (key == mKeys[i])
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
    constexpr size_t lastIndex = cacheSize-1;

    if (key == mKeys[0])
    {
        // already the most-used element, no need to update anything
        return 0;
    }

    size_t keyIndex = _search_index(key);
    if (keyIndex == CACHE_MISS)
    {
        keyIndex = lastIndex;
        mKeys[lastIndex] = CACHE_MISS;
    }

    // rotate elements from most-recently used to least used with the least-
    // used element at the end of our arrays
    const size_t tempKey = mKeys[keyIndex];
    const size_t tempIndex = mIndices[keyIndex];
    for (size_t i = keyIndex; i > 0; --i) // O(log(n)) best case, O(n) worst
    {
        mKeys[i] = mKeys[i-1];
        mIndices[i] = mIndices[i-1];
    }

    mKeys[0] = tempKey;
    mIndices[0] = tempIndex;

    return 0;
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline const T* LRUCache<T, cacheSize>::query(size_t key) const noexcept
{
    size_t i = _search_index(key);
    return i != CACHE_MISS ? &mData[mIndices[i]] : nullptr;
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T* LRUCache<T, cacheSize>::query(size_t key) noexcept
{
    size_t i = _search_index(key);
    return i != CACHE_MISS ? &mData[mIndices[i]] : nullptr;
}



/*--------------------------------------
 * Query the cache and update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T& LRUCache<T, cacheSize>::update(size_t key, UpdateFunc&& updater) noexcept
{
    size_t i = _update_index(key);

    if (mKeys[i] != key)
    {
        mKeys[i] = key;
    }

    updater(key, mData[mIndices[i]]);
    return mData[mIndices[i]];
}



/*--------------------------------------
 * Query the cache or update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T& LRUCache<T, cacheSize>::query_or_update(size_t key, UpdateFunc&& updater) noexcept
{
    size_t i = _update_index(key);

    if (mKeys[i] != key)
    {
        mKeys[i] = key;
        updater(key, mData[mIndices[i]]);
    }

    return mData[mIndices[i]];
}



/*--------------------------------------
 * Insert an object
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T& LRUCache<T, cacheSize>::insert(size_t key, const T& val) noexcept
{
    size_t i = _update_index(key);

    if (mKeys[i] != key)
    {
        mKeys[i] = key;
    }

    mData[mIndices[i]] = val;
    return mData[mIndices[i]];
}



/*--------------------------------------
 * Insert an object (r-value)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T& LRUCache<T, cacheSize>::insert(size_t key, T&& val) noexcept
{
    size_t i = _update_index(key);

    if (mKeys[i] != key)
    {
        mKeys[i] = key;
    }

    mData[mIndices[i]] = std::move(val);
    return mData[mIndices[i]];
}



/*--------------------------------------
 * Emplace an object
--------------------------------------*/
template <typename T, size_t cacheSize>
template <typename... Args>
inline T& LRUCache<T, cacheSize>::emplace(size_t key, Args&&... args) noexcept
{
    size_t i = _update_index(key);

    if (mKeys[i] != key)
    {
        mKeys[i] = key;
    }

    mData[mIndices[i]] = T{std::forward<Args>(args)...};
    return mData[mIndices[i]];
}



/*--------------------------------------
 * Element indexing (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline const T& LRUCache<T, cacheSize>::operator[](size_t index) const noexcept
{
    return mData[_search_index(index)];
}



/*--------------------------------------
 * Element indexing
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T& LRUCache<T, cacheSize>::operator[](size_t index) noexcept
{
    return mData[_search_index(index)];
}



/*--------------------------------------
 * Clear the cache
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void LRUCache<T, cacheSize>::clear() noexcept
{
    for (size_t i = 0; i < cacheSize; ++i)
    {
        mKeys[i] = CACHE_MISS;
        mIndices[i] = i;
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
