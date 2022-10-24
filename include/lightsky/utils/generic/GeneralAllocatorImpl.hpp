
#ifndef LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP
#define LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP

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
        LS_ASSERT(iter->header.numBlocks >= CacheSize/BlockSize);

        AllocationEntry* temp = iter;
        iter = iter->header.pNext;

        this->memory_source().free(temp, temp->header.numBlocks * BlockSize);
    }
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
    ls::utils::runtime_assert(initialSize % block_size == 0, ls::utils::ErrorLevel::LS_ERROR, "Cannot fit the current block size within an allocation table.");
    ls::utils::runtime_assert(block_size < initialSize, ls::utils::ErrorLevel::LS_ERROR, "Allocation block size must be less than the total byte size.");

    mHead = reinterpret_cast<AllocationEntry*>(this->memory_source().allocate(initialSize));

    // setup all links in the allocation list
    size_type numBlocks = initialSize / block_size;

    mHead->header.pNext = nullptr;
    mHead->header.numBlocks = numBlocks;
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
inline void GeneralAllocator<BlockSize, CacheSize>::_merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) noexcept
{
    const size_type headSize = pHead->header.numBlocks;

    // Double-free detection
    if ((pHead >= pBlock) && (pBlock < (pHead + headSize)))
    {
        ls::utils::runtime_assert(false, ls::utils::ErrorLevel::LS_ERROR, "Double-free deallocation detected!");
    }

    // Check if the head pointer can be expanded
    AllocationEntry* iter = pHead->header.pNext;

    if (pBlock == (pHead + pHead->header.numBlocks))
    {
        pHead->header.numBlocks += pBlock->header.numBlocks;
        pBlock->header.numBlocks = 0;
        pBlock->header.pNext = nullptr;

        if (!iter)
        {
            pHead->header.pNext = nullptr;
        }
        else if (iter == (pHead + pHead->header.numBlocks))
        {
            pHead->header.numBlocks += iter->header.numBlocks;
            pHead->header.pNext = iter->header.pNext;

            iter->header.numBlocks = 0;
            iter->header.pNext = nullptr;
        }
    }
    else
    {
        pBlock->header.pNext = iter;
        pHead->header.pNext = pBlock;

        AllocationEntry* next = pBlock->header.pNext;
        if (next == (pHead + pHead->header.numBlocks))
        {
            pBlock->header.numBlocks += next->header.numBlocks;
            next->header.numBlocks = 0;

            pBlock->header.pNext = next->header.pNext;
            next->header.pNext = nullptr;
        }
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
inline void GeneralAllocator<BlockSize, CacheSize>::_free_impl(AllocationEntry* reclaimed, size_type blockCount) noexcept
{
    constexpr size_type BlocksPerCache = CacheSize / BlockSize;

    // Minor input validation. Non-sized allocations will not contain an
    // valid block count.
    reclaimed->header.numBlocks = blockCount;
    reclaimed->header.pNext = nullptr;

    // reset the head pointer if we're out of memory
    if (mHead == nullptr)
    {
        // reset the head pointer
        mHead = reclaimed;
        return;
    }

    if (reclaimed < mHead)
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        // increase the block count if possible
        AllocationEntry* const temp = reclaimed;
        this->_merge_allocation_blocks(reclaimed, mHead);
        mHead = temp;

        if (BlocksPerCache <= mHead->header.numBlocks)
        {
            mHead = mHead->header.pNext;
            this->memory_source().free(temp, temp->header.numBlocks * BlockSize);
        }

        return;
    }

    //else: mHead > reclaimed
    AllocationEntry* prev = nullptr;
    AllocationEntry* curr = mHead;
    AllocationEntry* iter = mHead->header.pNext;

    while (iter && reclaimed > iter)
    {
        prev = curr;
        curr = iter;
        iter = iter->header.pNext;
    }

    // "prev" does not need its "pNext" pointer updated, disable
    // compile-time branch.
    this->_merge_allocation_blocks(curr, reclaimed);

    if (prev && BlocksPerCache <= curr->header.numBlocks)
    {
        prev->header.pNext = curr->header.pNext;
        this->memory_source().free(curr, curr->header.numBlocks * BlockSize);
    }

    if (BlocksPerCache <= reclaimed->header.numBlocks)
    {
        curr->header.pNext = reclaimed->header.pNext;
        this->memory_source().free(reclaimed, reclaimed->header.numBlocks * BlockSize);
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

    // Any "n" larger than "CacheSize" gets returned as its own allocation.
    size_type n1 = block_size * ((n / block_size) + ((n % block_size) ? 1 : 0));
    size_type n2 = n/2;

    size_type m = n1 + n2;
    void* pCache = this->memory_source().allocate(m);
    if (!pCache)
    {
        m = n1;
        pCache = this->memory_source().allocate(m);
        if (!pCache)
        {
            m = n;
            pCache = this->memory_source().allocate(m);
            if (!pCache)
            {
                return nullptr;
            }
        }
    }

    n = m;

    AllocationEntry* pCacheEntry = reinterpret_cast<AllocationEntry*>(pCache);
    pCacheEntry->header.numBlocks = n / block_size;
    pCacheEntry->header.pNext = nullptr;

    return pCacheEntry;
}



/*-------------------------------------
 * General Allocations
-------------------------------------*/
template <unsigned long long BlockSize, unsigned long long CacheSize>
void* GeneralAllocator<BlockSize, CacheSize>::allocate() noexcept
{
    if (mHead == nullptr)
    {
        mHead = _alloc_new_cache(cache_size);
        if (mHead == nullptr)
        {
            return nullptr;
        }
    }

    // Single allocation of "block_size". This should be matched to a non-sized
    // deallocation to avoid memory corruption.
    AllocationEntry* iter = mHead;

    if (mHead->header.numBlocks == 1)
    {
        mHead = mHead->header.pNext;

        iter->header.numBlocks = 0;
        iter->header.pNext = nullptr;
    }
    else
    {
        mHead = mHead + 1;
        mHead->header.numBlocks = iter->header.numBlocks - 1;
        iter->header.numBlocks = 0;

        mHead->header.pNext = iter->header.pNext;
        iter->header.pNext = nullptr;
    }

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

        if (!iter)
        {
            iter = _alloc_new_cache(n);
            if (!iter)
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



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
