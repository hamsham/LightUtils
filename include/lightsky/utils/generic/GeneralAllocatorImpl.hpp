
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
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator(MemorySource& memorySource) noexcept :
    GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator{memorySource, CacheSize} // delegate
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator(MemorySource& memorySource, size_type initialSize) noexcept :
    Allocator{memorySource}
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
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
GeneralAllocator<BlockSize, CacheSize>::GeneralAllocator(GeneralAllocator&& a) noexcept :
    Allocator{std::move(a)},
    mHead{a.mHead}
{
    a.mHead = nullptr;
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
    }

    return *this;
}



/*-------------------------------------
 * Allocation merging
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void GeneralAllocator<BlockSize, CacheSize>::_merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) const noexcept
{
    pHead->header.numBlocks += pBlock->header.numBlocks;
    pHead->header.pNext = pBlock->header.pNext;

    pBlock->header.numBlocks = 0;
    pBlock->header.pNext = nullptr;
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

    // reset the head pointer if we're out of memory
    if (mHead == nullptr)
    {
        // reset the head pointer
        reclaimed->header.pNext = nullptr;
        mHead = reclaimed;
    }
    else if (reclaimed < mHead)
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        // increase the block count if possible
        reclaimed->header.pNext = mHead;

        if (reclaimed + reclaimed->header.numBlocks == mHead)
        {
            _merge_allocation_blocks(reclaimed, mHead);
        }

        mHead = reclaimed;

        if (mHead->header.numBlocks == mHead->header.allocatedBlocks && mHead->header.pNext)
        {
            AllocationEntry* temp = mHead;
            mHead = mHead->header.pNext;

            const size_type numBytes = temp->header.numBlocks * BlockSize;
            temp->header.numBlocks = 0;
            temp->header.pNext = nullptr;

            this->memory_source().free(temp, numBytes);
        }
    }
    else // mHead > reclaimed
    {
        AllocationEntry* prev = nullptr;
        AllocationEntry* temp = nullptr;
        AllocationEntry* curr = mHead;

        while (reclaimed < curr)
        {
            prev = curr;
            curr = curr->header.pNext;
        }

        reclaimed->header.pNext = curr->header.pNext;
        curr->header.pNext = reclaimed;

        if (reclaimed + reclaimed->header.numBlocks == reclaimed->header.pNext)
        {
            temp = reclaimed;
            _merge_allocation_blocks(reclaimed, reclaimed->header.pNext);
        }

        if (curr + curr->header.numBlocks == curr->header.pNext)
        {
            temp = curr;
            _merge_allocation_blocks(curr, curr->header.pNext);
        }

        if (temp != curr)
        {
            prev = curr;
        }

        if (prev && temp && temp->header.numBlocks == temp->header.allocatedBlocks)
        {
            const size_type numBytes = temp->header.numBlocks * BlockSize;
            prev->header.pNext = temp->header.pNext;

            temp->header.numBlocks = 0;
            temp->header.pNext = nullptr;

            this->memory_source().free(temp, numBytes);
        }
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline typename GeneralAllocator<BlockSize, CacheSize>::AllocationEntry* GeneralAllocator<BlockSize, CacheSize>::_alloc_new_cache(size_type n) noexcept
{
    // round to the next multiple of block_size
    if (n < CacheSize)
    {
        n = CacheSize;
    }

    // Any "n" larger than "CacheSize" gets returned as its own allocation,
    // or round to the next block if there's not enough memory available.
    const size_type cacheRounding = n + (CacheSize - (n % CacheSize));
    const size_type blockRounding = n + (BlockSize - (n % BlockSize));

    size_type allocSize = cacheRounding;
    void* pCache = this->memory_source().allocate(allocSize);
    if (!pCache)
    {
        allocSize = blockRounding;
        pCache = this->memory_source().allocate(allocSize);
        if (!pCache)
        {
            return nullptr;
        }
    }

    const size_type numBlocks = allocSize / BlockSize;

    AllocationEntry* pCacheEntry = reinterpret_cast<AllocationEntry*>(pCache);
    pCacheEntry->header.numBlocks = numBlocks;
    pCacheEntry->header.allocatedBlocks = numBlocks;
    pCacheEntry->header.pNext = nullptr;

    return pCacheEntry;
}



/*-------------------------------------
 * General Allocations
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void* GeneralAllocator<BlockSize, CacheSize>::allocate() noexcept
{
    if (LS_UNLIKELY(mHead == nullptr))
    {
        mHead = _alloc_new_cache(cache_size);
        if (LS_UNLIKELY(mHead == nullptr))
        {
            return nullptr;
        }
    }

    // Single allocation of "block_size". This should be matched to a non-sized
    // deallocation to avoid memory corruption.
    AllocationEntry* iter = mHead;

    if (LS_LIKELY(mHead->header.numBlocks > 1))
    {
        mHead = mHead + 1;
        mHead->header.numBlocks = iter->header.numBlocks - 1;
        mHead->header.pNext = iter->header.pNext;
    }
    else
    {
        mHead = mHead->header.pNext;
    }

    iter->header.numBlocks = 0;
    iter->header.pNext = nullptr;

    return reinterpret_cast<void*>(iter);
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
    size_type blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

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
        AllocationEntry* next = reinterpret_cast<AllocationEntry*>(iter + blocksNeeded);
        next->header.numBlocks = iter->header.numBlocks - blocksNeeded;
        next->header.pNext = iter->header.pNext;
        prev->header.pNext = next;
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
inline void* GeneralAllocator<BlockSize, CacheSize>::allocate_contiguous(size_type numElements) noexcept
{
    return IAllocator::allocate_contiguous(numElements, BlockSize);
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
    if (!p)
    {
        return;
    }

    // Non-sized allocations do not contain a header to ensure the maximum
    // allocation size can be made available.
    this->_free_impl(reinterpret_cast<AllocationEntry*>(p), 1);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void GeneralAllocator<BlockSize, CacheSize>::free(void* p, size_type n) noexcept
{
    if (!p || !n)
    {
        return;
    }

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += GeneralAllocator<BlockSize, CacheSize>::header_size;
    const size_type blocksFreed = (n / block_size) + ((n % block_size) ? 1 : 0);

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<BlockSize, CacheSize>::header_size;

    AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    if (reclaimed == mHead)
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Read the input allocation size for validation
    const size_type allocSize = reclaimed->header.numBlocks;
    if (allocSize != blocksFreed)
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Invalid de-allocation size detected!");
    }

    this->_free_impl(reclaimed, allocSize);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void GeneralAllocator<BlockSize, CacheSize>::free_unsized(void* p) noexcept
{
    if (!p)
    {
        return;
    }

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<BlockSize, CacheSize>::header_size;

    AllocationEntry* const reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    if (reclaimed == mHead)
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Read the input allocation size for validation
    const size_type allocSize = reclaimed->header.numBlocks;
    this->_free_impl(reclaimed, allocSize);
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
