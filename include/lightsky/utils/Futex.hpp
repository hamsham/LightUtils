
#ifndef LS_UTILS_FUTEX_HPP
#define LS_UTILS_FUTEX_HPP

#include <atomic>

#include "lightsky/setup/OS.h"
#include "lightsky/setup/Compiler.h"

#if defined(LS_OS_LINUX) && defined(LS_COMPILER_GNU)
    #ifndef LS_UTILS_USE_LINUX_FUTEX
        #define LS_UTILS_USE_LINUX_FUTEX 1
    #endif
#endif

#if defined(LS_COMPILER_GNU)
    #ifndef LS_UTILS_USE_PTHREAD_FUTEX
        #define LS_UTILS_USE_PTHREAD_FUTEX 1
    #endif
#endif

#if defined(LS_OS_WINDOWS)
    #ifndef LS_UTILS_USE_WINDOWS_FUTEX
        #define LS_UTILS_USE_WINDOWS_FUTEX 1
    #endif
#endif

#if LS_UTILS_USE_PTHREAD_FUTEX
    #include <pthread.h> // pthread_mutex_t
#endif

#if LS_UTILS_USE_WINDOWS_FUTEX
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif /* WIN32_LEAN_AND_MEAN */

    // Typycal windows bullshit
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif /* NOMINMAX */

    #include <Windows.h> // SRWLOCK
#endif


namespace ls
{
namespace utils
{



class Futex;

#if LS_UTILS_USE_LINUX_FUTEX
    class SystemFutexLinux;
#else
    typedef Futex SystemFutexLinux;
#endif

#if LS_UTILS_USE_PTHREAD_FUTEX
    class SystemFutexLinux;
#else
    typedef Futex SystemFutexPthread;
#endif

#if LS_UTILS_USE_WINDOWS_FUTEX
    class SystemFutexLinux;
#else
    typedef Futex SystemFutexWin32;
#endif



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
class alignas(alignof(uint32_t)) Futex
{
  private:
    alignas(alignof(uint32_t)) std::atomic<uint32_t> mLock;
    alignas(alignof(uint32_t)) FutexPauseCount mMaxPauseCount;

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



/**----------------------------------------------------------------------------
 * @brief Exclusive lock type based on Linux's Futex
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_LINUX_FUTEX

class alignas(alignof(uint32_t)) SystemFutexLinux
{
  private:
    alignas(alignof(uint32_t)) uint32_t mLock;
    alignas(alignof(int32_t)) FutexPauseCount mMaxPauseCount;

  public:
    ~SystemFutexLinux() noexcept;

    SystemFutexLinux(FutexPauseCount maxPauses = FutexPauseCount::FUTEX_PAUSE_COUNT_32) noexcept;

    SystemFutexLinux(const SystemFutexLinux&) = delete;

    SystemFutexLinux(SystemFutexLinux&&) = delete;

    SystemFutexLinux& operator=(const SystemFutexLinux&) = delete;

    SystemFutexLinux& operator=(SystemFutexLinux&&) = delete;

    void pause_count(FutexPauseCount maxPauses) noexcept;

    FutexPauseCount pause_count() const noexcept;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};

#endif



/**----------------------------------------------------------------------------
 * @brief Exclusive lock type based on PThreads pthread_mutex_t
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_FUTEX

class alignas(alignof(uint64_t)) SystemFutexPthread
{
  private:
    alignas(alignof(pthread_mutex_t)) pthread_mutex_t mLock;
    alignas(alignof(uint64_t)) FutexPauseCount mMaxPauseCount;

  public:
    ~SystemFutexPthread() noexcept;

    SystemFutexPthread(FutexPauseCount maxPauses = FutexPauseCount::FUTEX_PAUSE_COUNT_32) noexcept;

    SystemFutexPthread(const SystemFutexPthread&) = delete;

    SystemFutexPthread(SystemFutexPthread&&) = delete;

    SystemFutexPthread& operator=(const SystemFutexPthread&) = delete;

    SystemFutexPthread& operator=(SystemFutexPthread&&) = delete;

    void pause_count(FutexPauseCount maxPauses) noexcept;

    FutexPauseCount pause_count() const noexcept;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};

#endif



/**----------------------------------------------------------------------------
 * @brief Exclusive lock type based on Windows' SRWLOCK
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_FUTEX

class alignas(alignof(uint32_t)) SystemFutexWin32
{
  private:
    alignas(alignof(SRWLOCK)) SRWLOCK mLock;
    alignas(alignof(int32_t)) FutexPauseCount mMaxPauseCount;

  public:
    ~SystemFutexWin32() noexcept;

    SystemFutexWin32(FutexPauseCount maxPauses = FutexPauseCount::FUTEX_PAUSE_COUNT_32) noexcept;

    SystemFutexWin32(const SystemFutexWin32&) = delete;

    SystemFutexWin32(SystemFutexWin32&&) = delete;

    SystemFutexWin32& operator=(const SystemFutexWin32&) = delete;

    SystemFutexWin32& operator=(SystemFutexWin32&&) = delete;

    void pause_count(FutexPauseCount maxPauses) noexcept;

    FutexPauseCount pause_count() const noexcept;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};

#endif



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/FutexImpl.hpp"

#endif /* LS_UTILS_FUTEX_HPP */
