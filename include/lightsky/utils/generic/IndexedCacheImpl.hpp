
#include <utility> // std::move()

namespace ls
{
namespace utils
{



/*--------------------------------------
 * Generic hash function
--------------------------------------*/
template <typename T, size_t cacheSize>
constexpr size_t IndexedCache<T, cacheSize>::hash_id(size_t key) noexcept
{
    return (cacheSize && !(cacheSize & (cacheSize - 1u))) ? (key & (cacheSize - 1)) : (key % cacheSize);
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
template <typename T, size_t cacheSize>
inline IndexedCache<T, cacheSize>::IndexedCache() noexcept
{
    clear();
}



/*--------------------------------------
 * Query the cache for data (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
inline const T* IndexedCache<T, cacheSize>::query(size_t key) const noexcept
{
    const size_t i = IndexedCache<T, cacheSize>::hash_id(key);
    return (mCacheIds[i] == key) ? &mData[i] : nullptr;
}



/*--------------------------------------
 * Query the cache for data
--------------------------------------*/
template <typename T, size_t cacheSize>
inline T* IndexedCache<T, cacheSize>::query(size_t key) noexcept
{
    const size_t i = IndexedCache<T, cacheSize>::hash_id(key);
    return (mCacheIds[i] == key) ? &mData[i] : nullptr;
}



/*--------------------------------------
 * Query the cache or update with new data
--------------------------------------*/
template <typename T, size_t cacheSize>
template <class UpdateFunc>
inline T& IndexedCache<T, cacheSize>::update(size_t key, const UpdateFunc& updater) noexcept
{
    const size_t i = IndexedCache<T, cacheSize>::hash_id(key);

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
inline T* IndexedCache<T, cacheSize>::query_or_update(size_t key, T& out, const UpdateFunc& updater) noexcept
{
    const size_t i = IndexedCache<T, cacheSize>::hash_id(key);
    T* ret = (mCacheIds[i] != key) ? nullptr : &mData[i];

    if (mCacheIds[i] != key)
    {
        mCacheIds[i] = key;
        mData[i] = std::move(updater(key));
    }

    out = mData[i];

    return ret;
}



/*--------------------------------------
 * Insert an object
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void IndexedCache<T, cacheSize>::insert(size_t key, const T& val) noexcept
{
    const size_t i = IndexedCache<T, cacheSize>::hash_id(key);
    mCacheIds[i] = key;
    mData[i] = val;
}



/*--------------------------------------
 * Emplace an object
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void IndexedCache<T, cacheSize>::emplace(size_t key, T&& val) noexcept
{
    const size_t i = IndexedCache<T, cacheSize>::hash_id(key);
    mCacheIds[i] = key;
    mData[i] = std::move(val);
}



/*--------------------------------------
 * Element indexing (const)
--------------------------------------*/
template <typename T, size_t cacheSize>
template <typename index_type>
inline const T& IndexedCache<T, cacheSize>::operator[](index_type index) const noexcept
{
    return mData[index];
}



/*--------------------------------------
 * Element indexing
--------------------------------------*/
template <typename T, size_t cacheSize>
template <typename index_type>
inline T& IndexedCache<T, cacheSize>::operator[](index_type index) noexcept
{
    return mData[index];
}



/*--------------------------------------
 * Clear the cache
--------------------------------------*/
template <typename T, size_t cacheSize>
inline void IndexedCache<T, cacheSize>::clear() noexcept
{
    for (size_t& index : mCacheIds)
    {
        index = CACHE_MISS;
    }
}



/*--------------------------------------
 * Total Capacity
--------------------------------------*/
template <typename T, size_t cacheSize>
constexpr size_t IndexedCache<T, cacheSize>::capacity() const noexcept
{
    return cacheSize;
}



} // end utils namespace
} // end ls namespace
