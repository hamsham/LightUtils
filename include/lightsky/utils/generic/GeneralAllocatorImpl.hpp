
#ifndef LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP
#define LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP

#include "lightsky/setup/Macros.h" // LS_LIKELY

#include "lightsky/utils/Assertions.h"

namespace ls
{
namespace utils
{



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
GeneralAllocator<CacheSize, OffsetFreeHeader>::~GeneralAllocator() noexcept
{
    AllocationEntry* iter = mHead;

    while (iter)
    {
        AllocationEntry* temp = iter;
        iter = iter->pNext;

        const size_type numBlocks = temp->numBlocks;
        const size_type numBytes = numBlocks * block_size;
        if (temp->pSrcPool == temp)
        {
            temp->numBlocks = 0;
            temp->pNext = nullptr;
            temp->pSrcPool = nullptr;
            temp->allocatedBlocks = 0;
            this->memory_source().free(temp, numBytes);

            LS_MEMTRACK_POOL_DESTROY(temp);
        }
        else if (!OffsetFreeHeader)
        {
            temp->pNext = nullptr;
            this->memory_source().free(reinterpret_cast<char*>(temp)+block_size, numBytes-block_size);
        }
    }

    mHead = nullptr;
    mTotalBlocksAllocd = 0;
    mLastAllocSize = 0;
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
GeneralAllocator<CacheSize, OffsetFreeHeader>::GeneralAllocator(MemorySource& memorySource) noexcept :
    //GeneralAllocator<CacheSize, OffsetFreeHeader>::GeneralAllocator{memorySource, cache_size} // delegate
    Allocator{memorySource},
    mHead{nullptr},
    mTotalBlocksAllocd{0},
    mLastAllocSize{0}
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
GeneralAllocator<CacheSize, OffsetFreeHeader>::GeneralAllocator(MemorySource& memorySource, size_type initialSize) noexcept :
    Allocator{memorySource},
    mHead{nullptr},
    mTotalBlocksAllocd{0},
    mLastAllocSize{0}
{
    ls::utils::runtime_assert(initialSize >= sizeof(size_type), ls::utils::ErrorLevel::LS_ERROR, "Allocated memory table cannot be less than sizeof(size_type).");
    ls::utils::runtime_assert(initialSize % block_size == 0, ls::utils::ErrorLevel::LS_ERROR, "Cannot fit the current block size within an allocation table.");
    ls::utils::runtime_assert(block_size < initialSize, ls::utils::ErrorLevel::LS_ERROR, "Allocation block size must be less than the total byte size.");

    mHead = reinterpret_cast<AllocationEntry*>(this->memory_source().allocate(initialSize));
    if (mHead)
    {
        // setup all links in the allocation list
        size_type numBlocks = initialSize / block_size;

        mHead->pNext = nullptr;
        mHead->numBlocks = numBlocks;
        mHead->pSrcPool = mHead;
        mHead->allocatedBlocks = numBlocks;
        mTotalBlocksAllocd = numBlocks;
        mLastAllocSize = initialSize;
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
GeneralAllocator<CacheSize, OffsetFreeHeader>::GeneralAllocator(GeneralAllocator&& a) noexcept :
    Allocator{std::move(a)},
    mHead{a.mHead},
    mTotalBlocksAllocd{a.mTotalBlocksAllocd},
    mLastAllocSize{a.mLastAllocSize}
{
    a.mHead = nullptr;
    a.mTotalBlocksAllocd = 0;
    a.mLastAllocSize = 0;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
GeneralAllocator<CacheSize, OffsetFreeHeader>& GeneralAllocator<CacheSize, OffsetFreeHeader>::operator=(GeneralAllocator&& a) noexcept
{
    if (this != &a)
    {
        Allocator::operator=(std::move(a));

        mHead = a.mHead;
        a.mHead = nullptr;

        mTotalBlocksAllocd = a.mTotalBlocksAllocd;
        a.mTotalBlocksAllocd = 0;

        mLastAllocSize = a.mLastAllocSize;
        a.mLastAllocSize = 0;
    }

    return *this;
}



/*-------------------------------------
 * Allocation merging
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline typename GeneralAllocator<CacheSize, OffsetFreeHeader>::AllocationEntry*
GeneralAllocator<CacheSize, OffsetFreeHeader>::_merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) const noexcept
{
    // Double-free detection
    if (LS_UNLIKELY((pHead < pBlock) && ((pHead + pHead->numBlocks) > pBlock)))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    if (pHead + pHead->numBlocks == pBlock)
    {
        pHead->numBlocks += pBlock->numBlocks;
        pHead->pNext = pBlock->pNext;

        pBlock->numBlocks = 0;
        pBlock->pNext = nullptr;
        pBlock->pSrcPool = nullptr;
        pBlock->allocatedBlocks = 0;
        pBlock = pHead;
    }

    return pBlock;
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
void GeneralAllocator<CacheSize, OffsetFreeHeader>::_free_impl(AllocationEntry* reclaimed, size_type blockCount) noexcept
{
    // Minor input validation. Non-sized allocations will not contain an
    // valid block count.
    reclaimed->numBlocks = blockCount;
    reclaimed->pNext = nullptr;

    // reset the head pointer if we're out of memory
    if (LS_UNLIKELY(mHead == nullptr))
    {
        // reset the head pointer
        mHead = reclaimed;
        return;
    }

    AllocationEntry* prev = nullptr;
    AllocationEntry* temp;

    if (LS_UNLIKELY(mHead > reclaimed))
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        // increase the block count if possible
        reclaimed->pNext = mHead;
        temp = _merge_allocation_blocks(reclaimed, mHead);
        mHead = reclaimed;
    }
    else // mHead < reclaimed
    {
        AllocationEntry* prev2 = nullptr;
        AllocationEntry* curr = mHead;

        while (curr && curr < reclaimed)
        {
            prev2 = prev;
            prev = curr;
            curr = curr->pNext;
        }

        reclaimed->pNext = curr;
        prev->pNext = reclaimed;

        _merge_allocation_blocks(reclaimed, curr);
        temp = _merge_allocation_blocks(prev, reclaimed);
        if (temp == prev)
        {
            prev = prev2;
        }
    }

    LS_MEMTRACK_POOL_FREE(reclaimed->pSrcPool, reclaimed+1);

    // Prune the smallest cache when possible
    if (temp->numBlocks == temp->allocatedBlocks && temp->pSrcPool == temp)// && temp->pNext)
    {
        const size_type numBlocks = temp->numBlocks;
        const size_type numBytes = numBlocks * block_size;

        if (LS_LIKELY(prev != nullptr))
        {
            prev->pNext = temp->pNext;
        }
        else
        {
            mHead = temp->pNext;
        }

        mTotalBlocksAllocd -= numBlocks;
        //mLastAllocSize = numBytes;

        temp->numBlocks = 0;
        temp->pNext = nullptr;
        temp->pSrcPool = nullptr;
        temp->allocatedBlocks = 0;
        this->memory_source().free(temp, numBytes);

        LS_MEMTRACK_POOL_DESTROY(temp);
    }
}



/*-------------------------------------
 * Allocate Memory
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline typename GeneralAllocator<CacheSize, OffsetFreeHeader>::AllocationEntry* GeneralAllocator<CacheSize, OffsetFreeHeader>::_alloc_new_cache(size_type n) noexcept
{
    // Any allocation larger than the block size gets rounded to the next cache
    // size. If that fails, we try again by rounding to the next block. Should
    // that also fail, we bail completely.

    //const size_type minBytes = (mTotalBlocksAllocd * block_size) / 2;
    //const size_type minBytes = mLastAllocSize;
    //const size_type minBytes = (mLastAllocSize * 3) / 2;
    //const size_type minBytes = (mLastAllocSize * 4) / 3;
    const size_type minBytes = (mLastAllocSize * 2) / 3;
    const size_type maxBytes = mLastAllocSize;
    //const size_type maxBytes = (mLastAllocSize * 2) / 3;
    //const size_type maxBytes = mLastAllocSize / 4;

    size_type t = n;
    //if (!OffsetFreeHeader)
    //if (n < cache_size)
    {
        if (n < mLastAllocSize/2)
        {
            t += minBytes;
        }
        else
        {
            t += maxBytes;
        }
    }

    size_type allocSize;
    void* pCache;
    size_type rem;

    rem = (t % cache_size);
    const size_type reserveRounding = t + (rem ? (cache_size - rem) : 0);

    rem = (n % cache_size);
    const size_type cacheRounding   = n + (rem ? (cache_size - rem) : 0);

    rem = (n % block_size);
    const size_type blockRounding   = n + (rem ? (block_size - rem) : 0);

    allocSize = reserveRounding;
    pCache = this->memory_source().allocate(reserveRounding);
    if (!pCache)
    {
        allocSize = cacheRounding;
        pCache = this->memory_source().allocate(cacheRounding);
        if (!pCache)
        {
            allocSize = blockRounding;
            pCache = this->memory_source().allocate(blockRounding);
            if (!pCache)
            {
                return nullptr;
            }
        }
    }

    const size_type numBlocks = allocSize / block_size;
    mTotalBlocksAllocd += numBlocks;
    mLastAllocSize = allocSize;

    AllocationEntry* pCacheEntry = reinterpret_cast<AllocationEntry*>(pCache);
    pCacheEntry->numBlocks = numBlocks;
    pCacheEntry->pNext = nullptr;
    pCacheEntry->pSrcPool = pCacheEntry;
    pCacheEntry->allocatedBlocks = numBlocks;

    LS_MEMTRACK_SLAB_POOL_CREATE(pCacheEntry, 0, true);
    return pCacheEntry;
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
void* GeneralAllocator<CacheSize, OffsetFreeHeader>::allocate(size_type n) noexcept
{
    if (!n)
    {
        return nullptr;
    }

    n += GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;
    const size_type blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

    if (mHead == nullptr)
    {
        mHead = _alloc_new_cache(n);
        if (mHead == nullptr)
        {
            return nullptr;
        }
    }

    // we're going to actually allocate "n+sizeof(size_type)" and place a header
    // in the first few bytes to keep track of the allocation size.
    AllocationEntry* prev = nullptr;
    AllocationEntry* iter = mHead;
    size_type count = mHead->numBlocks;

    while (count < blocksNeeded)
    {
        prev = iter;
        iter = iter->pNext;

        if (LS_UNLIKELY(!iter))
        {
            iter = _alloc_new_cache(n);
            if (LS_UNLIKELY(!iter))
            {
                return nullptr;
            }

            if (iter < mHead)
            {
                prev = nullptr;
                iter->pNext = mHead;
                mHead = iter;
            }
            else
            {
                AllocationEntry* curr;
                if (prev && iter > prev)
                {
                    curr = prev->pNext;
                }
                else
                {
                    prev = nullptr;
                    curr = mHead;
                }

                while (curr && curr < iter)
                {
                    prev = curr;
                    curr = curr->pNext;
                }

                iter->pNext = curr;
                prev->pNext = iter;
            }
        }

        count = iter->numBlocks;
    }

    if (count > blocksNeeded)
    {
        AllocationEntry* temp = iter + (iter->numBlocks - blocksNeeded);
        LS_PREFETCH(temp, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_NONTEMPORAL);

        AllocationEntry* const pSrcPool = iter->pSrcPool;
        iter->numBlocks -= blocksNeeded;
        iter = temp;

        temp->pSrcPool = pSrcPool;
        temp->allocatedBlocks = 0;
    }
    else // count == blocksNeeded
    {
        AllocationEntry* next = iter->pNext;

        if (prev)
        {
            prev->pNext = next;
        }
        else if (mHead == iter)
        {
            mHead = next;
        }
        else
        {
            mHead->pNext = next;
        }
    }

    // Add the allocation size to the result pointer
    iter->numBlocks = blocksNeeded;
    iter->pNext = nullptr;

    // Offset the result pointer to ensure we track the memory allocation
    char* result = reinterpret_cast<char*>(iter) + GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;
    LS_MEMTRACK_POOL_ALLOC(iter->pSrcPool, iter+1, n-header_size);

    return reinterpret_cast<void*>(result);
}



/*-------------------------------------
 * Calloc
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void* GeneralAllocator<CacheSize, OffsetFreeHeader>::allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept
{
    if (!numElements || !numBytesPerElement)
    {
        return nullptr;
    }

    if (calloc_can_overflow(numElements, numBytesPerElement))
    {
        return nullptr;
    }

    const size_type numBytes = numElements * numBytesPerElement;
    void* const pData = this->allocate(numBytes);
    if (pData)
    {
        fast_memset(pData, '\0', numBytes);
    }

    return pData;
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void* GeneralAllocator<CacheSize, OffsetFreeHeader>::reallocate(void* p, size_type numNewBytes) noexcept
{
    if (!numNewBytes)
    {
        if (p)
        {
            this->free(p);
        }

        return nullptr;
    }

    void* ret;

    if (p)
    {
        // Offset to the start of the allocation (a.k.a. the allocation header).
        char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;
        AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
        if (LS_UNLIKELY(reclaimed == mHead))
        {
            ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
        }

        // Read the input allocation size for validation
        const size_type numPrevBytes = reclaimed->numBlocks * block_size - GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;
        ret = this->reallocate(p, numNewBytes, numPrevBytes);
    }
    else
    {
        ret = this->allocate(numNewBytes);
    }

    return ret;
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void* GeneralAllocator<CacheSize, OffsetFreeHeader>::reallocate(void* p, size_type numNewBytes, size_type numPrevBytes) noexcept
{
    if (!numNewBytes)
    {
        if (p)
        {
            this->free(p);
        }

        return nullptr;
    }

    void* const pNewData = this->allocate(numNewBytes);
    if (pNewData)
    {
        if (p)
        {
            const size_type numMaxBytes = numNewBytes < numPrevBytes ? numNewBytes : numPrevBytes;
            fast_memcpy(pNewData, p, numMaxBytes);
            this->free(p, numPrevBytes);
        }
        else
        {
            fast_memset(pNewData, '\x00', numNewBytes);
        }
    }

    return pNewData;
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void GeneralAllocator<CacheSize, OffsetFreeHeader>::free(void* p) noexcept
{
    if (LS_UNLIKELY(!p))
    {
        return;
    }

    // Non-sized allocations do not contain a header to ensure the maximum
    // allocation size can be made available.
    //this->free_raw(reinterpret_cast<AllocationEntry*>(p), 1);

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;

    AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    if (LS_UNLIKELY(reclaimed == mHead))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Read the input allocation size for validation
    const size_type allocSize = reclaimed->numBlocks;
    this->_free_impl(reclaimed, allocSize);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void GeneralAllocator<CacheSize, OffsetFreeHeader>::free(void* p, size_type n) noexcept
{
    if (LS_UNLIKELY(!p || !n))
    {
        return;
    }

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;
    const size_type blocksFreed = (n / block_size) + ((n % block_size) ? 1 : 0);

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;

    AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    if (LS_UNLIKELY(reclaimed == mHead))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Read the input allocation size for validation
    const size_type allocSize = reclaimed->numBlocks;
    if (LS_UNLIKELY(allocSize != blocksFreed))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Invalid de-allocation size detected!");
    }

    this->_free_impl(reclaimed, allocSize);
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
