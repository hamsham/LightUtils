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
inline void* IAllocator::allocate(size_type n) noexcept
{
    return this->memory_source().allocate(n);
}

inline void IAllocator::free(void* p, size_type n) noexcept
{
    this->memory_source().free(p, n);
}



/*-----------------------------------------------------------------------------
 * Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
inline const MemorySource& Allocator::memory_source() const noexcept
{
    return *mMemSource;
}



/*-------------------------------------
-------------------------------------*/
inline MemorySource& Allocator::memory_source() noexcept
{
    return *mMemSource;
}



/*-----------------------------------------------------------------------------
 * Thread-safe Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
constexpr bool ThreadSafeAllocator::is_thread_safe() noexcept
{
    return true;
}



/*-----------------------------------------------------------------------------
 * Atomic Allocator wrapper
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
inline void* AtomicAllocator::allocate(size_type n) noexcept
{
    mLock.lock();
    void* const pMem = Allocator::allocate(n);
    mLock.unlock();

    return pMem;
}



/*-------------------------------------
-------------------------------------*/
inline void AtomicAllocator::free(void* p, size_type n) noexcept
{
    mLock.lock();
    Allocator::free(p, n);
    mLock.unlock();
}



/*-----------------------------------------------------------------------------
 * Threaded Memory Cache
-----------------------------------------------------------------------------*/
/*-------------------------------------
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
-------------------------------------*/
template <typename IAllocatorType>
ThreadedMemoryCache<IAllocatorType>::ThreadedMemoryCache() noexcept :
    mAllocators{nullptr}
{}



/*-------------------------------------
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
-------------------------------------*/
template <typename IAllocatorType>
void ThreadedMemoryCache<IAllocatorType>::remove_allocator(ThreadSafeAllocator* allocator) noexcept
{
    AllocatorList* pPrev = nullptr;
    AllocatorList* pIter = mAllocators;

    while (true)
    {
        if (pIter == nullptr)
        {
            break;
        }

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

            ThreadSafeAllocator* pAllocator = pIter->mAllocator;
            pIter->~AllocatorList();
            pAllocator->free(pIter, sizeof(AllocatorList));
            break;
        }

        pPrev = pIter;
        pIter = pIter->pNext;
    }
}



/*-------------------------------------
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
-------------------------------------*/
template <typename IAllocatorType>
inline void* ThreadedMemoryCache<IAllocatorType>::allocate(ThreadSafeAllocator* allocator, size_type n) noexcept
{
    static_assert(ls::setup::IsBaseOf<IAllocator, IAllocatorType>::value, "Template allocator type does not implement the IAllocator interface.");
    LS_DEBUG_ASSERT(allocator != nullptr);

    AllocatorList* iter = mAllocators;

    while (true)
    {
        if (!iter)
        {
            iter = _insert_sub_allocator(nullptr, allocator);
            if (!iter)
            {
                return nullptr;
            }
        }

        if (iter->mAllocator != allocator)
        {
            iter = iter->pNext;
            continue;
        }

        break;
    }

    return iter->mMemCache.allocate(n);
}



/*-------------------------------------
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedMemoryCache<IAllocatorType>::free(ThreadSafeAllocator* allocator, void* p, size_type n) noexcept
{
    static_assert(ls::setup::IsBaseOf<IAllocator, IAllocatorType>::value, "Template allocator type does not implement the IAllocator interface.");
    LS_DEBUG_ASSERT(allocator != nullptr);

    // pIter must never be NULL in this function, otherwise our memory-source
    // has mysteriously gone out of scope before its static member
    AllocatorList* pIter = mAllocators;

    while (pIter->mAllocator != allocator)
    {
        pIter = pIter->pNext;
    }

    pIter->mMemCache.free(p, n);
}



/*-----------------------------------------------------------------------------
 * Threaded Allocator
-----------------------------------------------------------------------------*/
template <typename IAllocatorType>
thread_local ThreadedMemoryCache<IAllocatorType> ThreadedAllocator<IAllocatorType>::sThreadCache;



/*-------------------------------------
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>::~ThreadedAllocator() noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    sThreadCache.remove_allocator(&memSrc);
}



/*-------------------------------------
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>::ThreadedAllocator(ThreadSafeAllocator& src) noexcept :
    ThreadSafeAllocator{static_cast<MemorySource&>(src)}
{
}



/*-------------------------------------
-------------------------------------*/
template <typename IAllocatorType>
ThreadedAllocator<IAllocatorType>::ThreadedAllocator(ThreadedAllocator&& allocator) noexcept :
    ThreadSafeAllocator{std::move(allocator)}
{
    sThreadCache.replace_allocator(&allocator, this);
}



/*-------------------------------------
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
-------------------------------------*/
template <typename IAllocatorType>
inline void* ThreadedAllocator<IAllocatorType>::allocate(size_type n) noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    return sThreadCache.allocate(&memSrc, n);
}



/*-------------------------------------
-------------------------------------*/
template <typename IAllocatorType>
inline void ThreadedAllocator<IAllocatorType>::free(void* p, size_type n) noexcept
{
    ThreadSafeAllocator& memSrc = static_cast<ThreadSafeAllocator&>(this->memory_source());
    sThreadCache.free(&memSrc, p, n);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_ALLOCATOR_IMPL_HPP */
