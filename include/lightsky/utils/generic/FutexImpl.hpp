
#ifndef LS_UTILS_FUTEX_IMPL_HPP
#define LS_UTILS_FUTEX_IMPL_HPP

#include "lightsky/setup/CPU.h"



namespace ls
{
namespace utils
{



/*-------------------------------------
 * Futex Lock
-------------------------------------*/
inline void Futex::lock() noexcept
{
    const uint_fast64_t maxPauses = static_cast<uint_fast64_t>(mMaxPauseCount);
    uint_fast64_t currentPauses = 1ull;

    while (!mLock.try_lock())
    {
        switch (currentPauses)
        {
            case 32: ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
            case 16: ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
            case 8:  ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
            case 4:  ls::setup::cpu_yield();
                     ls::setup::cpu_yield();
            case 2:  ls::setup::cpu_yield();
            case 1:  ls::setup::cpu_yield();
        }

        if (currentPauses < maxPauses)
        {
            currentPauses <<= 1ull;
        }
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool Futex::try_lock() noexcept
{
    return mLock.try_lock();
}



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
 * Futex unlock
-------------------------------------*/
inline void Futex::unlock() noexcept
{
    mLock.unlock();
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FUTEX_IMPL_HPP */