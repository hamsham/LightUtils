
#ifndef LS_UTILS_GENERAL_ALLOCATOR_HPP
#define LS_UTILS_GENERAL_ALLOCATOR_HPP

namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief GeneralAllocator allocates fixed-width chunks of memory up to a
 * specified limit. We are able to prevent memory fragmentation because of the
 * nature of fixed-size memory allocations. All chunks are sorted upon deletion
 * to ensure array allocations may still be made using the best-fit of
 * contiguous memory.
 *
 * All memory pointers are placed into a linked-list, using a pointer offset
 * and "next" pointer embedded within the allocated table itself. Once a chunk
 * is allocated, the head and next pointers are modified within the table.
 * This ensures the allocator only uses a fixed number of bytes while providing
 * the full allocation (of "total_size" bytes) to a program at runtime.
 *
 * @tparam block_size
 * The number of bytes to be allocated per chunk.
-----------------------------------------------------------------------------*/
template <unsigned long long block_size>
class GeneralAllocator
{
  public:
    typedef unsigned long long size_type;

  private:
    union alignas(alignof(size_type)) AllocationEntry;

    struct alignas(alignof(size_type)) AllocationHeader
    {
        size_type numBlocks;
        AllocationEntry* pNext;
    };

    // Use the header type for alignment. If needed we can change this to align
    // to a SIMD-sized block.
    union alignas(alignof(AllocationHeader)) AllocationEntry
    {
        AllocationHeader header;
        char memBlock[block_size];
    };

    enum : size_type
    {
        header_size = sizeof(AllocationHeader)
    };

    // insurance
    static_assert(block_size >= header_size, "Allocation sizes must be less than sizeof(header_type).");
    static_assert(sizeof(size_type) == sizeof(size_type*), "size_type's size is not sufficient to contain a pointer.");
    static_assert(sizeof(AllocationEntry) == block_size, "Allocation entry meta data contains invalid padding.");

  private:
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

    /**
     * @brief Helper function to merge two contiguous memory blocks.
     *
     * @note This function assumes incoming memory is nonnull.
     *
     * @param pHead
     * Pointer the the base memory block.
     *
     * @param pBlock
     * Pointer to memory block coming from the client API.
     *
     * @return A pointer to the head block.
     */
    void _merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) noexcept;

    /**
     * @brief Return an allocated block of memory back to *this allocator.
     *
     * @note This function assumes incoming memory is nonnull and blockCount is
     * nonzero.
     *
     * @param reclaimed
     * A pointer to a valid AllocationEntry struct.
     *
     * @param blockCount
     * The number of blocks which were freed (not number of bytes).
     */
    void _free_impl(AllocationEntry* reclaimed, size_type blockCount) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Returns all memory back to the OS.
     */
    ~GeneralAllocator() noexcept;

    /**
     * @brief Default Constructor
     *
     * Deleted so we force a total allocation size at construction time.
     */
    GeneralAllocator() noexcept = delete;

    /**
     * @brief Constructor
     *
     * Retrieves "total_size" bytes from the OS and sets up a linked list of
     * pointers within the allocated memory.
     */
    GeneralAllocator(size_type totalSize) noexcept;

    /**
     * @brief Copy Constructor
     *
     * Deleted to avoid requests for additional memory from the OS.
     */
    GeneralAllocator(const GeneralAllocator&) = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input allocator into *this.
     */
    GeneralAllocator(GeneralAllocator&&) noexcept;

    /**
     * @brief Copy Operator
     *
     * Deleted to prevent invalidation of memory currently allocated to others.
     */
    GeneralAllocator& operator=(const GeneralAllocator&) = delete;

    /**
     * @brief Move Operator
     *
     * Deleted to prevent invalidation of memory currently allocated to others.
     */
    GeneralAllocator& operator=(GeneralAllocator&&) = delete;

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
    void* allocate(size_type n) noexcept;

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
     * the "n" parameter to allocate(size_type n).
     */
    void free(void* p, size_type n) noexcept;
};



} // utils namespace
} // ls namespace

#include "lightsky/utils/generic/GeneralAllocatorImpl.hpp"



extern template class ls::utils::GeneralAllocator<sizeof(unsigned long long)*4>;



#endif /* LS_UTILS_GENERAL_ALLOCATOR_HPP */
