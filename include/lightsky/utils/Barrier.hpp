/*
 * File:   Barrier.hpp
 * Author: miles
 * Created on November 02, 2025, at 1:37 p.m.
 */

#ifndef LS_UTILS_BARRIER_HPP
#define LS_UTILS_BARRIER_HPP

#include <atomic>

#include "lightsky/setup/Api.h"
#include "lightsky/setup/OS.h"

#ifndef LS_UTILS_USE_PTHREAD_BARRIER
    #if defined(LS_OS_UNIX) || defined(LS_OS_LINUX) || defined(LS_OS_MINGW)
        #define LS_UTILS_USE_PTHREAD_BARRIER 1
    #else
        #define LS_UTILS_USE_PTHREADBARRIER 0
    #endif
#endif

#if LS_UTILS_USE_PTHREAD_BARRIER
        #include <pthread.h>
#endif



#ifndef LS_UTILS_USE_WINDOWS_BARRIER
    #if defined(LS_OS_WINDOWS)
        #define LS_UTILS_USE_WINDOWS_BARRIER 1
    #else
        #define LS_UTILS_USE_WINDOWSBARRIER 0
    #endif
#endif

#if LS_UTILS_USE_WINDOWS_BARRIER
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif /* WIN32_LEAN_AND_MEAN */

    // Typycal windows bullshit
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif /* NOMINMAX */

    #include <Windows.h>
#endif



namespace ls
{
namespace utils
{

/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class Barrier;

#if LS_UTILS_USE_PTHREAD_BARRIER
    class SystemBarrierPThread;
#else
    #if LS_UTILS_USE_WINDOWS_BARRIER
        class SystemBarrierWindows;
        typedef SystemBarrierWindows SystemBarrierPThread;
    #else
        typedef Barrier SystemBarrierPThread;
    #endif
#endif

#if LS_UTILS_USE_WINDOWS_BARRIER
    class SystemBarrierWindows;
#else
    #if LS_UTILS_USE_PTHREAD_BARRIER
        class SystemBarrierPThread;
        typedef SystemBarrierPThread SystemBarrierWindows;
    #else
        typedef Barrier SystemBarrierWindows;
    #endif
#endif



#if LS_UTILS_USE_WINDOWS_BARRIER
    typedef SystemBarrierWindows SystemBarrier;
#elif LS_UTILS_USE_PTHREAD_BARRIER
    typedef SystemBarrierPThread SystemBarrier;
#else
    typedef Barrier SystemBarrier;
#endif


/*-----------------------------------------------------------------------------
 * Sharable R/W Lock
 * Currently implemented as a spinlock with OS-based yielding
-----------------------------------------------------------------------------*/
class alignas(alignof(uint64_t)) Barrier
{
public:
    typedef std::atomic_uint32_t native_handle_type;

private:
    std::atomic_uint32_t mBarrierCount;
    const uint32_t mThreadCount;

public:
    ~Barrier() noexcept = default;

    Barrier() noexcept = delete;

    Barrier(uint32_t numThreads) noexcept;

    Barrier(const Barrier&) noexcept = delete;

    Barrier(Barrier&&) noexcept = delete;

    Barrier& operator=(const Barrier&) noexcept = delete;

    Barrier& operator=(Barrier&&) noexcept = delete;

    void wait() noexcept;

    uint32_t num_waiting_threads() const noexcept;

    uint32_t num_required_threads() const noexcept;

    const native_handle_type& native_handle() const noexcept;
};



/*-----------------------------------------------------------------------------
 * Pthread-based R/W Lock
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_BARRIER

class alignas(alignof(uint64_t)) SystemBarrierPThread
{
public:
    typedef pthread_barrier_t native_handle_type;

private:
    pthread_barrier_t mBarrier;
    std::atomic_uint32_t mBarrierCount;
    const uint32_t mThreadCount;

public:
    ~SystemBarrierPThread() noexcept;

    SystemBarrierPThread() noexcept = delete;

    SystemBarrierPThread(uint32_t numThreads) noexcept;

    SystemBarrierPThread(const SystemBarrierPThread&) noexcept = delete;

    SystemBarrierPThread(SystemBarrierPThread&&) noexcept = delete;

    SystemBarrierPThread& operator=(const SystemBarrierPThread&) noexcept = delete;

    SystemBarrierPThread& operator=(SystemBarrierPThread&&) noexcept = delete;

    void wait() noexcept;

    uint32_t num_waiting_threads() const noexcept;

    uint32_t num_required_threads() const noexcept;

    const native_handle_type& native_handle() const noexcept;

    native_handle_type& native_handle() noexcept;
};

#endif



/*-----------------------------------------------------------------------------
 * Windows-Native Barrier
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_BARRIER

class alignas(alignof(uint64_t)) SystemBarrierWindows
{
public:
    typedef SYNCHRONIZATION_BARRIER native_handle_type;

private:
    SYNCHRONIZATION_BARRIER mBarrier;
    std::atomic_uint32_t mBarrierCount;
    const uint32_t mThreadCount;

public:
    ~SystemBarrierWindows() noexcept;

    SystemBarrierWindows() noexcept = delete;

    SystemBarrierWindows(uint32_t numThreads) noexcept;

    SystemBarrierWindows(const SystemBarrierWindows&) noexcept = delete;

    SystemBarrierWindows(SystemBarrierWindows&&) noexcept = delete;

    SystemBarrierWindows& operator=(const SystemBarrierWindows&) noexcept = delete;

    SystemBarrierWindows& operator=(SystemBarrierWindows&&) noexcept = delete;

    void wait() noexcept;

    uint32_t num_waiting_threads() const noexcept;

    uint32_t num_required_threads() const noexcept;

    const native_handle_type& native_handle() const noexcept;

    native_handle_type& native_handle() noexcept;
};

#endif



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/BarrierImpl.hpp"

#endif /* LS_UTILS_BARRIER_HPP */
