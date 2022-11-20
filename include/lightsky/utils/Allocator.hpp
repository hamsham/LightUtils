/*
 * File:   Allocator.hpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:02 PM
 */

#ifndef LS_UTILS_ALLOCATOR_HPP
#define LS_UTILS_ALLOCATOR_HPP

#include "lightsky/setup/Types.h"

#include "lightsky/utils/MemorySource.hpp"
#include "lightsky/utils/SpinLock.hpp"



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * IAllocator
-----------------------------------------------------------------------------*/
class IAllocator : public MemorySource
{
  public:
    static constexpr bool calloc_can_overflow(size_type numElements, size_type numBytesPerElement) noexcept;

  protected:
    virtual const MemorySource& memory_source() const noexcept = 0;

    virtual MemorySource& memory_source() noexcept = 0;

  public:
    virtual ~IAllocator() noexcept = 0;

    virtual void* allocate(size_type n) noexcept;

    virtual void* allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept;

    virtual void* reallocate(void* p, size_type numNewBytes) noexcept;

    virtual void* reallocate(void* p, size_type numNewBytes, size_type numPrevBytes) noexcept;

    virtual void free(void* p) noexcept;

    virtual void free(void* p, size_type n) noexcept;
};



/*-----------------------------------------------------------------------------
 * Allocator
-----------------------------------------------------------------------------*/
class Allocator : public IAllocator
{
  private:
    MemorySource* mMemSource;

  protected:
    const MemorySource& memory_source() const noexcept override;

    MemorySource& memory_source() noexcept override;

  public:
    virtual ~Allocator() noexcept override;

    Allocator() noexcept = delete;

    Allocator(MemorySource& src) noexcept;

    Allocator(const Allocator&) = delete;

    Allocator(Allocator&& allocator) noexcept;

    Allocator& operator=(const Allocator& allocator) noexcept = delete;

    Allocator& operator=(Allocator&& allocator) noexcept;
};



/*-----------------------------------------------------------------------------
 * Constrained Allocator
-----------------------------------------------------------------------------*/
template <unsigned long long MaxNumBytes>
class ConstrainedAllocator : public Allocator
{
  private:
    size_type mBytesAllocated;

  public:
    virtual ~ConstrainedAllocator() noexcept override;

    ConstrainedAllocator() noexcept = delete;

    ConstrainedAllocator(MemorySource& memorySource) noexcept;

    ConstrainedAllocator(const ConstrainedAllocator& src) noexcept = delete;

    ConstrainedAllocator(ConstrainedAllocator&& src) noexcept;

    ConstrainedAllocator& operator=(const ConstrainedAllocator&) noexcept = delete;

    ConstrainedAllocator& operator=(ConstrainedAllocator&&) noexcept;

    virtual void* allocate(size_type numBytes) noexcept override;

    virtual void* allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept override;

    virtual void free(void* pData) noexcept override;

    virtual void free(void* pData, size_type numBytes) noexcept override;
};



/*-----------------------------------------------------------------------------
 * Constrained Allocator (specialized for run-time constraints)
-----------------------------------------------------------------------------*/
template <>
class ConstrainedAllocator<0> : public Allocator
{
  private:
    size_type mBytesAllocated;
    size_type mMaxAllocSize;

  public:
    virtual ~ConstrainedAllocator() noexcept override;

    ConstrainedAllocator() noexcept = delete;

    ConstrainedAllocator(MemorySource& memorySource, size_type maxNumBytes) noexcept;

    ConstrainedAllocator(const ConstrainedAllocator& src) noexcept = delete;

    ConstrainedAllocator(ConstrainedAllocator&& src) noexcept;

    ConstrainedAllocator& operator=(const ConstrainedAllocator&) noexcept = delete;

    ConstrainedAllocator& operator=(ConstrainedAllocator&&) noexcept;

    virtual void* allocate(size_type numBytes) noexcept override;

    virtual void* allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept override;

    virtual void free(void* pData) noexcept override;

    virtual void free(void* pData, size_type numBytes) noexcept override;
};



/*-----------------------------------------------------------------------------
 * Block Allocator
-----------------------------------------------------------------------------*/
template <unsigned long long BlockSize>
class BlockAllocator : public Allocator
{
  public:
    virtual ~BlockAllocator() noexcept override;

    BlockAllocator() noexcept = delete;

    BlockAllocator(MemorySource& memorySource) noexcept;

    BlockAllocator(const BlockAllocator& src) noexcept = delete;

    BlockAllocator(BlockAllocator&& src) noexcept;

    BlockAllocator& operator=(const BlockAllocator&) noexcept = delete;

    BlockAllocator& operator=(BlockAllocator&&) noexcept;

    virtual void* allocate(size_type numBytes) noexcept override;

    virtual void* allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept override;

    virtual void free(void* pData) noexcept override;

    virtual void free(void* pData, size_type numBytes) noexcept override;
};



/*-----------------------------------------------------------------------------
 * Thread-Safe Allocator
-----------------------------------------------------------------------------*/
class ThreadSafeAllocator : public Allocator
{
  public:
    static constexpr bool is_thread_safe() noexcept;

  public:
    virtual ~ThreadSafeAllocator() noexcept override;

    ThreadSafeAllocator() noexcept = delete;

    ThreadSafeAllocator(MemorySource& src) noexcept;

    ThreadSafeAllocator(const ThreadSafeAllocator&) = delete;

    ThreadSafeAllocator(ThreadSafeAllocator&& allocator) noexcept;

    ThreadSafeAllocator& operator=(const ThreadSafeAllocator& allocator) noexcept = delete;

    ThreadSafeAllocator& operator=(ThreadSafeAllocator&& allocator) noexcept;
};



/*-----------------------------------------------------------------------------
 * Malloc-based Allocator
-----------------------------------------------------------------------------*/
class MallocAllocator : public ThreadSafeAllocator
{
  public:
    virtual ~MallocAllocator() noexcept override;

    MallocAllocator() noexcept = delete;

    MallocAllocator(MallocMemorySource& src) noexcept;

    MallocAllocator(const Allocator&) = delete;

    MallocAllocator(MallocAllocator&& allocator) noexcept;

    MallocAllocator& operator=(const MallocAllocator& allocator) noexcept = delete;

    MallocAllocator& operator=(MallocAllocator&& allocator) noexcept;
};



/*-----------------------------------------------------------------------------
 * Atomic Allocator wrapper
-----------------------------------------------------------------------------*/
class AtomicAllocator : public ThreadSafeAllocator
{
  private:
    ls::utils::SpinLock mLock;

  public:
    virtual ~AtomicAllocator() noexcept override;

    AtomicAllocator() noexcept = delete;

    AtomicAllocator(MemorySource& src) noexcept;

    AtomicAllocator(const AtomicAllocator&) = delete;

    AtomicAllocator(AtomicAllocator&& allocator) noexcept;

    AtomicAllocator& operator=(const AtomicAllocator& allocator) noexcept = delete;

    AtomicAllocator& operator=(AtomicAllocator&& allocator) noexcept;

    virtual void* allocate(size_type numBytes) noexcept override;

    virtual void* allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept override;

    virtual void free(void* pData) noexcept override;

    virtual void free(void* pData, size_type numBytes) noexcept override;
};



/*-----------------------------------------------------------------------------
 * Threaded Memory Cache
-----------------------------------------------------------------------------*/
template <typename IAllocatorType = ls::utils::Allocator>
class ThreadedMemoryCache
{
    static_assert(ls::setup::IsBaseOf<ls::utils::IAllocator, IAllocatorType>::value, "Template allocator type does not implement the IAllocator interface.");

  public:
    typedef unsigned long long size_type;

    // There may be more than one allocator per-thread. Here we maintain a list
    // of them, and their associated per-thread caches.
    struct AllocatorList
    {
        ThreadSafeAllocator* mAllocator;
        AllocatorList* pNext;
        IAllocatorType mMemCache;
    };

  private:
    AllocatorList* mAllocators;

    AllocatorList* _insert_sub_allocator(AllocatorList* iter, ThreadSafeAllocator* allocator) noexcept;

  public:
    ~ThreadedMemoryCache() noexcept;

    ThreadedMemoryCache() noexcept;

    // When an allocator goes out of scope, we remove it from the allocator
    // list. There's absolutely no need to keep it's references around.
    void remove_allocator(ThreadSafeAllocator* allocator) noexcept;

    // Handle moving an allocator out of a temporary variable
    void replace_allocator(const ThreadSafeAllocator* pOld, ThreadSafeAllocator* pNew) noexcept;

    void* allocate(ThreadSafeAllocator* allocator, size_type n) noexcept;

    void free(ThreadSafeAllocator* allocator, void* p) noexcept;

    void free(ThreadSafeAllocator* allocator, void* p, size_type n) noexcept;
};

extern template class ThreadedMemoryCache<ls::utils::Allocator>;



/*-----------------------------------------------------------------------------
 * Threaded Allocator
-----------------------------------------------------------------------------*/
template <typename IAllocatorType = ls::utils::Allocator>
class ThreadedAllocator final : public ThreadSafeAllocator
{
    static_assert(ls::setup::IsBaseOf<ls::utils::IAllocator, IAllocatorType>::value, "Template allocator type does not implement the IAllocator interface.");

  private:
    static thread_local ThreadedMemoryCache<IAllocatorType> sThreadCache;

  public:
    virtual ~ThreadedAllocator() noexcept override;

    ThreadedAllocator() noexcept = delete;

    ThreadedAllocator(ThreadSafeAllocator& src) noexcept;

    ThreadedAllocator(const ThreadedAllocator&) = delete;

    ThreadedAllocator(ThreadedAllocator&& allocator) noexcept;

    ThreadedAllocator& operator=(const ThreadedAllocator&) = delete;

    ThreadedAllocator& operator=(ThreadedAllocator&& allocator) noexcept;

    virtual void* allocate(size_type numBytes) noexcept override;

    virtual void free(void* pData) noexcept override;

    virtual void free(void* pData, size_type numBytes) noexcept override;
};

extern template class ThreadedAllocator<ls::utils::Allocator>;



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/AllocatorImpl.hpp"

#endif /* LS_UTILS_ALLOCATOR_HPP */
