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



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_SHARED_MUTEX_IMPL_HPP */
