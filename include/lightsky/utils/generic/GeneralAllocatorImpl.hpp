
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
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::~GeneralAllocator() noexcept
{
    AllocationEntry* iter = mHead;

    while (iter)
    {
        // This had to be disabled in threaded builds due to allocating/freeing
        // in different threads. It should be enabled when testing single
        // threaded though
        //LS_ASSERT(iter->header.numBlocks >= CacheSize/BlockSize);

        AllocationEntry* temp = iter;
        iter = iter->header.pNext;

        temp->header.numBlocks = 0;
        temp->header.pNext = nullptr;

        this->memory_source().free(temp, temp->header.numBlocks * BlockSize);
    }

    mHead = nullptr;
    mTotalBlocksAllocd = 0;
    mLastAllocSize = 0;
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator(MemorySource& memorySource) noexcept :
    //GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator{memorySource, CacheSize} // delegate
    Allocator{memorySource},
    mHead{nullptr},
    mTotalBlocksAllocd{0},
    mLastAllocSize{0}
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator(MemorySource& memorySource, size_type initialSize) noexcept :
    Allocator{memorySource},
    mHead{nullptr},
    mTotalBlocksAllocd{0},
    mLastAllocSize{0}
{
    ls::utils::runtime_assert(initialSize >= sizeof(size_type), ls::utils::ErrorLevel::LS_ERROR, "Allocated memory table cannot be less than sizeof(size_type).");
    ls::utils::runtime_assert(initialSize % BlockSize == 0, ls::utils::ErrorLevel::LS_ERROR, "Cannot fit the current block size within an allocation table.");
    ls::utils::runtime_assert(BlockSize < initialSize, ls::utils::ErrorLevel::LS_ERROR, "Allocation block size must be less than the total byte size.");

    mHead = reinterpret_cast<AllocationEntry*>(this->memory_source().allocate(initialSize));
    if (mHead)
    {
        // setup all links in the allocation list
        size_type numBlocks = initialSize / BlockSize;

        mHead->header.pNext = nullptr;
        mHead->header.numBlocks = numBlocks;
        mHead->header.allocatedBlocks = numBlocks;
        mTotalBlocksAllocd = numBlocks;
        mLastAllocSize = initialSize;
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator(GeneralAllocator&& a) noexcept :
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
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>& GeneralAllocator<BlockSize, CacheSize>::operator=(GeneralAllocator&& a) noexcept
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
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void GeneralAllocator<BlockSize, CacheSize>::_merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) const noexcept
{
    // Double-free detection
    if (LS_UNLIKELY((pHead < pBlock) && ((pHead + pHead->header.numBlocks) > pBlock)))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    if (pHead + pHead->header.numBlocks == pBlock)
    {
        pHead->header.numBlocks += pBlock->header.numBlocks;
        pHead->header.pNext = pBlock->header.pNext;

        pBlock->header.numBlocks = 0;
        pBlock->header.pNext = nullptr;
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void GeneralAllocator<BlockSize, CacheSize>::_free_impl(AllocationEntry* reclaimed, size_type blockCount) noexcept
{
    // Minor input validation. Non-sized allocations will not contain an
    // valid block count.
    reclaimed->header.numBlocks = blockCount;
    reclaimed->header.pNext = nullptr;

    // reset the head pointer if we're out of memory
    if (mHead == nullptr)
    {
        // reset the head pointer
        reclaimed->header.pNext = nullptr;
        mHead = reclaimed;
        return;
    }

    AllocationEntry* prev = nullptr;
    AllocationEntry* temp = reclaimed;

    if (mHead > reclaimed)
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        // increase the block count if possible
        reclaimed->header.pNext = mHead;

        _merge_allocation_blocks(reclaimed, mHead);

        prev = nullptr;
        mHead = reclaimed;
    }
    else // mHead < reclaimed
    {
        AllocationEntry* curr = mHead;

        while (reclaimed < curr)
        {
            prev = curr;
            curr = curr->header.pNext;
        }

        reclaimed->header.pNext = curr->header.pNext;
        curr->header.pNext = reclaimed;

        _merge_allocation_blocks(curr, curr->header.pNext);
        _merge_allocation_blocks(reclaimed, reclaimed->header.pNext);

        // Move our cursors depending on whether "reclaimed" was merged into
        // the "curr" allocation block.
        if (LS_LIKELY(!reclaimed->header.numBlocks))
        {
            temp = curr;
        }
        else
        {
            prev = curr;
        }
    }

    if (temp->header.numBlocks == temp->header.allocatedBlocks && temp->header.pNext)
    {
        const size_type numBlocks = temp->header.numBlocks;
        const size_type numBytes = numBlocks * BlockSize;

        if (LS_LIKELY(temp != mHead))
        {
            prev->header.pNext = temp->header.pNext;
        }
        else
        {
            mHead = mHead->header.pNext;
        }

        mTotalBlocksAllocd -= numBlocks;

        temp->header.numBlocks = 0;
        temp->header.pNext = nullptr;

        this->memory_source().free(temp, numBytes);
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline typename GeneralAllocator<BlockSize, CacheSize>::AllocationEntry* GeneralAllocator<BlockSize, CacheSize>::_alloc_new_cache(size_type n) noexcept
{
    // Any allocation larger than the block size gets rounded to the next cache
    // size. If that fails, we try again by rounding to the next block. Should
    // that also fail, we bail completely.
    const size_type maxBytes0 = mLastAllocSize * 2;
    const size_type maxBytes1 = mTotalBlocksAllocd / 2;
    const size_type t = (n < maxBytes0) ? maxBytes0 : (n <= maxBytes1 ? maxBytes1 : n);
    const size_type reserveRounding = t + (CacheSize - (t % CacheSize));
    const size_type cacheRounding   = n + (CacheSize - (n % CacheSize));
    const size_type blockRounding   = n + (BlockSize - (n % BlockSize));

    size_type allocSize;
    void* pCache;

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

    const size_type numBlocks = allocSize / BlockSize;
    mTotalBlocksAllocd += numBlocks;
    mLastAllocSize = allocSize;

    AllocationEntry* pCacheEntry = reinterpret_cast<AllocationEntry*>(pCache);
    pCacheEntry->header.numBlocks = numBlocks;
    pCacheEntry->header.allocatedBlocks = numBlocks;
    pCacheEntry->header.pNext = nullptr;

    return pCacheEntry;
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void* GeneralAllocator<BlockSize, CacheSize>::allocate(size_type n) noexcept
{
    if (!n)
    {
        return nullptr;
    }

    n += GeneralAllocator<BlockSize, CacheSize>::header_size;
    const size_type blocksNeeded = (n / BlockSize) + ((n % BlockSize) ? 1 : 0);

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
    AllocationEntry* prev = mHead;
    AllocationEntry* iter = mHead;
    size_type count = mHead->header.numBlocks;

    while (count < blocksNeeded)
    {
        prev = iter;
        iter = iter->header.pNext;

        if (LS_UNLIKELY(!iter))
        {
            iter = _alloc_new_cache(n);
            if (LS_UNLIKELY(!iter))
            {
                return nullptr;
            }

            prev->header.pNext = iter;
        }

        count = iter->header.numBlocks;
    }

    if (count > blocksNeeded)
    {
        //AllocationEntry* next = reinterpret_cast<AllocationEntry*>(iter + blocksNeeded);
        //next->header.numBlocks = iter->header.numBlocks - blocksNeeded;
        //next->header.pNext = iter->header.pNext;
        //prev->header.pNext = next;

        AllocationEntry* temp = iter + (iter->header.numBlocks - blocksNeeded);
        iter->header.numBlocks -= blocksNeeded;
        iter = temp;
    }
    else // count == blocksNeeded
    {
        AllocationEntry* next = iter->header.pNext;
        prev->header.pNext = next;
    }

    if (mHead == iter)
    {
        mHead = iter->header.pNext;
    }

    // Add the allocation size to the result pointer
    iter->header.numBlocks = blocksNeeded;
    iter->header.pNext = nullptr;

    // Offset the result pointer to ensure we track the memory allocation
    char* result = reinterpret_cast<char*>(iter) + GeneralAllocator<BlockSize, CacheSize>::header_size;
    return reinterpret_cast<void*>(result);
}



/*-------------------------------------
 * Calloc
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void* GeneralAllocator<BlockSize, CacheSize>::allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept
{
    return IAllocator::allocate_contiguous(numElements, numBytesPerElement);
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void* GeneralAllocator<BlockSize, CacheSize>::reallocate(void* p, size_type numNewBytes) noexcept
{
    return IAllocator::reallocate(p, numNewBytes, BlockSize);
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void* GeneralAllocator<BlockSize, CacheSize>::reallocate(void* p, size_type numNewBytes, size_type numPrevBytes) noexcept
{
    return IAllocator::reallocate(p, numNewBytes, numPrevBytes);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void GeneralAllocator<BlockSize, CacheSize>::free(void* p) noexcept
{
    if (LS_UNLIKELY(!p))
    {
        return;
    }

    // Non-sized allocations do not contain a header to ensure the maximum
    // allocation size can be made available.
    //this->free_raw(reinterpret_cast<AllocationEntry*>(p), 1);

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<BlockSize, CacheSize>::header_size;

    AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    if (LS_UNLIKELY(reclaimed == mHead))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Read the input allocation size for validation
    const size_type allocSize = reclaimed->header.numBlocks;
    this->_free_impl(reclaimed, allocSize);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void GeneralAllocator<BlockSize, CacheSize>::free(void* p, size_type n) noexcept
{
    if (LS_UNLIKELY(!p || !n))
    {
        return;
    }

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += GeneralAllocator<BlockSize, CacheSize>::header_size;
    const size_type blocksFreed = (n / BlockSize) + ((n % BlockSize) ? 1 : 0);

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<BlockSize, CacheSize>::header_size;

    AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    if (LS_UNLIKELY(reclaimed == mHead))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Read the input allocation size for validation
    const size_type allocSize = reclaimed->header.numBlocks;
    if (LS_UNLIKELY(allocSize != blocksFreed))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Invalid de-allocation size detected!");
    }

    this->_free_impl(reclaimed, allocSize);
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
