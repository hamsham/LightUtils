
#ifndef LS_UTILS_SPINLOCK_HPP
#define LS_UTILS_SPINLOCK_HPP

#include <atomic>



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief A SpinLock attempts to mimic a mutex object but will allow the CPU
 * to continue running while it is locked.
-----------------------------------------------------------------------------*/
class SpinLock
{
  private:
    std::atomic_flag mLock = ATOMIC_FLAG_INIT;

  public:
    ~SpinLock() noexcept;

    SpinLock() noexcept = default;

    SpinLock(const SpinLock&) = delete;

    SpinLock(SpinLock&&) = delete;

    SpinLock& operator=(const SpinLock&) = delete;

    SpinLock& operator=(SpinLock&&) = delete;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/SpinLockImpl.hpp"

#endif /* LS_UTILS_SPINLOCK_HPP */
