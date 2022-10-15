
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
template <unsigned long long block_size>
GeneralAllocator<block_size>::~GeneralAllocator() noexcept
{
    delete [] mAllocTable;
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long block_size>
GeneralAllocator<block_size>::GeneralAllocator(size_type totalSize) noexcept
{
    ls::utils::runtime_assert(totalSize >= sizeof(size_type), ls::utils::ErrorLevel::LS_ERROR, "Allocated memory table cannot be less than sizeof(size_type).");
    ls::utils::runtime_assert(totalSize % block_size == 0, ls::utils::ErrorLevel::LS_ERROR, "Cannot fit the current block size within an allocation table.");
    ls::utils::runtime_assert(block_size < totalSize, ls::utils::ErrorLevel::LS_ERROR, "Allocation block size must be less than the total byte size.");

    mAllocTable = new char[totalSize];
    mHead = reinterpret_cast<AllocationEntry*>(mAllocTable);

    // setup all links in the allocation list
    mHead->header.pNext = nullptr;
    mHead->header.numBlocks = totalSize / block_size;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long block_size>
GeneralAllocator<block_size>::GeneralAllocator(GeneralAllocator&& a) noexcept :
    mAllocTable{a.mAllocTable},
    mHead{a.mHead}
{
    a.mAllocTable = nullptr;
    a.mHead = nullptr;
}



/*-------------------------------------
 * Allocation merging
-------------------------------------*/
template <unsigned long long block_size>
inline void GeneralAllocator<block_size>::_merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) noexcept
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
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long block_size>
inline void GeneralAllocator<block_size>::_free_impl(AllocationEntry* reclaimed, size_type blockCount) noexcept
{
    // Minor input validation. Non-sized allocations will not contain an
    // valid block count.
    reclaimed->header.numBlocks = blockCount;
    reclaimed->header.pNext = nullptr;

    // reset the head pointer if we're out of memory
    if (mHead == nullptr)
    {
        // reset the head pointer
        mHead = reclaimed;
    }
    else if (reclaimed < mHead)
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        // increase the block count if possible
        AllocationEntry* const temp = reclaimed;
        this->_merge_allocation_blocks(reclaimed, mHead);
        mHead = temp;
    }
    else // mHead > reclaimed
    {
        AllocationEntry* prev = mHead;
        AllocationEntry* iter = mHead->header.pNext;

        while (iter && reclaimed > iter)
        {
            prev = iter;
            iter = iter->header.pNext;
        }

        // "prev" does not need its "pNext" pointer updated, disable
        // compile-time branch.
        this->_merge_allocation_blocks(prev, reclaimed);
    }
}



/*-------------------------------------
 * General Allocations
-------------------------------------*/
template <unsigned long long block_size>
void* GeneralAllocator<block_size>::allocate() noexcept
{
    if (mHead == nullptr)
    {
        return nullptr;
    }

    // Single allocation of "block_size". This should be matched to a non-sized
    // deallocation to avoid memory corruption.
    AllocationEntry* head = mHead;

    if (mHead->header.numBlocks == 1)
    {
        mHead = nullptr;

        head->header.numBlocks = 0;
        head->header.pNext = nullptr;
    }
    else
    {
        mHead = reinterpret_cast<AllocationEntry*>(reinterpret_cast<char*>(mHead) + block_size);

        mHead->header.numBlocks = head->header.numBlocks - 1;
        head->header.numBlocks = 0;

        mHead->header.pNext = head->header.pNext;
        head->header.pNext = nullptr;
    }

    return reinterpret_cast<void*>(head);
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <unsigned long long block_size>
void* GeneralAllocator<block_size>::allocate(size_type n) noexcept
{
    if (!n || mHead == nullptr)
    {
        return nullptr;
    }

    n += GeneralAllocator<block_size>::header_size;
    size_type blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

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
            return nullptr;
        }

        count = iter->header.numBlocks;
    }

    if (count != blocksNeeded)
    {
        AllocationEntry* next = reinterpret_cast<AllocationEntry*>(iter + blocksNeeded);
        next->header.numBlocks = iter->header.numBlocks - blocksNeeded;
        next->header.pNext = iter->header.pNext;
        prev->header.pNext = next;
    }

    if (mHead == prev)
    {
        mHead = prev->header.pNext;
    }

    // Add the allocation size to the result pointer
    iter->header.numBlocks = blocksNeeded;
    iter->header.pNext = nullptr;

    // Offset the result pointer to ensure we track the memory allocation
    char* result = reinterpret_cast<char*>(iter) + GeneralAllocator<block_size>::header_size;
    return reinterpret_cast<void*>(result);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long block_size>
void GeneralAllocator<block_size>::free(void* p) noexcept
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
template <unsigned long long block_size>
void GeneralAllocator<block_size>::free(void* p, size_type n) noexcept
{
    if (!p || !n)
    {
        return;
    }

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += GeneralAllocator<block_size>::header_size;
    const size_type blocksFreed = (n / block_size) + ((n % block_size) ? 1 : 0);

    // Offset to the start of the allocation (a.k.a. the allocation header).
    char* const pData = reinterpret_cast<char*>(p) - GeneralAllocator<block_size>::header_size;

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
