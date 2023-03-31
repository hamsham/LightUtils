
#ifndef LS_UTILS_FUTEX_HPP
#define LS_UTILS_FUTEX_HPP

#include <atomic>

#include "lightsky/setup/OS.h"
#include "lightsky/setup/Compiler.h"

#if defined(LS_OS_LINUX) && defined(LS_COMPILER_GNU)
    #ifndef LS_UTILS_USE_FALLBACK_FUTEX
        #define LS_UTILS_HAVE_LINUX_FUTEX 1
    #endif

#elif defined(LS_OS_WINDOWS)
    #ifndef LS_UTILS_USE_FALLBACK_FUTEX
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif /* WIN32_LEAN_AND_MEAN */

        #include <Windows.h>

        #define LS_UTILS_HAVE_WIN32_FUTEX 1
    #endif

#endif


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
enum class FutexPauseCount : int32_t
{
    FUTEX_PAUSE_COUNT_1 = 1ull,
    FUTEX_PAUSE_COUNT_2 = 2ull,
    FUTEX_PAUSE_COUNT_4 = 4ull,
    FUTEX_PAUSE_COUNT_8 = 8ull,
    FUTEX_PAUSE_COUNT_16 = 16ull,
    FUTEX_PAUSE_COUNT_32 = 32ull,

    FUTEX_PAUSE_COUNT_MAX = FUTEX_PAUSE_COUNT_32,
};



/**----------------------------------------------------------------------------
 * @brief A Futex attempts to mimic a mutex object but will attempt to run in
 * user-space rather than switch to kernel-space unless necessary.
-----------------------------------------------------------------------------*/
class alignas(64) Futex
{
  private:
#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    alignas(alignof(int32_t)) int mLock;

#elif defined(LS_UTILS_HAVE_WIN32_FUTEX)
    alignas(alignof(SRWLOCK)) SRWLOCK mLock;


#else
    alignas(alignof(int32_t)) std::atomic<int32_t> mLock;

#endif

    alignas(alignof(int32_t)) FutexPauseCount mMaxPauseCount;

    alignas(alignof(int32_t)) int32_t mPad[16-(sizeof(mLock)+sizeof(FutexPauseCount))];

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

static_assert(sizeof(Futex) == 64, "Incorrect size for futex type.");
static_assert(alignof(Futex) == 64, "Incorrect alignment for futex type.");



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/FutexImpl.hpp"

#endif /* LS_UTILS_FUTEX_HPP */
