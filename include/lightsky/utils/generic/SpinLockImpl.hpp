
#ifndef LS_UTILS_SPINLOCK_IMPL_HPP
#define LS_UTILS_SPINLOCK_IMPL_HPP
#include "lightsky/setup/CPU.h"

namespace ls
{
namespace utils
{



/*-------------------------------------
 * Mutex Lock
-------------------------------------*/
inline void SpinLock::lock() noexcept
{
    while (!try_lock())
    {
        std::this_thread::yield();
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool SpinLock::try_lock() noexcept
{
    uint_fast32_t cachedVal = 0;
    return mLock.compare_exchange_strong(cachedVal, 1, std::memory_order_acq_rel);
}



/*-------------------------------------
 * Mutex unlock
-------------------------------------*/
inline void SpinLock::unlock() noexcept
{
    mLock.store(0, std::memory_order_release);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_SPINLOCK_IMPL_HPP */
