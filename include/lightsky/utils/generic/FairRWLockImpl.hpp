/*
 * File:   FairRWLockImpl.hpp
 * Author: hammy
 *
 * Created on Mar 30, 2023 at 9:42 PM
 */

#ifndef LS_UTILS_FAIR_RW_LOCK_IMPL_HPP
#define LS_UTILS_FAIR_RW_LOCK_IMPL_HPP

#include "lightsky/utils/Assertions.h"

namespace ls
{
namespace utils
{



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename MutexType>
FairRWLockType<MutexType>::FairRWLockType() noexcept :
    mHead{{}, {nullptr}, {nullptr}, {nullptr}},
    mTail{{}, {nullptr}, {nullptr}, {&mHead}},
    mNumUsers{0}
{
    mHead.mNextLock = &mTail.mLock;
    mHead.pNext.store(&mTail);
}



/*-------------------------------------
 * Enqueue Lock
-------------------------------------*/
template <typename MutexType>
void FairRWLockType<MutexType>::_insert_node(RWLockNode& lock) noexcept
{
    RWLockNode* const pTail = &mTail;
    RWLockNode* pPrev;

    lock.mNextLock.store(&pTail->mLock, std::memory_order_relaxed);
    lock.pNext.store(pTail, std::memory_order_release);

    pTail->mLock.lock();
    lock.mLock.lock();

    pPrev = pTail->pPrev.load(std::memory_order_acquire);
    lock.pPrev.store(pPrev, std::memory_order_release);
    pTail->pPrev.store(&lock, std::memory_order_release);

    pPrev->pNext.store(&lock, std::memory_order_release);
    pPrev->mNextLock.store(&lock.mLock, std::memory_order_relaxed);

    lock.mLock.unlock();
    pTail->mLock.unlock();
}



/*-------------------------------------
 * Enqueue Lock
-------------------------------------*/
template <typename MutexType>
bool FairRWLockType<MutexType>::_try_insert_node(RWLockNode& lock) noexcept
{
    bool haveLock = false;
    RWLockNode* const pTail = &mTail;
    RWLockNode* pPrev;

    if (mNumUsers.load(std::memory_order_relaxed) == 0)
    {
        if (pTail->mLock.try_lock())
        {
            lock.mLock.lock();
            lock.mNextLock.store(&pTail->mLock, std::memory_order_relaxed);
            lock.pNext.store(pTail, std::memory_order_release);

            pPrev = pTail->pPrev.load(std::memory_order_acquire);
            lock.pPrev.store(pPrev, std::memory_order_release);
            pTail->pPrev.store(&lock, std::memory_order_release);

            pPrev->pNext.store(&lock, std::memory_order_release);
            pPrev->mNextLock.store(&lock.mLock, std::memory_order_relaxed);

            lock.mLock.unlock();
            pTail->mLock.unlock();

            haveLock = true;
        }
    }

    return haveLock;
}



/*-------------------------------------
 * Pop From Lock Queue
-------------------------------------*/
template <typename MutexType>
void FairRWLockType<MutexType>::_pop_node_impl(RWLockNode& lock, bool lockedNextNode) noexcept
{
    RWLockNode* pNext;
    native_handle_type* mtx = lock.mNextLock.load(std::memory_order_relaxed);

    if (!lockedNextNode)
    {
        mtx->lock();
    }

    pNext = lock.pNext.load(std::memory_order_acquire);
    pNext->pPrev.store(&mHead, std::memory_order_release);

    mHead.mNextLock.store(mtx, std::memory_order_relaxed);
    mHead.pNext.store(pNext, std::memory_order_release);

    mtx->unlock();
}



/*-------------------------------------
 * Wait in the Queue
-------------------------------------*/
template <typename MutexType>
bool FairRWLockType<MutexType>::_wait_impl(RWLockNode& lock) noexcept
{
    bool lockedNext = false;
    const RWLockNode* const pHead = &mHead;
    const RWLockNode* const pTail = &mTail;
    native_handle_type& mtx = lock.mLock;

    RWLockNode* const pNext = lock.pNext.load(std::memory_order_acquire);
    if (LS_LIKELY(pNext != pTail))
    {
        lockedNext = pNext->mLock.try_lock();
    }

    bool amFree = lock.pPrev.load(std::memory_order_relaxed) == pHead;
    unsigned spinCount = 1;
    constexpr unsigned maxLoops = (unsigned)ls::utils::FutexPauseCount::FUTEX_PAUSE_COUNT_32 - 1u;

    while (!amFree && (spinCount & maxLoops))
    {
        unsigned i = spinCount;
        while (i--)
        {
            ls::setup::cpu_yield();
            ls::setup::cpu_yield();
        }

        spinCount <<= 1u;
        amFree = lock.pPrev.load(std::memory_order_relaxed) == pHead;
    }

    while (!amFree)
    {
        mtx.lock();
        mtx.unlock();

        amFree = lock.pPrev.load(std::memory_order_relaxed) == pHead;
    }

    return lockedNext;
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
template <typename MutexType>
void FairRWLockType<MutexType>::lock_shared() noexcept
{
    RWLockNode lock{{}, {nullptr}, {nullptr}, {nullptr}};
    _insert_node(lock);

    bool lockedNext = _wait_impl(lock);
    while (mNumUsers.load(std::memory_order_relaxed) < 0)
    {
    }

    mNumUsers.fetch_add(1, std::memory_order_acq_rel);
    _pop_node_impl(lock, lockedNext);
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
template <typename MutexType>
void FairRWLockType<MutexType>::lock() noexcept
{
    RWLockNode lock{{}, {nullptr}, {nullptr}, {nullptr}};
    _insert_node(lock);

    bool lockedNext = _wait_impl(lock);
    while (mNumUsers.load(std::memory_order_relaxed) != 0)
    {
    }

    mNumUsers.store(-1, std::memory_order_release);
    _pop_node_impl(lock, lockedNext);
}



/*-------------------------------------
 * Non-Exclusive Lock Attempt
-------------------------------------*/
template <typename MutexType>
bool FairRWLockType<MutexType>::try_lock_shared() noexcept
{
    RWLockNode lock{{}, {nullptr}, {nullptr}, {nullptr}};
    bool acquiredLock = _try_insert_node(lock);

    if (acquiredLock)
    {
        bool lockedNext = _wait_impl(lock);
        while (mNumUsers.load(std::memory_order_relaxed) < 0)
        {
        }

        mNumUsers.fetch_add(1, std::memory_order_release);
        _pop_node_impl(lock, lockedNext);
    }

    return acquiredLock;
}



/*-------------------------------------
 * Exclusive Lock Attempt
-------------------------------------*/
template <typename MutexType>
bool FairRWLockType<MutexType>::try_lock() noexcept
{
    RWLockNode lock{{}, {nullptr}, {nullptr}, {nullptr}};
    bool acquiredLock = _try_insert_node(lock);

    if (acquiredLock)
    {
        bool lockedNext = _wait_impl(lock);
        while (mNumUsers.load(std::memory_order_relaxed) != 0)
        {
        }

        mNumUsers.store(-1, std::memory_order_release);
        _pop_node_impl(lock, lockedNext);
    }

    return acquiredLock;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
template <typename MutexType>
inline void FairRWLockType<MutexType>::unlock_shared() noexcept
{
    long long amShared = mNumUsers.fetch_sub(1, std::memory_order_acq_rel);
    (void)amShared;
    LS_DEBUG_ASSERT(amShared > 0);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
template <typename MutexType>
inline void FairRWLockType<MutexType>::unlock() noexcept
{
    LS_DEBUG_ASSERT(-1 == mNumUsers.load(std::memory_order_acquire));
    mNumUsers.store(0, std::memory_order_release);
}



/*-------------------------------------
 * Retrieve the internal lock
-------------------------------------*/
template <typename MutexType>
inline const typename FairRWLockType<MutexType>::native_handle_type& FairRWLockType<MutexType>::native_handle() const noexcept
{
    return mHead.mLock;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FAIR_RW_LOCK_IMPL_HPP */
