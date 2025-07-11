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



/*-----------------------------------------------------------------------------
 * Spinnable R/W Semaphore
-----------------------------------------------------------------------------*/
#ifndef LS_UTILS_SWRLOCK_CPU_YIELD
    #define LS_UTILS_SWRLOCK_CPU_YIELD 0
#endif

/*-------------------------------------
 * Yield
-------------------------------------*/
inline void LS_IMPERATIVE RWLock::yield() noexcept
{
    #if LS_UTILS_SWRLOCK_CPU_YIELD
        ls::setup::cpu_yield();
    #else
        std::this_thread::yield();
    #endif
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
inline RWLock::RWLock() noexcept :
    mLockType{0},
    mShareCount{0}
{}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
inline void RWLock::lock_shared() noexcept
{
    while (!try_lock_shared())
    {
        RWLock::yield();
    }
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
inline void RWLock::lock() noexcept
{
    while (!try_lock())
    {
        RWLock::yield();
    }
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool RWLock::try_lock_shared() noexcept
{
    mShareCount.fetch_add(1);

    const bool haveLock = mLockType.load(std::memory_order_acquire) == 0;
    if (!haveLock)
    {
        mShareCount.fetch_sub(1);
    }

    return haveLock;
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool RWLock::try_lock() noexcept
{
    uint32_t writeBit = 0;
    if (!mLockType.compare_exchange_strong(writeBit, LockFlags::LOCK_TRY_WRITE_BIT))
    {
        return false;
    }

    bool haveLock = mShareCount.load(std::memory_order_acquire) == 0;
    if (!haveLock)
    {
        mLockType.store(0, std::memory_order_release);
    }

    return haveLock;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void RWLock::unlock_shared() noexcept
{
    mShareCount.fetch_sub(1);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void RWLock::unlock() noexcept
{
    mLockType.store(0);
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const RWLock::native_handle_type& RWLock::native_handle() const noexcept
{
    return *this;
}



/*-----------------------------------------------------------------------------
 * PThreads R/W Semaphore
-----------------------------------------------------------------------------*/
#if defined(LS_OS_UNIX) || defined(LS_OS_LINUX)

/*-------------------------------------
 * Destructor
-------------------------------------*/
inline SystemRWLock::~SystemRWLock() noexcept
{
    pthread_rwlock_destroy(&mLock);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
inline SystemRWLock::SystemRWLock() noexcept
{
    pthread_rwlock_init(&mLock, nullptr);
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
inline void SystemRWLock::lock_shared() noexcept
{
    constexpr unsigned maxPauses = 32;
    unsigned currentPauses = 1;

    do
    {
        if (try_lock_shared())
        {
            return;
        }

        for (unsigned i = 0; i < currentPauses; ++i)
        {
            std::this_thread::yield();
        }

        currentPauses <<= 1;
    }
    while (currentPauses < maxPauses);

    while (pthread_rwlock_rdlock(&mLock) != 0)
    {
        std::this_thread::yield();
    }
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
inline void SystemRWLock::lock() noexcept
{
    constexpr unsigned maxPauses = 32;
    unsigned currentPauses = 1;

    do
    {
        if (try_lock())
        {
            return;
        }

        for (unsigned i = 0; i < currentPauses; ++i)
        {
            std::this_thread::yield();
        }

        currentPauses <<= 1;
    }
    while (currentPauses < maxPauses);

    while (pthread_rwlock_wrlock(&mLock) != 0)
    {
        std::this_thread::yield();
    }
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool SystemRWLock::try_lock_shared() noexcept
{
    return pthread_rwlock_tryrdlock(&mLock) == 0;
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool SystemRWLock::try_lock() noexcept
{
    return pthread_rwlock_trywrlock(&mLock) == 0;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void SystemRWLock::unlock_shared() noexcept
{
    pthread_rwlock_unlock(&mLock);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void SystemRWLock::unlock() noexcept
{
    pthread_rwlock_unlock(&mLock);
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const SystemRWLock::native_handle_type& SystemRWLock::native_handle() const noexcept
{
    return mLock;
}



/*-----------------------------------------------------------------------------
 * Windows R/W Semaphore
-----------------------------------------------------------------------------*/
#elif defined(LS_OS_WINDOWS)

/*-------------------------------------
 * Destructor
-------------------------------------*/
inline SystemRWLock::~SystemRWLock() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
inline SystemRWLock::SystemRWLock() noexcept
{
    InitializeSRWLock(&mLock);
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
inline void SystemRWLock::lock_shared() noexcept
{
    constexpr unsigned maxPauses = 32;
    unsigned currentPauses = 1;

    do
    {
        if (try_lock_shared())
        {
            return;
        }

        for (unsigned i = 0; i < currentPauses; ++i)
        {
            std::this_thread::yield();
        }

        currentPauses <<= 1;
    }
    while (currentPauses <= maxPauses);

    AcquireSRWLockShared(&mLock);
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
inline void SystemRWLock::lock() noexcept
{
    do
    {
        if (try_lock())
        {
            return;
        }

        for (unsigned i = 0; i < currentPauses; ++i)
        {
            std::this_thread::yield();
        }

        currentPauses <<= 1;
    }
    while (currentPauses <= maxPauses);

    AcquireSRWLockExclusive(&mLock);
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool SystemRWLock::try_lock_shared() noexcept
{
    return TryAcquireSRWLockShared(&mLock) != 0;
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool SystemRWLock::try_lock() noexcept
{
    return TryAcquireSRWLockExclusive(&mLock) != 0;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void SystemRWLock::unlock_shared() noexcept
{
    ReleaseSRWLockShared(&mLock);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void SystemRWLock::unlock() noexcept
{
    ReleaseSRWLockExclusive(&mLock);
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const SystemRWLock::native_handle_type& SystemRWLock::native_handle() const noexcept
{
    return mLock;
}



#endif /* LS_OS_WINDOWS */



/*-----------------------------------------------------------------------------
 * Sharable R/W Lock With Fair Ordering
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Constructor
-------------------------------------*/
inline FairRWLock::FairRWLock() noexcept :
    mLockBits{0}
{}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
inline void FairRWLock::lock_shared() noexcept
{
    const uint16_t lockId = mLockFields.currentLockId.fetch_add(1);
    while (lockId != mLockFields.nextLockId.load(std::memory_order_acquire))
    {
        std::this_thread::yield();
    }

    mLockFields.shareCount.fetch_add(1);

    while (mLockFields.lockType.load(std::memory_order_acquire))
    {
        std::this_thread::yield();
    }

    mLockFields.nextLockId.fetch_add(1);
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
inline void FairRWLock::lock() noexcept
{
    const uint16_t lockId = mLockFields.currentLockId.fetch_add(1);
    while (lockId != mLockFields.nextLockId.load(std::memory_order_acquire))
    {
        std::this_thread::yield();
    }

    uint8_t writeBit;
    do
    {
        writeBit = 0;
    }
    while (!mLockFields.lockType.compare_exchange_strong(writeBit, LockFlags::LOCK_WRITE_BIT));

    while (mLockFields.shareCount.load(std::memory_order_acquire) != 0)
    {
        std::this_thread::yield();
    }
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool FairRWLock::try_lock_shared() noexcept
{
    mLockFields.shareCount.fetch_add(1);

    const bool haveLock = mLockFields.lockType.load(std::memory_order_acquire) == 0;
    if (!haveLock)
    {
        mLockFields.shareCount.fetch_sub(1);
    }

    return haveLock;
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool FairRWLock::try_lock() noexcept
{
    uint8_t writeBit = 0;
    if (!mLockFields.lockType.compare_exchange_strong(writeBit, LockFlags::LOCK_TRY_WRITE_BIT))
    {
        return false;
    }

    bool haveLock = mLockFields.shareCount.load(std::memory_order_acquire) == 0;
    if (!haveLock)
    {
        mLockFields.lockType.store(0, std::memory_order_release);
    }

    return haveLock;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void FairRWLock::unlock_shared() noexcept
{
    mLockFields.shareCount.fetch_sub(1);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void FairRWLock::unlock() noexcept
{
    const uint16_t lockType = mLockFields.lockType.exchange(0);
    if (lockType == LockFlags::LOCK_WRITE_BIT)
    {
        mLockFields.nextLockId.fetch_add(1);
    }
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const FairRWLock::native_handle_type& FairRWLock::native_handle() const noexcept
{
    return mLockBits;
}



/*-----------------------------------------------------------------------------
 * Shared Lock Guard
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename RWLockType>
inline LockGuardShared<RWLockType>::~LockGuardShared() noexcept
{
    mRWLock.unlock_shared();
}



/*-------------------------------------
 * RAII Constructor
-------------------------------------*/
template <typename RWLockType>
inline LockGuardShared<RWLockType>::LockGuardShared(RWLockType& rwLock) noexcept :
    mRWLock{rwLock}
{
    mRWLock.lock_shared();
}



/*-----------------------------------------------------------------------------
 * Exclusive Lock Guard
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename RWLockType>
inline LockGuardExclusive<RWLockType>::~LockGuardExclusive() noexcept
{
    mRWLock.unlock();
}



/*-------------------------------------
 * RAII Constructor
-------------------------------------*/
template <typename RWLockType>
inline LockGuardExclusive<RWLockType>::LockGuardExclusive(RWLockType& rwLock) noexcept :
    mRWLock{rwLock}
{
    mRWLock.lock();
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FAIR_RW_LOCK_IMPL_HPP */
