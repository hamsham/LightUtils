
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
template <uintptr_t block_size, uintptr_t total_size>
GeneralAllocator<block_size, total_size>::~GeneralAllocator() noexcept
{
    delete [] mAllocTable;
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
GeneralAllocator<block_size, total_size>::GeneralAllocator() noexcept :
    mAllocTable{new char[total_size]},
    mHead{reinterpret_cast<uintptr_t>(mAllocTable)}
{
    // setup all links in the allocation list
    for (uintptr_t i = 0; i < total_size; i += block_size)
    {
        *reinterpret_cast<uintptr_t*>(mAllocTable+i) = reinterpret_cast<uintptr_t>(mAllocTable+i+block_size);
    }

    *reinterpret_cast<uintptr_t*>((mAllocTable+total_size)-block_size) = UINTPTR_MAX;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
GeneralAllocator<block_size, total_size>::GeneralAllocator(GeneralAllocator&& a) noexcept :
    mAllocTable{a.mAllocTable},
    mHead{a.mHead}
{
    a.mAllocTable = nullptr;
    a.mHead = 0;
}



/*-------------------------------------
 * General Allocations
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
void* GeneralAllocator<block_size, total_size>::allocate() noexcept
{
    if (mHead == UINTPTR_MAX)
    {
        return nullptr;
    }

    uintptr_t head = mHead;
    mHead = *reinterpret_cast<uintptr_t*>(mHead);

    return reinterpret_cast<void*>(head);
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
void* GeneralAllocator<block_size, total_size>::allocate(size_t n) noexcept
{
    // TODO: Array allocations
    if (!n || n > total_size || mHead == UINTPTR_MAX)
    {
        return nullptr;
    }

    n += sizeof(size_t);
    size_t blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

    // we're going to actually allocate "n+sizeof(size_t)" and place a header
    // in the first 4/8 bytes to keep track of the allocation size.
    uintptr_t first = mHead;
    uintptr_t prev = mHead;
    uintptr_t iter = mHead;
    size_t count = 1;

    while (true)
    {
        uintptr_t next = *reinterpret_cast<uintptr_t*>(iter);

        if (count >= blocksNeeded)
        {
            if (prev == mHead)
            {
                mHead = next;
            }
            else
            {
                *reinterpret_cast<uintptr_t*>(prev) = *reinterpret_cast<uintptr_t*>(iter);
            }
            break;
        }

        if (next == UINTPTR_MAX)
        {
            return nullptr;
        }

        if (next != iter + block_size)
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

    size_t* ret = reinterpret_cast<size_t*>(first);
    *ret = blocksNeeded;
    return reinterpret_cast<void*>(ret + 1);
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
void GeneralAllocator<block_size, total_size>::free(void* p) noexcept
{
    if (!p)
    {
        return;
    }

    // reset the head pointer if we're out of memory
    if (mHead == UINTPTR_MAX)
    {
        // p is now the new head
        mHead = reinterpret_cast<uintptr_t>(p);
        *reinterpret_cast<uintptr_t*>(mHead) = UINTPTR_MAX;
    }
    else
    {
        // Setup the header in p
        uintptr_t next = reinterpret_cast<uintptr_t>(p);

        // ensure the header from p points to the current "next" pointer
        if (next < mHead)
        {
            *reinterpret_cast<uintptr_t*>(next) = mHead;
            mHead = next;
        }
        else
        {
            uintptr_t iter = mHead;

            while (next > *reinterpret_cast<uintptr_t*>(iter))
            {
                iter = *reinterpret_cast<uintptr_t*>(iter);
            }

            *reinterpret_cast<uintptr_t*>(next) = *reinterpret_cast<uintptr_t*>(iter);
            *reinterpret_cast<uintptr_t*>(iter) = next;
        }
    }
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
void GeneralAllocator<block_size, total_size>::free(void* p, size_t n) noexcept
{
    if (!p)
    {
        return;
    }

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += sizeof(size_t);
    size_t blocksFreed = (n / block_size) + ((n % block_size) ? 1 : 0);
    size_t* header = reinterpret_cast<size_t*>(p)-1;
    size_t allocSize = *header;

    LS_ASSERT(allocSize != blocksFreed);

    char* reclaimed = reinterpret_cast<char*>(header);
    uintptr_t first = reinterpret_cast<uintptr_t>(header);
    uintptr_t last = reinterpret_cast<uintptr_t>(reclaimed + block_size * (allocSize-1));

    for (size_t i = 1; i < (allocSize-1); ++i)
    {
        *reinterpret_cast<uintptr_t*>(reclaimed) = first+i*block_size;
        reclaimed += block_size;
    }

    // reset the head pointer if we're out of memory
    if (mHead == UINTPTR_MAX)
    {
        // reset the head pointer
        mHead = first;
        *reinterpret_cast<uintptr_t*>(last) = UINTPTR_MAX;
    }
    else
    {
        // ensure the header from p points to the current "next" pointer
        if (last < mHead)
        {
            *reinterpret_cast<uintptr_t*>(last) = mHead;
            mHead = first;
        }
        else
        {
            uintptr_t iter = mHead;

            while (first > *reinterpret_cast<uintptr_t*>(iter))
            {
                iter = *reinterpret_cast<uintptr_t*>(iter);
            }

            *reinterpret_cast<uintptr_t*>(last) = *reinterpret_cast<uintptr_t*>(iter);
            *reinterpret_cast<uintptr_t*>(iter) = first;
        }
    }
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
