/*
 * File:   RWLock.cpp
 * Author: hammy
 * Created on July 23, 2025, at 12:11 AM
 */

#include "lightsky/utils/RWLock.hpp"

namespace ls
{
namespace utils
{


/*-----------------------------------------------------------------------------
 * PThreads R/W Semaphore
-----------------------------------------------------------------------------*/
#if defined(LS_OS_UNIX) || defined(LS_OS_LINUX) || defined(LS_OS_MINGW)

/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void SystemRWLock::lock_shared() noexcept
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
void SystemRWLock::lock() noexcept
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



/*-----------------------------------------------------------------------------
 * Windows R/W Semaphore
-----------------------------------------------------------------------------*/
#elif defined(LS_OS_WINDOWS)

/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void SystemRWLock::lock_shared() noexcept
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
void SystemRWLock::lock() noexcept
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
    while (currentPauses <= maxPauses);

    AcquireSRWLockExclusive(&mLock);
}



#endif /* LS_OS_WINDOWS */



/*-----------------------------------------------------------------------------
 * Sharable R/W Lock With Fair Ordering
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void FairRWLock::lock_shared() noexcept
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
void FairRWLock::lock() noexcept
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



} // end utils namespace
} // end ls namespace
