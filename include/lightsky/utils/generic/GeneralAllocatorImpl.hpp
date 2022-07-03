
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
    const size_type numBlocks = totalSize / block_size;

    for (size_type i = 0; i < numBlocks; ++i)
    {
        mHead[i].pNext = mHead+i+1;
    }

    mHead[numBlocks - 1].pNext = nullptr;
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

    AllocationEntry* head = mHead;
    mHead = mHead->pNext;

    return reinterpret_cast<void*>(head);
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <unsigned long long block_size>
void* GeneralAllocator<block_size>::allocate(size_type n) noexcept
{
    // TODO: Array allocations
    if (!n || mHead == nullptr)
    {
        return nullptr;
    }

    n += GeneralAllocator<block_size>::header_size;
    size_type blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

    // we're going to actually allocate "n+sizeof(size_type)" and place a header
    // in the first 4/8 bytes to keep track of the allocation size.
    AllocationEntry* first = mHead;
    AllocationEntry* prev = mHead;
    AllocationEntry* iter = mHead;
    size_type count = 1;

    while (true)
    {
        AllocationEntry* next = iter->pNext;

        if (count >= blocksNeeded)
        {
            if (prev == mHead)
            {
                mHead = next;
            }
            else
            {
                prev->pNext = iter->pNext;
            }
            break;
        }

        if (next == nullptr)
        {
            return nullptr;
        }

        if (next != iter + 1)
        {
            prev = iter;
            first = next;
            count = 1;
        }
        else
        {
            ++count;
        }

        iter = next;
    }

    // Add the allocation size to the result pointer
    first->header = blocksNeeded;

    // offset the result by the allocation header size
    return reinterpret_cast<void*>(&first->memBlock[header_size]);
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

    // reset the head pointer if we're out of memory
    if (mHead == nullptr)
    {
        // p is now the new head
        mHead = reinterpret_cast<AllocationEntry*>(p);
        mHead->pNext = nullptr;
    }
    else
    {
        // Setup the header in p
        AllocationEntry* next = reinterpret_cast<AllocationEntry*>(p);

        // ensure the header from p points to the current "next" pointer
        if (next < mHead)
        {
            next->pNext = mHead;
            mHead = next;
        }
        else
        {
            AllocationEntry* iter = mHead;
            AllocationEntry* prev = mHead;

            while (iter && next > iter->pNext)
            {
                prev = iter;
                iter = iter->pNext;
            }

            if (iter != nullptr)
            {
                next->pNext = iter->pNext;
                iter->pNext = next;
            }
            else
            {
                next->pNext = nullptr;
                prev->pNext = next;
            }
        }
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <unsigned long long block_size>
void GeneralAllocator<block_size>::free(void* p, size_type n) noexcept
{
    if (!p)
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
    const size_type allocSize = reclaimed->header;

    LS_ASSERT(allocSize == blocksFreed);

    // Pointers to the free'd allocation's first and last blocks
    AllocationEntry* first = reclaimed;
    AllocationEntry* last = reclaimed + allocSize - 1;

    for (size_type i = 1; i < (allocSize-1); ++i)
    {
        reclaimed->pNext = first + i;
        reclaimed += 1;
    }

    // reset the head pointer if we're out of memory
    if (mHead == nullptr)
    {
        // reset the head pointer
        mHead = first;
        last->pNext = nullptr;
    }
    else
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        if (last < mHead)
        {
            last->pNext = mHead;
            mHead = first;
        }
        else
        {
            AllocationEntry* iter = mHead;

            while (first > iter->pNext)
            {
                iter = iter->pNext;
            }

            last->pNext = iter->pNext;
            iter->pNext = first;
        }
    }
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
