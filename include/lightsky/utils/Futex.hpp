
#ifndef LS_UTILS_FUTEX_HPP
#define LS_UTILS_FUTEX_HPP

#include <atomic>

#include "lightsky/utils/SpinLock.hpp"



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief A Futex attempts to mimic a mutex object but will attempt to run in
 * user-space rather than switch to kernel-space unless necessary.
-----------------------------------------------------------------------------*/
class Futex
{
  private:
    ls::utils::SpinLock mLock;

    uint_fast64_t mMaxPauseCount;

  public:
    ~Futex() noexcept;

    Futex(uint_fast64_t maxPauses = 16) noexcept;

    Futex(const Futex&) = delete;

    Futex(Futex&&) = delete;

    Futex& operator=(const Futex&) = delete;

    Futex& operator=(Futex&&) = delete;

    void pause_count(uint_fast64_t maxPauses) noexcept;

    uint_fast64_t pause_count() const noexcept;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/FutexImpl.hpp"

#endif /* LS_UTILS_FUTEX_HPP */
