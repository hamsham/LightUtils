
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
template <unsigned long long CacheSize = 4096-(sizeof(unsigned long long)*4), bool OffsetFreeHeader = true>
class GeneralAllocator final : public Allocator
{
  public:
    typedef unsigned long long size_type;

  private:
    struct alignas(alignof(size_type)*4) AllocationEntry
    {
        size_type numBlocks;
        AllocationEntry* pNext;
        AllocationEntry* pSrcPool;
        size_type allocatedBlocks;
    };

    enum : size_type
    {
        header_size = sizeof(AllocationEntry),
        block_size = sizeof(AllocationEntry),
        cache_size = CacheSize
    };

    // insurance
    static_assert(cache_size % block_size == 0, "Cache Size must be a multiple of Block Size.");
    static_assert(sizeof(AllocationEntry) == sizeof(size_type)*4, "Unexpected AllocationHeader size.");
    static_assert(alignof(AllocationEntry) == alignof(size_type)*4, "Unexpected AllocationHeader alignment.");
    static_assert(sizeof(size_type) == sizeof(size_type*), "size_type's size is not sufficient to contain a pointer.");

  private:
    /**
     * @brief Each chunk of memory contains a reference to the next subsequent
     * chunk of memory in its first set of bytes. "mHead" is an offset to the
     * first available chunk of memory contained within "mAllocTable." Through
     * this, all other chunks in the allocation table can be accessed.
     */
    AllocationEntry* mHead;

    /**
     * @brief Convenience member to track the size of the last allocation.
     */
    size_type mLastAllocSize;

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
    AllocationEntry* _merge_allocation_blocks(AllocationEntry* pHead, AllocationEntry* pBlock) const noexcept;

    /**
     * @brief Return an allocated block of memory back to *this allocator.
     *
     * @note This function assumes incoming memory is nonnull and blockCount is
     * nonzero.
     *
     * @param reclaimed
     * A pointer to a valid AllocationEntry struct.
     */
    void _free_impl(AllocationEntry* reclaimed) noexcept;

    /**
     * @brief Refill the internal cache of memory.
     */
    AllocationEntry* _alloc_new_cache(size_type n) noexcept;

    /**
     * @brief Locate an allocation entry with at least \param numBlocksNeeded.
     *
     * @param numBlocksNeeded
     * The number of blocks needed for an allocation.
     *
     * @param A reference to a pointer which will reference the resulting
     * allocation block with at least \param numBlocksNeeded.
     *
     * @param A reference to a pointer which will reference the allocation
     * block immediately preceeding \param pIter.
     *
     * @return TRUE if a sufficient number of blocks could be found or
     * allocated This function will return FALSE otherwise, without placing the
     * result values in \param pIter and \param pPrev.
     */
    bool _find_or_allocate_entry(size_type numBlocksNeeded, AllocationEntry*& pIter, AllocationEntry*& pPrev) noexcept;

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
     * @brief Allocate an array of blocks and zero-initialize the allocation.
     *
     * @param numElements
     * The number of elements of "BlockSize" to allocate.
     *
     * @param numBytesPerElement
     * The size of each element, in bytes, to contiguously allocate.
     *
     * @return A pointer to the newly allocated array, or NULL of the memory
     * allocation failed.
     */
    virtual void* allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept override;

    /**
     * @brief Reallocate a prior allocation, following the rules of
     * std::realloc().
     *
     * @note It is assumed the number of previously allocated bytes pointed at
     * by "p" is equal to "BlockSize".
     *
     * @param p
     * A pointer to an allocation previously generated through allocate().
     *
     * @param numNewBytes
     * The number of bytes needed for the new allocation.
     *
     * @return A pointer to the newly allocated data, or NULL of the
     * reallocation failed.
     */
    virtual void* reallocate(void* p, size_type numNewBytes) noexcept override;

    /**
     * @brief Reallocate a prior allocation, following the rules of
     * std::realloc().
     *
     * @param p
     * A pointer to an allocation previously generated through allocate().
     *
     * @param numNewBytes
     * The number of bytes needed for the new allocation.
     *
     * @param numPrevBytes
     * The number of bytes contained within the prior allocation, pointed at by
     * "p". This value must match exactly to what was passed into "allocate()"
     * or "allocate_contiguous()."
     *
     * @return A pointer to the newly allocated data, or NULL of the
     * reallocation failed.
     */
    virtual void* reallocate(void* p, size_type numNewBytes, size_type numPrevBytes) noexcept override;

    /**
     * @brief Return an allocated block of memory back to *this allocator.
     *
     * @note It is assumed the number of previously allocated bytes pointed at
     * by "p" is equal to "BlockSize".
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
     * the "n" parameter to allocate() or allocate_contiguous().
     */
    virtual void free(void* p, size_type n) noexcept override;
};



} // utils namespace
} // ls namespace

#include "lightsky/utils/generic/GeneralAllocatorImpl.hpp"



extern template class ls::utils::GeneralAllocator<4096-(sizeof(unsigned long long)*4), true>;



#endif /* LS_UTILS_GENERAL_ALLOCATOR_HPP */
