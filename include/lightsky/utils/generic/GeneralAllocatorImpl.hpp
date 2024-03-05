
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
            //*temp = AllocationEntry{0, nullptr, nullptr, 0};
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
    mLastAllocSize{0}
{
    ls::utils::runtime_assert(initialSize >= sizeof(size_type), ls::utils::ErrorLevel::LS_ERROR, "Allocated memory table cannot be less than sizeof(size_type).");
    ls::utils::runtime_assert(initialSize % block_size == 0, ls::utils::ErrorLevel::LS_ERROR, "Cannot fit the current block size within an allocation table.");
    ls::utils::runtime_assert(block_size < initialSize, ls::utils::ErrorLevel::LS_ERROR, "Allocation block size must be less than the total byte size.");

    size_type numAllocatedBytes = 0;
    mHead = reinterpret_cast<AllocationEntry*>(this->memory_source().allocate(initialSize, &numAllocatedBytes));
    if (mHead)
    {
        // setup all links in the allocation list
        size_type numBlocks = numAllocatedBytes / block_size;

        mHead->pNext = nullptr;
        mHead->numBlocks = numBlocks;
        mHead->pSrcPool = mHead;
        mHead->allocatedBlocks = numBlocks;
        mLastAllocSize = numAllocatedBytes;
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
GeneralAllocator<CacheSize, OffsetFreeHeader>::GeneralAllocator(GeneralAllocator&& a) noexcept :
    Allocator{std::move(a)},
    mHead{a.mHead},
    mLastAllocSize{a.mLastAllocSize}
{
    a.mHead = nullptr;
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
GeneralAllocator<CacheSize, OffsetFreeHeader>::_merge_allocation_blocks(AllocationEntry* LS_RESTRICT_PTR pHead, AllocationEntry* LS_RESTRICT_PTR pBlock) const noexcept
{
    const size_type numBlocks = pHead->numBlocks;
    const size_type numBytes = numBlocks * block_size;
    const size_type diff = ((size_type)pBlock - (size_type)pHead); // expects pBlock > pHead

    if (diff > numBytes)
    {
        pHead->pNext = pBlock;
    }
    else if (diff == numBytes)
    {
        const size_type numMergedBlocks = pBlock->numBlocks;
        AllocationEntry* pMergedNext = pBlock->pNext;

        pHead->numBlocks = numBlocks + numMergedBlocks;
        pHead->pNext = pMergedNext;

        *pBlock = AllocationEntry{0, nullptr, nullptr, 0};
        pBlock = pHead;
    }
    else // Double-free detection
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    return pBlock;
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
void GeneralAllocator<CacheSize, OffsetFreeHeader>::_free_impl(AllocationEntry* LS_RESTRICT_PTR reclaimed) noexcept
{
    AllocationEntry* prev = nullptr;
    AllocationEntry* temp;

    // reset the head pointer if we're out of memory
    if (LS_LIKELY(!mHead || mHead > reclaimed))
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        // increase the block count if possible
        _merge_allocation_blocks(reclaimed, mHead);

        temp = reclaimed;
        mHead = reclaimed;
    }
    else // mHead < reclaimed
    {
        AllocationEntry* prev2 = nullptr;
        AllocationEntry* curr = mHead->pNext;
        prev = mHead;

        while (curr && curr < reclaimed)
        {
            prev2 = prev;
            prev = curr;
            curr = curr->pNext;
        }

        _merge_allocation_blocks(reclaimed, curr);
        temp = _merge_allocation_blocks(prev, reclaimed);

        if (temp == prev)
        {
            prev = prev2;
        }
    }

    LS_MEMTRACK_POOL_FREE(reclaimed->pSrcPool, reclaimed+1);

    // Prune the smallest cache when possible
    if (temp->numBlocks == temp->allocatedBlocks)
    {
        if (!(prev || temp->pNext))
        {
            return;
        }

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

        //*temp = AllocationEntry{0, nullptr, nullptr, 0};

        this->memory_source().free(temp, numBytes);
        LS_MEMTRACK_POOL_DESTROY(temp);
    }
}



/*-------------------------------------
 * Allocate Memory
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline typename GeneralAllocator<CacheSize, OffsetFreeHeader>::AllocationEntry*
GeneralAllocator<CacheSize, OffsetFreeHeader>::_alloc_new_cache(size_type n, size_type* pOutNumBytes) noexcept
{
    // Any allocation larger than the block size gets rounded to the next cache
    // size. If that fails, we try again by rounding to the next block. Should
    // that also fail, we bail completely.

    const size_type lastAllocSize = mLastAllocSize;
    size_type reserveBytes;
    if (n < cache_size)
    {
        reserveBytes = cache_size;
    }
    else if (n < (lastAllocSize / 2))
    {
        reserveBytes = lastAllocSize;
    }
    else
    {
        reserveBytes = (lastAllocSize*3) / 2;
    }

    const size_type t = n + reserveBytes;
    size_type allocSize = 0;
    void* pCache;
    size_type rem;

    rem = (t % cache_size);
    const size_type reserveRounding = t + (rem ? (cache_size - rem) : 0);

    rem = (n % cache_size);
    const size_type cacheRounding   = n + (rem ? (cache_size - rem) : 0);

    rem = (n % block_size);
    const size_type blockRounding   = n + (rem ? (block_size - rem) : 0);

    pCache = this->memory_source().allocate(reserveRounding, &allocSize);
    if (!pCache)
    {
        pCache = this->memory_source().allocate(cacheRounding, &allocSize);
        if (!pCache)
        {
            pCache = this->memory_source().allocate(blockRounding, &allocSize);
            if (!pCache)
            {
                return nullptr;
            }
        }
    }

    if (pOutNumBytes)
    {
        *pOutNumBytes = allocSize;
    }

    const size_type numBlocks = allocSize / block_size;
    mLastAllocSize = allocSize;

    AllocationEntry* pCacheEntry = reinterpret_cast<AllocationEntry*>(pCache);
    *pCacheEntry = AllocationEntry{numBlocks, nullptr, pCacheEntry, numBlocks};

    LS_MEMTRACK_SLAB_POOL_CREATE(pCacheEntry, 0, true);
    return pCacheEntry;
}



/*-------------------------------------
 * Locate or Allocate
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline bool GeneralAllocator<CacheSize, OffsetFreeHeader>::_find_or_allocate_entry(
    size_type numBlocksNeeded,
    AllocationEntry*& pIter,
    AllocationEntry*& pPrev,
    size_type* pOutNumBytes) noexcept
{
    // we're going to actually allocate "n+sizeof(size_type)" and place a header
    // in the first few bytes to keep track of the allocation size.
    bool ret = false;
    const size_type numBytes = numBlocksNeeded * block_size;

    if (LS_UNLIKELY(mHead == nullptr))
    {
        mHead = _alloc_new_cache(numBytes, pOutNumBytes);
        if (LS_LIKELY(mHead != nullptr))
        {
            ret = true;
            pIter = mHead;
            pPrev = nullptr;
        }

        return ret;
    }

    AllocationEntry* prev = nullptr;
    AllocationEntry* iter = mHead;
    size_type count = mHead->numBlocks;

    do
    {
        if (count >= numBlocksNeeded)
        {
            ret = true;
            pIter = iter;
            pPrev = prev;
            break;
        }

        prev = iter;
        iter = iter->pNext;

        if (LS_UNLIKELY(!iter))
        {
            iter = _alloc_new_cache(numBytes, pOutNumBytes);
            if (LS_UNLIKELY(!iter))
            {
                break;
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
    while (true);

    return ret;
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
void* GeneralAllocator<CacheSize, OffsetFreeHeader>::allocate(size_type n, size_type* pOutNumBytes) noexcept
{
    if (!n)
    {
        return nullptr;
    }

    // increase the allocation size to make room for a header.
    n += GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size;
    const size_type blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

    AllocationEntry* pPrev = nullptr;
    AllocationEntry* pIter = nullptr;

    if (LS_UNLIKELY(!_find_or_allocate_entry(blocksNeeded, pIter, pPrev, pOutNumBytes)))
    {
        return nullptr;
    }

    char* result;

    // Sub-allocate from the beginning of a block if we're expecting large
    // allocations to be returned
    #if 1
    {
        const size_type newBlockCount = pIter->numBlocks - blocksNeeded;
        AllocationEntry* LS_RESTRICT_PTR pNext;

        if (newBlockCount > 0)
        {
            pNext = pIter + blocksNeeded;
            pNext->numBlocks = newBlockCount;
            pNext->pNext = pIter->pNext;
            pNext->pSrcPool = pIter->pSrcPool;
            pNext->allocatedBlocks = pIter->allocatedBlocks;
        }
        else
        {
            pNext = pIter->pNext;
        }

        if (LS_UNLIKELY(pPrev != nullptr))
        {
            pPrev->pNext = pNext;
        }
        else if (mHead == pIter)
        {
            mHead = pNext;
        }
        else
        {
            mHead->pNext = pNext;
        }

        pIter->numBlocks = blocksNeeded;
        pIter->pNext = nullptr;

        // Offset the result pointer to ensure we track the memory allocation
        result = reinterpret_cast<char*>(pIter);
    }
    #else // Sub-allocate from the end of a block to reduce fragmentation
    {
        const AllocationEntry iter = *pIter;
        const size_type newBlockCount = iter.numBlocks - blocksNeeded;
        const bool updatePrevPtr = !newBlockCount; // FALSE if the data being allocated begins at the start of an allocation block
        AllocationEntry* const LS_RESTRICT_PTR pResult = pIter + newBlockCount;
        LS_PREFETCH(pResult, LS_PREFETCH_ACCESS_RW, LS_PREFETCH_LEVEL_NONTEMPORAL);

        pIter->numBlocks = newBlockCount;

        if (LS_LIKELY(updatePrevPtr))
        {
            AllocationEntry* next = iter.pNext;

            if (LS_LIKELY(pPrev != nullptr))
            {
                pPrev->pNext = next;
            }
            else if (mHead == pIter)
            {
                mHead = next;
            }
            else
            {
                mHead->pNext = next;
            }
        }

        // Add the allocation size to the result pointer
        const AllocationEntry temp = {blocksNeeded, nullptr, iter.pSrcPool, iter.allocatedBlocks};
        *pResult = temp; // LHS performance penalty mitigated with LS_PREFETCH

        // Offset the result pointer to ensure we track the memory allocation
        result = reinterpret_cast<char*>(pResult);
    }
    #endif

    LS_MEMTRACK_POOL_ALLOC(pResult->pSrcPool, pResult+1, n-header_size);
    return reinterpret_cast<void*>(result + GeneralAllocator<CacheSize, OffsetFreeHeader>::header_size);
}



/*-------------------------------------
 * Calloc
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void* GeneralAllocator<CacheSize, OffsetFreeHeader>::allocate_contiguous(
    size_type numElements,
    size_type numBytesPerElement,
    size_type* pOutNumBytes) noexcept
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
    void* const pData = this->allocate(numBytes, pOutNumBytes);
    if (pData)
    {
        fast_memset(pData, '\0', pOutNumBytes ? *pOutNumBytes : numBytes);
    }

    return pData;
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void* GeneralAllocator<CacheSize, OffsetFreeHeader>::reallocate(
    void* p,
    size_type numNewBytes,
    size_type* pOutNumBytes) noexcept
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
        ret = this->reallocate(p, numNewBytes, numPrevBytes, pOutNumBytes);
    }
    else
    {
        ret = this->allocate(numNewBytes, pOutNumBytes);
    }

    return ret;
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
template <unsigned long long CacheSize, bool OffsetFreeHeader>
inline void* GeneralAllocator<CacheSize, OffsetFreeHeader>::reallocate(
    void* p,
    size_type numNewBytes,
    size_type numPrevBytes,
    size_type* pOutNumBytes) noexcept
{
    if (!numNewBytes)
    {
        if (p)
        {
            this->free(p);
        }

        return nullptr;
    }

    void* const pNewData = this->allocate(numNewBytes, pOutNumBytes);
    if (pNewData)
    {
        if (pOutNumBytes)
        {
            numNewBytes = *pOutNumBytes;
        }

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
    this->_free_impl(reclaimed);
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

    this->_free_impl(reclaimed);
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
