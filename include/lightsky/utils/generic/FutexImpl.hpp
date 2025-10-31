
#ifndef LS_UTILS_FUTEX_IMPL_HPP
#define LS_UTILS_FUTEX_IMPL_HPP

#include <limits>

#include "lightsky/setup/CPU.h"



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Futex
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Set the maximum consecutive pauses before retrying a lock
-------------------------------------*/
inline void Futex::pause_count(FutexPauseCount maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline FutexPauseCount Futex::pause_count() const noexcept
{
    return mMaxPauseCount;
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool Futex::try_lock() noexcept
{
    uint32_t tmp = 0;
    return mLock.compare_exchange_strong(tmp, 1, std::memory_order_seq_cst, std::memory_order_relaxed);
}



/*-------------------------------------
 * Futex unlock
-------------------------------------*/
inline void Futex::unlock() noexcept
{
    mLock.store(0, std::memory_order_release);
}



/*-----------------------------------------------------------------------------
 * SystemFutexLinux
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_LINUX_FUTEX

/*-------------------------------------
 * Set the maximum consecutive pauses before retrying a lock
-------------------------------------*/
inline void SystemFutexLinux::pause_count(FutexPauseCount maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline FutexPauseCount SystemFutexLinux::pause_count() const noexcept
{
    return mMaxPauseCount;
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool SystemFutexLinux::try_lock() noexcept
{
    uint32_t tmp = 0u;
    return __atomic_compare_exchange_n(&mLock, &tmp, 1u, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}

#endif // LS_UTILS_USE_LINUX_FUTEX



/*-----------------------------------------------------------------------------
 * SystemFutexPThread
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_FUTEX

/*-------------------------------------
 * Set the maximum consecutive pauses before retrying a lock
-------------------------------------*/
inline void SystemFutexPThread::pause_count(FutexPauseCount maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline FutexPauseCount SystemFutexPThread::pause_count() const noexcept
{
    return mMaxPauseCount;
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool SystemFutexPThread::try_lock() noexcept
{
    return pthread_mutex_trylock(&mLock) == 0;
}



/*-------------------------------------
 * SystemFutexPThread unlock
-------------------------------------*/
inline void SystemFutexPThread::unlock() noexcept
{
    pthread_mutex_unlock(&mLock);
}

#endif // LS_UTILS_USE_PTHREAD_FUTEX



/*-----------------------------------------------------------------------------
 * SystemFutexWin32
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_FUTEX

/*-------------------------------------
 * Set the maximum consecutive pauses before retrying a lock
-------------------------------------*/
inline void SystemFutexWin32::pause_count(FutexPauseCount maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline FutexPauseCount SystemFutexWin32::pause_count() const noexcept
{
    return mMaxPauseCount;
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool SystemFutexWin32::try_lock() noexcept
{
    return 0 != TryAcquireSRWLockExclusive(&mLock);
}



/*-------------------------------------
 * SystemFutexWin32 unlock
-------------------------------------*/
inline void SystemFutexWin32::unlock() noexcept
{
    ReleaseSRWLockExclusive(&mLock);
}

#endif // LS_UTILS_USE_WINDOWS_FUTEX



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FUTEX_IMPL_HPP */