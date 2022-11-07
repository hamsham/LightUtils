
#ifndef LS_UTILS_SPINLOCK_IMPL_HPP
#define LS_UTILS_SPINLOCK_IMPL_HPP

namespace ls
{
namespace utils
{



/*-------------------------------------
 * Mutex Lock
-------------------------------------*/
inline void SpinLock::lock() noexcept
{
    while (mLock.test_and_set(std::memory_order_acq_rel))
    {
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool SpinLock::try_lock() noexcept
{
    return !mLock.test_and_set(std::memory_order_acq_rel);
}



/*-------------------------------------
 * Mutex unlock
-------------------------------------*/
inline void SpinLock::unlock() noexcept
{
    mLock.clear(std::memory_order_release);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_SPINLOCK_IMPL_HPP */
