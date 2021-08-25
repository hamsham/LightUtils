
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
    const uint_fast64_t maxPauses = mMaxPauseCount;
    uint_fast64_t currentPauses = 1ull;

    while (!mLock.try_lock())
    {
        for (uint_fast64_t i = 0; i < currentPauses; ++i)
        {
            ls::setup::cpu_yield();
        }

        // detect overflow before increasing the pause count
        const uint_fast64_t validPauseCount = currentPauses * 2ull;
        const uint_fast64_t nextPauseCount = ((~0ull/2ull) >= currentPauses) ? validPauseCount : ~0ull;
        currentPauses = nextPauseCount < currentPauses ? nextPauseCount : maxPauses;
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
inline void Futex::pause_count(uint_fast64_t maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline uint_fast64_t Futex::pause_count() const noexcept
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
