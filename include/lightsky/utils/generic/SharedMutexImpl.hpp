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
    mLock{},
    mWaitCond{}
{}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
template <typename mutex_type>
void SharedMutexType<mutex_type>::lock_shared() noexcept
{
    unsigned long long readVal;

    do
    {
        readVal = mShareCount.fetch_add(1ull, std::memory_order_acq_rel);

        if ((readVal & LOCK_WRITE_BIT))
        {
            std::unique_lock<native_handle_type> guard{mLock};
            mWaitCond.wait(guard, [this]() noexcept -> bool
            {
                return !(mShareCount.load(std::memory_order_acquire) & LOCK_WRITE_BIT);
            });
        }

        readVal = mShareCount.load(std::memory_order_acquire);
    }
    while (readVal & LOCK_WRITE_BIT);

    if (!readVal)
    {
        while (!mLock.try_lock())
        {
            mWaitCond.notify_one();
        }
    }
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
template <typename mutex_type>
void SharedMutexType<mutex_type>::lock() noexcept
{
    do
    {
        unsigned long long writeVal = 0ull;
        if (mShareCount.compare_exchange_strong(writeVal, LOCK_WRITE_BIT, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            break;
        }

        std::unique_lock<native_handle_type> guard{mLock};
        mWaitCond.wait(guard, [this]() noexcept->bool
        {
            return !mShareCount.load(std::memory_order_acquire);
        });
    }
    while (true);

    while (!mLock.try_lock())
    {
        mWaitCond.notify_one();
    }
}



/*-------------------------------------
 * Non-Exclusive Lock Attempt
-------------------------------------*/
template <typename mutex_type>
bool SharedMutexType<mutex_type>::try_lock_shared() noexcept
{
    bool result = false;
    unsigned long long readVal = mShareCount.fetch_add(1ull, std::memory_order_acq_rel);

    if (readVal & LOCK_WRITE_BIT)
    {
        mShareCount.fetch_sub(1ull, std::memory_order_acq_rel);
    }
    else
    {
        if (!readVal)
        {
            while (!mLock.try_lock())
            {
                mWaitCond.notify_one();
            }
        }

        result = true;
    }

    return result;
}



/*-------------------------------------
 * Exclusive Lock Attempt
-------------------------------------*/
template <typename mutex_type>
bool SharedMutexType<mutex_type>::try_lock() noexcept
{
    bool result = false;
    unsigned long long writeVal = 0;

    if (mShareCount.compare_exchange_strong(writeVal, LOCK_WRITE_BIT, std::memory_order_acq_rel, std::memory_order_relaxed))
    {
        while (!mLock.try_lock())
        {
            mWaitCond.notify_one();
        }

        result = true;
    }

    return result;
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
        mLock.unlock();
        mWaitCond.notify_all();
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

    mLock.unlock();
    mWaitCond.notify_all();
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
