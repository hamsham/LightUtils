
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
ChunkAllocator<block_size, total_size>::ChunkAllocator(ChunkAllocator&& a) noexcept :
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
inline void* ChunkAllocator<block_size, total_size>::allocate() noexcept
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
    uintptr_t prev = reinterpret_cast<uintptr_t>(p);

    // ensure the header from p points to the current "next" pointer
    *reinterpret_cast<uintptr_t*>(prev) = mHead;
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
    uintptr_t prev = reinterpret_cast<uintptr_t>(p);

    // ensure the header from p points to the current "next" pointer
    *reinterpret_cast<uintptr_t*>(prev) = reinterpret_cast<uintptr_t>(mHead);
    mHead = prev;
}



} // utils namespace
} // ls namespace

#endif /* LS_UTILS_CHUNK_ALLOCATOR_IMPL_HPP */
