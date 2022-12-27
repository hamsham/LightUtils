
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
class alignas(64) SpinLock
{
  private:
    std::atomic<uint_fast32_t> mLock{0};
    char mPadding[64ull - sizeof(mLock)];

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

static_assert(sizeof(SpinLock) == 64, "Incorrect size for SpinLock type.");
static_assert(alignof(SpinLock) == 64, "Incorrect alignment for SpinLock type.");



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/SpinLockImpl.hpp"

#endif /* LS_UTILS_SPINLOCK_HPP */
