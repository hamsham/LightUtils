
#ifndef LS_UTILS_FUTEX_HPP
#define LS_UTILS_FUTEX_HPP

#include <atomic>

#include "lightsky/utils/SpinLock.hpp"



namespace ls
{
namespace utils
{



/**
 * @brief Futexes implement CPU yielding rather than yield to the OS.
 *
 * When yielding the CPU, we lower the core frequency with hardware
 * instructions (_mm_pause() on x86, __yield() on ARM, and
 * std::this_thread_yield() on unsupported hardware). Use these enum values to
 * control how many yield instructions are sent to the CPU.
 */
enum class FutexPauseCount : uint_fast64_t
{
    FUTEX_PAUSE_COUNT_1,
    FUTEX_PAUSE_COUNT_2,
    FUTEX_PAUSE_COUNT_4,
    FUTEX_PAUSE_COUNT_8,
    FUTEX_PAUSE_COUNT_16,
    FUTEX_PAUSE_COUNT_32,

    FUTEX_PAUSE_COUNT_MAX = FUTEX_PAUSE_COUNT_32,
};



/**----------------------------------------------------------------------------
 * @brief A Futex attempts to mimic a mutex object but will attempt to run in
 * user-space rather than switch to kernel-space unless necessary.
-----------------------------------------------------------------------------*/
class Futex
{
  private:
    ls::utils::SpinLock mLock;

    FutexPauseCount mMaxPauseCount;

  public:
    ~Futex() noexcept;

    Futex(FutexPauseCount maxPauses = FutexPauseCount::FUTEX_PAUSE_COUNT_32) noexcept;

    Futex(const Futex&) = delete;

    Futex(Futex&&) = delete;

    Futex& operator=(const Futex&) = delete;

    Futex& operator=(Futex&&) = delete;

    void pause_count(FutexPauseCount maxPauses) noexcept;

    FutexPauseCount pause_count() const noexcept;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/FutexImpl.hpp"

#endif /* LS_UTILS_FUTEX_HPP */
