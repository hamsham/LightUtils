/*
 * File:   FairRWLockImpl.hpp
 * Author: hammy
 *
 * Created on Mar 30, 2023 at 9:42 PM
 */

#ifndef LS_UTILS_FAIR_RW_LOCK_IMPL_HPP
#define LS_UTILS_FAIR_RW_LOCK_IMPL_HPP

#include <thread>

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
#if LS_UTILS_USE_PTHREAD_RWLOCK

/*-------------------------------------
 * Destructor
-------------------------------------*/
inline SystemRWLockPThread::~SystemRWLockPThread() noexcept
{
    pthread_rwlock_destroy(&mLock);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
inline SystemRWLockPThread::SystemRWLockPThread() noexcept
{
    pthread_rwlock_init(&mLock, nullptr);
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool SystemRWLockPThread::try_lock_shared() noexcept
{
    return pthread_rwlock_tryrdlock(&mLock) == 0;
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool SystemRWLockPThread::try_lock() noexcept
{
    return pthread_rwlock_trywrlock(&mLock) == 0;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void SystemRWLockPThread::unlock_shared() noexcept
{
    pthread_rwlock_unlock(&mLock);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void SystemRWLockPThread::unlock() noexcept
{
    pthread_rwlock_unlock(&mLock);
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const SystemRWLockPThread::native_handle_type& SystemRWLockPThread::native_handle() const noexcept
{
    return mLock;
}

#endif



/*-----------------------------------------------------------------------------
 * Windows R/W Semaphore
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_RWLOCK

/*-------------------------------------
 * Destructor
-------------------------------------*/
inline SystemRWLockWindows::~SystemRWLockWindows() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
inline SystemRWLockWindows::SystemRWLockWindows() noexcept
{
    InitializeSRWLock(&mLock);
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool SystemRWLockWindows::try_lock_shared() noexcept
{
    return TryAcquireSRWLockShared(&mLock) != 0;
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool SystemRWLockWindows::try_lock() noexcept
{
    return TryAcquireSRWLockExclusive(&mLock) != 0;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void SystemRWLockWindows::unlock_shared() noexcept
{
    ReleaseSRWLockShared(&mLock);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void SystemRWLockWindows::unlock() noexcept
{
    ReleaseSRWLockExclusive(&mLock);
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const SystemRWLockWindows::native_handle_type& SystemRWLockWindows::native_handle() const noexcept
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
