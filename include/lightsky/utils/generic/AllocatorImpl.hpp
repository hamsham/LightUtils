/*
 * File:   AllocatorImpl.hpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:03 PM
 */

#ifndef LS_UTILS_ALLOCATOR_IMPL_HPP
#define LS_UTILS_ALLOCATOR_IMPL_HPP

#include <new> // new overload
#include <utility> // std::move

#include "lightsky/utils/Assertions.h"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * IAllocator
-----------------------------------------------------------------------------*/
constexpr bool IAllocator::calloc_can_overflow(size_type numElements, size_type numBytesPerElement) noexcept
{
    return numBytesPerElement && numElements != ((numElements * numBytesPerElement) / numBytesPerElement);
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
inline void* IAllocator::allocate(size_type n) noexcept
{
    return this->memory_source().allocate(n);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
inline void IAllocator::free(void* p, size_type n) noexcept
{
    this->memory_source().free(p, n);
}



/*-------------------------------------
 * Free (unsized)
-------------------------------------*/
inline void IAllocator::free_unsized(void* p) noexcept
{
    this->memory_source().free(p);
}



/*-----------------------------------------------------------------------------
 * Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Internal Memory Source (const)
-------------------------------------*/
inline const MemorySource& Allocator::memory_source() const noexcept
{
    return *mMemSource;
}



/*-------------------------------------
 * Internal Memory Source
-------------------------------------*/
inline MemorySource& Allocator::memory_source() noexcept
{
    return *mMemSource;
}



/*-----------------------------------------------------------------------------
 * Constrained Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <unsigned long long maxNumBytes>
ConstrainedAllocator<maxNumBytes>::~ConstrainedAllocator() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long maxNumBytes>
ConstrainedAllocator<maxNumBytes>::ConstrainedAllocator(MemorySource& src) noexcept :
    Allocator{src},
    mBytesAllocated{0}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long maxNumBytes>
ConstrainedAllocator<maxNumBytes>::ConstrainedAllocator(ConstrainedAllocator&& allocator) noexcept :
    Allocator{std::move(allocator)},
    mBytesAllocated{allocator.mBytesAllocated}
{
    allocator.mBytesAllocated = 0;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <unsigned long long maxNumBytes>
ConstrainedAllocator<maxNumBytes>& ConstrainedAllocator<maxNumBytes>::operator=(ConstrainedAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        Allocator::operator=(std::move(allocator));

        mBytesAllocated = allocator.mBytesAllocated;
        allocator.mBytesAllocated = 0;
    }

    return *this;
}



/*-------------------------------------
 * Allocate
-------------------------------------*/
template <unsigned long long maxNumBytes>
void* ConstrainedAllocator<maxNumBytes>::allocate() noexcept
{
    LS_ASSERT(false);
    return nullptr;
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
template <unsigned long long maxNumBytes>
inline void* ConstrainedAllocator<maxNumBytes>::allocate(size_type numBytes) noexcept
{
    const size_type newByteCount = mBytesAllocated+numBytes;

    if (newByteCount > maxNumBytes)
    {
        return nullptr;
    }

    void* pData = this->memory_source().allocate(numBytes);
    if (pData)
    {
        mBytesAllocated = newByteCount;
    }

    return  pData;
}



/*-------------------------------------
 * Free
-------------------------------------*/
template <unsigned long long maxNumBytes>
void ConstrainedAllocator<maxNumBytes>::free(void* pData) noexcept
{
    (void)pData;
    LS_ASSERT(false);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
template <unsigned long long maxNumBytes>
inline void ConstrainedAllocator<maxNumBytes>::free(void* pData, size_type numBytes) noexcept
{
    if (pData && numBytes)
    {
        LS_ASSERT(mBytesAllocated >= numBytes);
        mBytesAllocated -= numBytes;
        this->memory_source().free(pData, numBytes);
    }
}



/*-----------------------------------------------------------------------------
 * Constrained Allocator (specialized for run-time constraints)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
inline void* ConstrainedAllocator<0>::allocate(size_type numBytes) noexcept
{
    const size_type newByteCount = mBytesAllocated+numBytes;

    if (newByteCount > mMaxAllocSize)
    {
        return nullptr;
    }

    void* pData = this->memory_source().allocate(numBytes);
    if (pData)
    {
        mBytesAllocated = newByteCount;
    }

    return  pData;
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
inline void ConstrainedAllocator<0>::free(void* pData, size_type numBytes) noexcept
{
    if (pData && numBytes)
    {
        LS_ASSERT(mBytesAllocated >= numBytes);
        mBytesAllocated -= numBytes;
        this->memory_source().free(pData, numBytes);
    }
}



/*-----------------------------------------------------------------------------
 * Thread-safe Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Thread-Safety Check
-------------------------------------*/
constexpr bool ThreadSafeAllocator::is_thread_safe() noexcept
{
    return true;
}



/*-----------------------------------------------------------------------------
 * Atomic Allocator wrapper
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Allocate
-------------------------------------*/
inline void* AtomicAllocator::allocate() noexcept
{
    mLock.lock();
    void* const pMem = this->memory_source().allocate();
    mLock.unlock();

    return pMem;
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
inline void* AtomicAllocator::allocate(size_type n) noexcept
{
    mLock.lock();
    void* const pMem = this->memory_source().allocate(n);
    mLock.unlock();

    return pMem;
}



/*-------------------------------------
 * Free
-------------------------------------*/
inline void AtomicAllocator::free(void* p) noexcept
{
    mLock.lock();
    this->memory_source().free(p);
    mLock.unlock();
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
inline void AtomicAllocator::free(void* p, size_type n) noexcept
{
    mLock.lock();
    this->memory_source().free(p, n);
    mLock.unlock();
}



/*-----------------------------------------------------------------------------
 * Threaded Memory Cache
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename IAllocatorType>
ThreadedMemoryCache<IAllocatorType>::~ThreadedMemoryCache() noexcept
{
    // Free all of the allocations using the per-allocator member
    // functions. Each allocator is responsible for its own cache's memory
    // AND the AllocatorList node associated with it.
    AllocatorList* pIter = mAllocators;
    AllocatorList* pNext = mAllocators ? mAllocators->pNext : nullptr;

    while (pIter != nullptr)
    {
        // Each allocator must free its own cache entry
        ThreadSafeAllocator* pAllocator = pIter->mAllocator;
        pIter->~AllocatorList();
        pAllocator->free(pIter, sizeof(AllocatorList));
        pIter = pNext;

        if (pNext)
        {
            pNext = pNext->pNext;
        }
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename IAllocatorType>
ThreadedMemoryCache<IAllocatorType>::ThreadedMemoryCache() noexcept :
    mAllocators{nullptr}
{}



/*-------------------------------------
 * Insert TLS Cache
-------------------------------------*/
template <typename IAllocatorType>
typename ThreadedMemoryCache<IAllocatorType>::AllocatorList*
ThreadedMemoryCache<IAllocatorType>::_insert_sub_allocator(
    typename ThreadedMemoryCache<IAllocatorType>::AllocatorList* iter, ThreadSafeAllocator* allocator) noexcept
{
    void* const pCacheLocation = allocator->allocate(sizeof(AllocatorList));
    if (!pCacheLocation)
    {
        return nullptr;
    }

    // The primary allocator will also need to allocate its own list node
    // for bookkeeping... nobody else will.
    MemorySource* const pMemSrc = static_cast<MemorySource*>(allocator);

    AllocatorList* const pListEntry = new (pCacheLocation) AllocatorList{
        allocator,
        nullptr,
        IAllocatorType{*pMemSrc}
    };

    if (iter)
    {
        iter->pNext = pListEntry;
    }
    else
    {
        mAllocators = pListEntry;
    }

    return pListEntry;
}



/*-------------------------------------
 * Remove TLS Cache
-------------------------------------*/
template <typename IAllocatorType>
void ThreadedMemoryCache<IAllocatorType>::remove_allocator(ThreadSafeAllocator* allocator) noexcept
{
    AllocatorList* pPrev = nullptr;
    AllocatorList* pIter = mAllocators;

    while (true)
    {
        ls::utils::runtime_assert(pIter != nullptr, ls::utils::ErrorLevel::LS_ERROR, "Attempted to free a nonexistent allocator cache.");

        if (pIter->mAllocator == allocator)
        {
            AllocatorList* pNext = pIter->pNext;
            if (pPrev)
            {
                pPrev->pNext = pNext;
            }

            if (mAllocators == pIter)
            {
                mAllocators = pNext;
            }

            break;
        }

        pPrev = pIter;
        pIter = pIter->pNext;
    }

    ThreadSafeAllocator* pAllocator = pIter->mAllocator;
    pIter->~AllocatorList();
    pAllocator->free(pIter, sizeof(AllocatorList));
}



/*-------------------------------------
 * Replace TLS Cache
-------------------------------------*/
template <typename IAllocatorType>
void ThreadedMemoryCache<IAllocatorType>::replace_allocator(const ThreadSafeAllocator* pOld, ThreadSafeAllocator* pNew) noexcept
{
    AllocatorList* pIter = mAllocators;

    while (pIter != nullptr)
    {
        if (pIter->mAllocator == pOld)
        {
            pIter->mAllocator = pNew;
            break;
        }

        pIter = pIter->pNext;
    }
}



/*-------------------------------------
 * Allocate
-------------------------------------*/
template <typename IAllocatorType>
inline void* ThreadedMemoryCache<IAllocatorType>::allocate(ThreadSafeAllocator* allocator) noexcept
{
    AllocatorList* iter = mAllocators;

    do
    {
        if (LS_UNLIKELY(!iter))
        {
            iter = _insert_sub_allocator(nullptr, allocator);
            if (LS_UNLIKELY(!iter))
            {
                return nullptr;
            }

            break;
        }

        if (LS_LIKELY(iter->mAllocator == allocator))
        {
            break;
        }

        iter = iter->pNext;
    }
    while (true);

    return iter->mMemCache.allocate();
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
template <typename IAllocatorType>
inline void* ThreadedMemoryCache<IAllocatorType>::allocate(ThreadSafeAllocator* allocator, size_type n) noexcept
{
    AllocatorList* iter = mAllocators;

    do
    {
        if (LS_UNLIKELY(!iter))
        {
            iter = _insert_sub_allocator(nullptr, allocator);
            if (LS_UNLIKELY(!iter))
            {
                return nullptr;
            }

            break;
        }

        if (LS_LIKELY(iter->mAllocator == allocator))
        {
            break;
        }

        iter = iter->pNext;
    }
    while (true);

    return iter->mMemCache.allocate(n);
}



/*-------------------------------------
 * Free
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedMemoryCache<IAllocatorType>::free(ThreadSafeAllocator* allocator, void* p) noexcept
{
    // pIter must never be NULL in this function, otherwise our memory-source
    // has mysteriously gone out of scope before its static member
    AllocatorList* pIter = mAllocators;

    do
    {
        if (LS_LIKELY(pIter->mAllocator == allocator))
        {
            break;
        }

        pIter = pIter->pNext;
    }
    while (true);

    pIter->mMemCache.free(p);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedMemoryCache<IAllocatorType>::free(ThreadSafeAllocator* allocator, void* p, size_type n) noexcept
{
    // pIter must never be NULL in this function, otherwise our memory-source
    // has mysteriously gone out of scope before its static member
    AllocatorList* pIter = mAllocators;

    do
    {
        if (LS_LIKELY(pIter->mAllocator == allocator))
        {
            break;
        }

        pIter = pIter->pNext;
    }
    while (true);

    pIter->mMemCache.free(p, n);
}



/*-------------------------------------
 * Free (unsized)
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedMemoryCache<IAllocatorType>::free_unsized(ThreadSafeAllocator* allocator, void* p) noexcept
{
    // pIter must never be NULL in this function, otherwise our memory-source
    // has mysteriously gone out of scope before its static member
    AllocatorList* pIter = mAllocators;

    do
    {
        if (LS_LIKELY(pIter->mAllocator == allocator))
        {
            break;
        }

        pIter = pIter->pNext;
    }
    while (true);

    pIter->mMemCache.free_unsized(p);
}



/*-----------------------------------------------------------------------------
 * Threaded Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * TLS Manager
-------------------------------------*/
template <typename IAllocatorType>
thread_local ThreadedMemoryCache<IAllocatorType> ThreadedAllocator<IAllocatorType>::sThreadCache;



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>::~ThreadedAllocator() noexcept
{
    //ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    //sThreadCache.remove_allocator(&memSrc);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>::ThreadedAllocator(ThreadSafeAllocator& src) noexcept :
    ThreadSafeAllocator{static_cast<MemorySource&>(src)}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>::ThreadedAllocator(ThreadedAllocator&& allocator) noexcept :
    ThreadSafeAllocator{std::move(allocator)}
{
    sThreadCache.replace_allocator(&allocator, this);
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>& ThreadedAllocator<IAllocatorType>::operator=(ThreadedAllocator&& allocator) noexcept
{
    if (this != &allocator)
    {
        Allocator::operator=(std::move(allocator));
        sThreadCache.replace_allocator(&allocator, this);
    }

    return *this;
}



/*-------------------------------------
 * Allocate
-------------------------------------*/
template <typename IAllocatorType>
inline void* ThreadedAllocator<IAllocatorType>::allocate() noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    return sThreadCache.allocate(&memSrc);
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
template <typename IAllocatorType>
inline void* ThreadedAllocator<IAllocatorType>::allocate(size_type n) noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    return sThreadCache.allocate(&memSrc, n);
}



/*-------------------------------------
 * Free
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedAllocator<IAllocatorType>::free(void* p) noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    sThreadCache.free(&memSrc, p);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedAllocator<IAllocatorType>::free(void* p, size_type n) noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    sThreadCache.free(&memSrc, p, n);
}



/*-------------------------------------
 * Free (unsized)
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedAllocator<IAllocatorType>::free_unsized(void* p) noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    sThreadCache.free_unsized(&memSrc, p);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_ALLOCATOR_IMPL_HPP */
