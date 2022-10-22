
#ifndef LS_UTILS_GENERAL_ALLOCATOR_HPP
#define LS_UTILS_GENERAL_ALLOCATOR_HPP

#include "lightsky/utils/Allocator.hpp"

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
template <unsigned long long BlockSize = 16, unsigned long long CacheSize = 4096>
class GeneralAllocator final : public Allocator
{
  public:
    typedef unsigned long long size_type;

  private:
    union alignas(alignof(size_type)*2) AllocationEntry;

    struct alignas(alignof(size_type)*2) AllocationHeader
    {
        size_type numBlocks;
        AllocationEntry* pNext;
    };

    enum : size_type
    {
        header_size = sizeof(AllocationHeader),
        block_size = BlockSize,
        cache_size = CacheSize
    };

    // Use the header type for alignment. If needed we can change this to align
    // to a SIMD-sized block.
    union alignas(alignof(AllocationHeader)) AllocationEntry
    {
        AllocationHeader header;
        char memBlock[block_size];
    };

    // insurance
    static_assert(CacheSize % BlockSize == 0, "Cache Size must be a multiple of Block Size.");
    static_assert(BlockSize >= 16, "Block size must be a minimum of 32 bytes.");
    static_assert(block_size >= header_size, "Allocation sizes must be less than sizeof(header_type).");
    static_assert(sizeof(size_type) == sizeof(size_type*), "size_type's size is not sufficient to contain a pointer.");
    static_assert(sizeof(AllocationEntry) == block_size, "Allocation entry meta data contains invalid padding.");

  private:
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

    /**
     * @brief Refill the internal cache of memory.
     */
    AllocationEntry* _alloc_new_cache(size_type n) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Returns all memory back to the OS.
     */
    virtual ~GeneralAllocator() noexcept override;

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
    GeneralAllocator(MemorySource& memorySource) noexcept;

    /**
     * @brief Constructor
     *
     * Retrieves "initialSize" bytes from the OS and sets up a linked list of
     * pointers within the allocated memory.
     */
    GeneralAllocator(MemorySource& memorySource, size_type initialSize) noexcept;

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
    GeneralAllocator& operator=(GeneralAllocator&&) noexcept;

    /**
     * @brief Retrieve a contiguous block of memory from *this.
     *
     * The returned block of memory is "block_size" bytes in length.
     *
     * @return A pointer to the first element in the newly allocated memory
     * block. If no more blocks exist within the allocator, NULL is returned.
     */
    virtual void* allocate() noexcept override;

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
    virtual void* allocate(size_type n) noexcept override;

    /**
     * @brief Return an allocated block of memory back to *this allocator.
     *
     * This function does nothing if the input pointer is NULL.
     *
     * @param p
     * A pointer to a block of memory which was returned from a call to
     * "allocate()."
     */
    virtual void free(void* p) noexcept override;

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
    virtual void free(void* p, size_type n) noexcept override;
};



} // utils namespace
} // ls namespace

#include "lightsky/utils/generic/GeneralAllocatorImpl.hpp"



extern template class ls::utils::GeneralAllocator<16, 4096>;



#endif /* LS_UTILS_GENERAL_ALLOCATOR_HPP */
