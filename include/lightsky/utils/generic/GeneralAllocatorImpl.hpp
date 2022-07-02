
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
    mHead = reinterpret_cast<size_type>(mAllocTable);

    // setup all links in the allocation list
    for (size_type i = 0; i < totalSize; i += block_size)
    {
        *reinterpret_cast<size_type*>(mAllocTable+i) = reinterpret_cast<size_type>(mAllocTable+i+block_size);
    }

    *reinterpret_cast<size_type*>((mAllocTable+totalSize)-block_size) = GeneralAllocator<block_size>::null_value;
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
    a.mHead = GeneralAllocator<block_size>::null_value;
}



/*-------------------------------------
 * General Allocations
-------------------------------------*/
template <unsigned long long block_size>
void* GeneralAllocator<block_size>::allocate() noexcept
{
    if (mHead == GeneralAllocator<block_size>::null_value)
    {
        return nullptr;
    }

    size_type head = mHead;
    mHead = *reinterpret_cast<size_type*>(mHead);

    return reinterpret_cast<void*>(head);
}



/*-------------------------------------
 * Array Allocations
-------------------------------------*/
template <unsigned long long block_size>
void* GeneralAllocator<block_size>::allocate(size_type n) noexcept
{
    // TODO: Array allocations
    if (!n || mHead == GeneralAllocator<block_size>::null_value)
    {
        return nullptr;
    }

    n += GeneralAllocator<block_size>::header_size;
    size_type blocksNeeded = (n / block_size) + ((n % block_size) ? 1 : 0);

    // we're going to actually allocate "n+sizeof(size_type)" and place a header
    // in the first 4/8 bytes to keep track of the allocation size.
    size_type first = mHead;
    size_type prev = mHead;
    size_type iter = mHead;
    size_type count = 1;

    while (true)
    {
        size_type next = *reinterpret_cast<size_type*>(iter);

        if (count >= blocksNeeded)
        {
            if (prev == mHead)
            {
                mHead = next;
            }
            else
            {
                *reinterpret_cast<size_type*>(prev) = *reinterpret_cast<size_type*>(iter);
            }
            break;
        }

        if (next == GeneralAllocator<block_size>::null_value)
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

    // Add the allocation size to the result pointer
    header_type* ret = reinterpret_cast<header_type*>(first);
    *ret = blocksNeeded;

    // offset the result by the allocation header size
    char* pRet = reinterpret_cast<char*>(ret) + GeneralAllocator<block_size>::header_size;
    return reinterpret_cast<void*>(pRet);
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
    if (mHead == GeneralAllocator<block_size>::null_value)
    {
        // p is now the new head
        mHead = reinterpret_cast<size_type>(p);
        *reinterpret_cast<size_type*>(mHead) = GeneralAllocator<block_size>::null_value;
    }
    else
    {
        // Setup the header in p
        size_type next = reinterpret_cast<size_type>(p);

        // ensure the header from p points to the current "next" pointer
        if (next < mHead)
        {
            *reinterpret_cast<size_type*>(next) = mHead;
            mHead = next;
        }
        else
        {
            size_type iter = mHead;

            while (next > *reinterpret_cast<size_type*>(iter))
            {
                iter = *reinterpret_cast<size_type*>(iter);
            }

            *reinterpret_cast<size_type*>(next) = *reinterpret_cast<size_type*>(iter);
            *reinterpret_cast<size_type*>(iter) = next;
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

    // retrieve the allocation size from header, make sure it matches what is
    // expected
    n += GeneralAllocator<block_size>::header_size;
    size_type blocksFreed = (n / block_size) + ((n % block_size) ? 1 : 0);
    header_type* header = reinterpret_cast<header_type*>(p)-1;
    size_type allocSize = *header;

    LS_ASSERT(allocSize == blocksFreed);

    char* reclaimed = reinterpret_cast<char*>(header);
    size_type first = reinterpret_cast<size_type>(header);
    size_type last = reinterpret_cast<size_type>(reclaimed + block_size * (allocSize-1));

    for (size_type i = 1; i < (allocSize-1); ++i)
    {
        *reinterpret_cast<size_type*>(reclaimed) = first+i*block_size;
        reclaimed += block_size;
    }

    // reset the head pointer if we're out of memory
    if (mHead == GeneralAllocator<block_size>::null_value)
    {
        // reset the head pointer
        mHead = first;
        *reinterpret_cast<size_type*>(last) = GeneralAllocator<block_size>::null_value;
    }
    else
    {
        // ensure the header-pointer from p points to the current "next" chunk.
        if (last < mHead)
        {
            *reinterpret_cast<size_type*>(last) = mHead;
            mHead = first;
        }
        else
        {
            size_type iter = mHead;

            while (first > *reinterpret_cast<size_type*>(iter))
            {
                iter = *reinterpret_cast<size_type*>(iter);
            }

            *reinterpret_cast<size_type*>(last) = *reinterpret_cast<size_type*>(iter);
            *reinterpret_cast<size_type*>(iter) = first;
        }
    }
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_GENERIC_ALLOCATOR_IMPL_HPP */
