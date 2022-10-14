
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
    LS_ASSERT(totalSize >= sizeof(size_type)); // "Allocated memory table cannot be less than sizeof(size_type).");
    LS_ASSERT(totalSize % block_size == 0);    // "Cannot fit the current block size within an allocation table.");
    LS_ASSERT(block_size < totalSize);         // "Block size must be less than the total byte size.");

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

    AllocationEntry* reclaimed = reinterpret_cast<AllocationEntry*>(p);
    reclaimed->header.numBlocks = 1;

    char* offset = reinterpret_cast<char*>(reclaimed)+GeneralAllocator<block_size>::header_size;

    this->free(reinterpret_cast<void*>(offset), block_size - GeneralAllocator<block_size>::header_size);
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

    // Offset to the start of the allocation (minus the header).
    char* pData = reinterpret_cast<char*>(p) - GeneralAllocator<block_size>::header_size;

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += GeneralAllocator<block_size>::header_size;
    const size_type blocksFreed = (n / block_size) + ((n % block_size) ? 1 : 0);

    AllocationEntry* reclaimed = reinterpret_cast<AllocationEntry*>(pData);
    const size_type allocSize = reclaimed->header.numBlocks;

    LS_ASSERT(allocSize == blocksFreed);
    LS_ASSERT(reclaimed != mHead); // possible double-free
    // TODO: Add double-free checking

    reclaimed->header.numBlocks = allocSize;
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
        if (mHead == (reclaimed + allocSize))
        {
            reclaimed->header.numBlocks += mHead->header.numBlocks;
            mHead->header.numBlocks = 0;

            reclaimed->header.pNext = mHead->header.pNext;
            mHead->header.pNext = nullptr;
        }
        else
        {
            reclaimed->header.pNext = mHead;
        }

        mHead = reclaimed;
    }
    else
    {
        AllocationEntry* iter = mHead;
        AllocationEntry* prev = nullptr;

        while (iter && reclaimed > iter)
        {
            prev = iter;
            iter = iter->header.pNext;
        }

        if (!prev)
        {
            prev = iter;
            iter = nullptr;
        }

        // expand the previous allocation if possible
        if (reclaimed == (prev + prev->header.numBlocks))
        {
            prev->header.numBlocks += allocSize;

            while (true)
            {
                if (!iter)
                {
                    prev->header.pNext = nullptr;
                }
                else if (iter == (prev + prev->header.numBlocks))
                {
                    prev->header.numBlocks += iter->header.numBlocks;

                    AllocationEntry* temp = iter;
                    iter = iter->header.pNext;

                    temp->header.numBlocks = 0;
                    temp->header.pNext = nullptr;
                    continue;
                }

                break;
            }
        }
        else
        {
            reclaimed->header.pNext = iter;
            prev->header.pNext = reclaimed;
        }
    }
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
