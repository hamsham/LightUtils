/*
 * File:   SharedMutexImpl.hpp
 * Author: hammy
 *
 * Created on Jan 06, 2023 at 12:35 AM
 */

#ifndef LS_UTILS_SHARED_MUTEX_IMPL_HPP
#define LS_UTILS_SHARED_MUTEX_IMPL_HPP

#include "lightsky/utils/Assertions.h"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Lockable R/W Semaphore
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename mutex_type>
SharedMutexType<mutex_type>::SharedMutexType() noexcept :
    mShareCount{0ull},
    mLock{}
{}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
template <typename mutex_type>
void SharedMutexType<mutex_type>::lock_shared() noexcept
{
    unsigned long long readVal = mShareCount.fetch_add(1ull, std::memory_order_acq_rel);

    do
    {
        if (0 == (readVal & LOCK_WRITE_BIT))
        {
            break;
        }

        // Lock to produce an OS-scheduled wait
        mLock.lock();
        readVal = mShareCount.load(std::memory_order_acquire);
        mLock.unlock();
    }
    while (true);
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
template <typename mutex_type>
void SharedMutexType<mutex_type>::lock() noexcept
{
    constexpr unsigned maxPauses = 32u;
    unsigned currentPauses = 1u;

    do
    {
        mLock.lock();

        unsigned long long writeVal = 0ull;
        bool amWriter = mShareCount.compare_exchange_strong(writeVal, LOCK_WRITE_BIT, std::memory_order_acq_rel, std::memory_order_relaxed);

        if (amWriter)
        {
            // maintain the lock while writing
            break;
        }

        mLock.unlock();

        for (unsigned i = 0u; i < currentPauses; ++i)
        {
            ls::setup::cpu_yield();
        }

        if (currentPauses < maxPauses)
        {
            currentPauses <<= 1u;
        }
    }
    while (true);
}



/*-------------------------------------
 * Non-Exclusive Lock Attempt
-------------------------------------*/
template <typename mutex_type>
bool SharedMutexType<mutex_type>::try_lock_shared() noexcept
{
    bool result = true;

    const unsigned long long readVal = mShareCount.fetch_add(1ull, std::memory_order_acq_rel);
    if (readVal & LOCK_WRITE_BIT)
    {
        result = false;
        mShareCount.fetch_sub(1ull, std::memory_order_acq_rel);
    }

    return result;
}



/*-------------------------------------
 * Exclusive Lock Attempt
-------------------------------------*/
template <typename mutex_type>
bool SharedMutexType<mutex_type>::try_lock() noexcept
{
    unsigned long long writeVal = 0ull;
    bool amWriter = mShareCount.compare_exchange_strong(writeVal, LOCK_WRITE_BIT, std::memory_order_acq_rel, std::memory_order_relaxed);

    if (amWriter)
    {
        mLock.lock();
    }

    return amWriter;
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
template <typename mutex_type>
void SharedMutexType<mutex_type>::unlock_shared() noexcept
{
    unsigned long long readState = mShareCount.fetch_sub(1ull, std::memory_order_acq_rel);

    if (readState == 1ull)
    {
        //mWaitCond.notify_one();
        //mLock.unlock();
    }


    LS_DEBUG_ASSERT(readState > 0);
    LS_DEBUG_ASSERT(!(readState & LOCK_WRITE_BIT));
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
template <typename mutex_type>
void SharedMutexType<mutex_type>::unlock() noexcept
{
    unsigned long long writeVal = mShareCount.fetch_xor(LOCK_WRITE_BIT, std::memory_order_acq_rel);
    (void)writeVal;

    LS_DEBUG_ASSERT((writeVal & LOCK_WRITE_BIT) == LOCK_WRITE_BIT);

    //mWaitCond.notify_one();
    mLock.unlock();
}



/*-------------------------------------
 * Retrieve the internal lock
-------------------------------------*/
template <typename mutex_type>
const typename SharedMutexType<mutex_type>::native_handle_type& SharedMutexType<mutex_type>::native_handle() const noexcept
{
    return mLock;
}



/*-----------------------------------------------------------------------------
 * Spinnable R/W Semaphore
-----------------------------------------------------------------------------*/
#ifndef LS_UTILS_SHAREDMUTEX2_CPU_YIELD
    #define LS_UTILS_SHAREDMUTEX2_CPU_YIELD 0
#endif

#ifndef LS_UTILS_SHAREDMUTEX2_ENABLE_FAIRNESS
    #define LS_UTILS_SHAREDMUTEX2_ENABLE_FAIRNESS 1
#endif

#ifndef LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK
    #define LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK 1
#endif

#if LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK == 0 && LS_UTILS_SHAREDMUTEX2_ENABLE_FAIRNESS == 0
    #error "LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK must be enabled when LS_UTILS_SHAREDMUTEX2_ENABLE_FAIRNESS is disabled."
#endif

/*-------------------------------------
 * Constructor
-------------------------------------*/
inline SharedMutex2::SharedMutex2() noexcept :
    mLockBits{0}
{}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
inline void SharedMutex2::lock_shared() noexcept
{
    #if LS_UTILS_SHAREDMUTEX2_ENABLE_FAIRNESS
        const uint16_t lockId = mLockFields.currentLockId.fetch_add(1);
        while (lockId != mLockFields.nextLockId.load())
        {
            #if LS_UTILS_SHAREDMUTEX2_CPU_YIELD
                ls::setup::cpu_yield();
            #else
                std::this_thread::yield();
            #endif
        }

        mLockFields.shareCount.fetch_add(1);

        #if LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK
            while (mLockFields.lockType.load())
            {
                #if LS_UTILS_SHAREDMUTEX2_CPU_YIELD
                    ls::setup::cpu_yield();
                #else
                    std::this_thread::yield();
                #endif
            }
        #endif

        mLockFields.nextLockId.fetch_add(1);
    #else

        while (!try_lock_shared())
        {
            #if LS_UTILS_SHAREDMUTEX2_CPU_YIELD
                ls::setup::cpu_yield();
            #else
                std::this_thread::yield();
            #endif
        }
    #endif
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
inline void SharedMutex2::lock() noexcept
{
    #if LS_UTILS_SHAREDMUTEX2_ENABLE_FAIRNESS
        const uint16_t lockId = mLockFields.currentLockId.fetch_add(1);
        while (lockId != mLockFields.nextLockId.load())
        {
            #if LS_UTILS_SHAREDMUTEX2_CPU_YIELD
                ls::setup::cpu_yield();
            #else
                std::this_thread::yield();
            #endif
        }

        #if LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK
            uint16_t writeBit;
            do
            {
                writeBit = 0;
            }
            while (!mLockFields.lockType.compare_exchange_strong(writeBit, LockFlags::LOCK_WRITE_BIT));
        #else

            mLockFields.lockType.store(LockFlags::LOCK_WRITE_BIT);
        #endif

        while (mLockFields.shareCount.load() != 0)
        {
            #if LS_UTILS_SHAREDMUTEX2_CPU_YIELD
                ls::setup::cpu_yield();
            #else
                std::this_thread::yield();
            #endif
        }
    #else

        while (!try_lock())
        {
            #if LS_UTILS_SHAREDMUTEX2_CPU_YIELD
                ls::setup::cpu_yield();
            #else
                std::this_thread::yield();
            #endif
        }
    #endif
}



/*-------------------------------------
 * Attempt Non-Exclusive Lock
-------------------------------------*/
inline bool SharedMutex2::try_lock_shared() noexcept
{
    #if LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK
        mLockFields.shareCount.fetch_add(1);

        const bool haveLock = mLockFields.lockType.load() == 0;
        if (!haveLock)
        {
            mLockFields.shareCount.fetch_sub(1);
        }

        return haveLock;
    #else

        return false;
    #endif
}



/*-------------------------------------
 * Attempt Exclusive Lock
-------------------------------------*/
inline bool SharedMutex2::try_lock() noexcept
{
    #if LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK
        uint16_t writeBit = 0;
        if (!mLockFields.lockType.compare_exchange_strong(writeBit, LockFlags::LOCK_TRY_WRITE_BIT))
        {
            return false;
        }

        bool haveLock = mLockFields.shareCount.load() == 0;
        if (!haveLock)
        {
            mLockFields.lockType.store(0);
        }

        return haveLock;
    #else

        return false;
    #endif
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
inline void SharedMutex2::unlock_shared() noexcept
{
    mLockFields.shareCount.fetch_sub(1);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
inline void SharedMutex2::unlock() noexcept
{
    const uint16_t lockType = mLockFields.lockType.exchange(0);

    #if LS_UTILS_SHAREDMUTEX2_ENABLE_TRY_LOCK
        if (lockType == LockFlags::LOCK_WRITE_BIT)
        {
            mLockFields.nextLockId.fetch_add(1);
        }
    #else

        mLockFields.nextLockId.fetch_add(1);
    #endif
}



/*-------------------------------------
 * Handle Type
-------------------------------------*/
inline const SharedMutex2::native_handle_type& SharedMutex2::native_handle() const noexcept
{
    return mLockBits;
}


} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_SHARED_MUTEX_IMPL_HPP */
