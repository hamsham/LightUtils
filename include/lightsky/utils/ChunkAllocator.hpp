
#ifndef LS_UTILS_CHUNK_ALLOCATOR_HPP
#define LS_UTILS_CHUNK_ALLOCATOR_HPP

#include <cstdint> // uintptr_t, UINTPTR_MAX
#include <cmath>

namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief ChunkAllocator allocates fixed-width chunks of memory up to a
 * specified limit. We are able to prevent memory fragmentation because of the
 * nature of fixed-size memory allocations, however this implementation does
 * not provide methods to allocate arrays of chunks.
 *
 * All memory pointers are placed into a linked-list, using a pointer offset
 * and "next" pointer embedded within the allocated table itself. Once a chunk
 * is allocated, the head and next pointers are modified within the table.
 * This ensures the allocator only uses a fixed number of bytes while providing
 * the full allocation (of "total_size" bytes) to a program at runtime.
 *
 * @tparam block_size
 * The number of bytes to be allocated per chunk.
 *
 * @tparam total_size
 * The total amount of bytes contained required by this allocator.
-----------------------------------------------------------------------------*/
template <uintptr_t block_size, uintptr_t total_size>
class ChunkAllocator
{
    // insurance
    static_assert(block_size >= sizeof(uintptr_t), "Allocation sizes must not be less than sizeof(uintptr_t).");
    static_assert(total_size >= sizeof(uintptr_t), "Allocated memory table cannot be less than sizeof(uintptr_t).");
    static_assert(total_size % block_size == 0,    "Cannot fit the current block size within an allocation table.");
    static_assert(block_size < total_size,         "Block size must be less than the total byte size.");

  private:
    union AllocationEntry
    {
        AllocationEntry* pNext;
        char padding[block_size];
    };

    static_assert(sizeof(AllocationEntry) == block_size, "Allocation entry meta data contains invalid padding.");

    /**
     * @brief Pointer to memory given by the OS.
     */
    char* mAllocTable;

    /**
     * @brief Each chunk of memory contains a reference to the next subsequent
     * chunk of memory in its first set of bytes. "mHead" is an offset to the
     * first available chunk of memory contained within "mAllocTable." Through
     * this, all other chunks in the allocation table can be accessed.
     */
    AllocationEntry* mHead;

  public:
    /**
     * @brief Destructor
     *
     * Returns all memory back to the OS.
     */
    ~ChunkAllocator() noexcept;

    /**
     * @brief Constructor
     *
     * Retrieves "total_size" bytes from the OS and sets up a linked list of
     * pointers within the allocated memory.
     */
    ChunkAllocator() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Deleted to avoid requests for additional memory from the OS.
     */
    ChunkAllocator(const ChunkAllocator&) = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input allocator into *this.
     */
    ChunkAllocator(ChunkAllocator&&) noexcept;

    /**
     * @brief Copy Operator
     *
     * Deleted to prevent invalidation of memory currently allocated to others.
     */
    ChunkAllocator& operator=(const ChunkAllocator&) = delete;

    /**
     * @brief Move Operator
     *
     * Deleted to prevent invalidation of memory currently allocated to others.
     */
    ChunkAllocator& operator=(ChunkAllocator&&) = delete;

    /**
     * @brief Retrieve a contiguous block of memory from *this.
     *
     * The returned block of memory is "block_size" bytes in length.
     *
     * @return A pointer to the first element in the newly allocated memory
     * block. If no more blocks exist within the allocator, NULL is returned.
     */
    void* allocate() noexcept;

    /**
     * @brief Allocate a contiguous block of memory which is at least "n" bytes
     * in size.
     *
     * @note This function can fail (i.e., return NULL) if "n" is greater than
     * or equal to "block_size."
     *
     * @param n
     * The minimum number of bytes to allocate.
     *
     * @return A pointer to the first element in the newly allocated memory
     * block. NULL is returned if not enough more blocks exist within the
     * allocator to fufill the request for memory.
     */
    void* allocate(size_t n) noexcept;

    /**
     * @brief Return an allocated block of memory back to *this allocator.
     *
     * This function does nothing if the input pointer is NULL.
     *
     * @param p
     * A pointer to a block of memory which was returned from a call to
     * "allocate()."
     */
    void free(void* p) noexcept;

    /**
     * @brief Return an allocated block of memory back to *this allocator.
     *
     * This function does nothing if the input pointer is NULL.
     *
     * @param p
     * A pointer to a block of memory which was returned from a call to
     * "allocate()."
     *
     * @param n
     * The number of bytes which were freed. This parameter must match exactly
     * the "n" parameter to allocate(size_t n).
     */
    void free(void* p, size_t n) noexcept;
};



} // utils namespace
} // ls namespace

#include "lightsky/utils/generic/ChunkAllocatorImpl.hpp"




#endif /* LS_UTILS_CHUNK_ALLOCATOR_HPP */
