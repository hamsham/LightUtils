
#ifndef LS_UTILS_CHUNK_ALLOCATOR_IMPL_HPP
#define LS_UTILS_CHUNK_ALLOCATOR_IMPL_HPP

namespace ls
{
namespace utils
{



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
ChunkAllocator<block_size, total_size>::~ChunkAllocator() noexcept
{
    delete [] mAllocTable;
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
ChunkAllocator<block_size, total_size>::ChunkAllocator() noexcept :
    mAllocTable{new char[total_size]},
    mHead{reinterpret_cast<AllocationEntry*>(mAllocTable)}
{
    // setup all links in the allocation list
    for (uintptr_t i = 0; i < (total_size/block_size); ++i)
    {
        mHead[i].pNext = mHead+i+1;
    }

    mHead[(total_size/block_size) - 1].pNext = nullptr;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
ChunkAllocator<block_size, total_size>::ChunkAllocator(ChunkAllocator&& a) noexcept :
    mAllocTable{a.mAllocTable},
    mHead{a.mHead}
{
    a.mAllocTable = nullptr;
    a.mHead = nullptr;
}



/*-------------------------------------
 * General Allocations
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
inline void* ChunkAllocator<block_size, total_size>::allocate() noexcept
{
    if (!mHead)
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
template <uintptr_t block_size, uintptr_t total_size>
inline void* ChunkAllocator<block_size, total_size>::allocate(size_t n) noexcept
{
    if (n <= 0 || n > block_size)
    {
        return nullptr;
    }

    return allocate();
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
inline void ChunkAllocator<block_size, total_size>::free(void* p) noexcept
{
    if (!p)
    {
        return;
    }

    // Setup the header in p
    AllocationEntry* prev = reinterpret_cast<AllocationEntry*>(p);

    // ensure the header from p points to the current "next" pointer
    prev->pNext = mHead;
    mHead = prev;
}



/*-------------------------------------
 * Free Memory
-------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
inline void ChunkAllocator<block_size, total_size>::free(void* p, size_t n) noexcept
{
    (void)n;

    if (!p)
    {
        return;
    }

    // Setup the header in p
    AllocationEntry* prev = reinterpret_cast<AllocationEntry*>(p);

    // ensure the header from p points to the current "next" pointer
    prev->pNext = mHead;
    mHead = prev;
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_CHUNK_ALLOCATOR_IMPL_HPP */
