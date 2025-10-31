/*
 * File:   FairRWLock.hpp
 * Author: hammy
 *
 * Created on Mar 30, 2023 at 9:40 PM
 */

#ifndef LS_UTILS_FAIR_RW_LOCK_HPP
#define LS_UTILS_FAIR_RW_LOCK_HPP

#include <atomic>

#include "lightsky/setup/Api.h"
#include "lightsky/setup/OS.h"

#ifndef LS_UTILS_USE_PTHREAD_RWLOCK
    #if defined(LS_OS_UNIX) || defined(LS_OS_LINUX) || defined(LS_OS_MINGW)
        #define LS_UTILS_USE_PTHREAD_RWLOCK 1
    #else
        #define LS_UTILS_USE_PTHREAD_RWLOCK 0
    #endif
#endif

#if LS_UTILS_USE_PTHREAD_RWLOCK
        #include <pthread.h>
#endif



#ifndef LS_UTILS_USE_WINDOWS_RWLOCK
    #if defined(LS_OS_WINDOWS)
        #define LS_UTILS_USE_WINDOWS_RWLOCK 1
    #else
        #define LS_UTILS_USE_WINDOWS_RWLOCK 0
    #endif
#endif

#if LS_UTILS_USE_WINDOWS_RWLOCK
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
class RWLock;

#if LS_UTILS_USE_PTHREAD_RWLOCK
    class SystemRWLockPThread;
#else
    #if LS_UTILS_USE_WINDOWS_RWLOCK
        class SystemRWLockWindows;
        typedef SystemRWLockWindows SystemRWLockPThread;
    #else
        typedef RWLock SystemRWLockPThread;
    #endif
#endif

#if LS_UTILS_USE_WINDOWS_RWLOCK
    class SystemRWLockWindows;
#else
    #if LS_UTILS_USE_PTHREAD_RWLOCK
        class SystemRWLockPThread;
        typedef SystemRWLockPThread SystemRWLockWindows;
    #else
        typedef RWLock SystemRWLockWindows;
    #endif
#endif



#if LS_UTILS_USE_WINDOWS_RWLOCK
    // Windows' SRWLock performs better than pthreads on MinGW.
    // Prefer the faster implementation
    typedef SystemRWLockWindows SystemRWLock;
#elif LS_UTILS_USE_PTHREAD_RWLOCK
    typedef SystemRWLockPThread SystemRWLock;
#else
    typedef RWLock SystemRWLock;
#endif


/*-----------------------------------------------------------------------------
 * Sharable R/W Lock
 * Currently implemented as a spinlock with OS-based yielding
-----------------------------------------------------------------------------*/
class alignas(alignof(uint64_t)) RWLock
{
public:
    typedef RWLock native_handle_type;

private:
    enum LockFlags : uint32_t
    {
        LOCK_WRITE_BIT = 0x0001,
        LOCK_TRY_WRITE_BIT = 0x0003
    };

    std::atomic_uint32_t mLockType;
    std::atomic_uint32_t mShareCount;

    static void yield() noexcept;

public:
    ~RWLock() noexcept = default;

    RWLock() noexcept;

    RWLock(const RWLock&) noexcept = delete;

    RWLock(RWLock&&) noexcept = delete;

    RWLock& operator=(const RWLock&) noexcept = delete;

    RWLock& operator=(RWLock&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};



/*-----------------------------------------------------------------------------
 * Pthread-based R/W Lock
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_RWLOCK

class alignas(alignof(uint64_t)) SystemRWLockPThread
{
public:
    typedef pthread_rwlock_t native_handle_type;

private:
    pthread_rwlock_t mLock;

public:
    ~SystemRWLockPThread() noexcept;

    SystemRWLockPThread() noexcept;

    SystemRWLockPThread(const SystemRWLockPThread&) noexcept = delete;

    SystemRWLockPThread(SystemRWLockPThread&&) noexcept = delete;

    SystemRWLockPThread& operator=(const SystemRWLockPThread&) noexcept = delete;

    SystemRWLockPThread& operator=(SystemRWLockPThread&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};

#endif



/*-----------------------------------------------------------------------------
 * Windows-Native RWLock
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_RWLOCK

class alignas(alignof(uint64_t)) SystemRWLockWindows
{
public:
    typedef SRWLOCK native_handle_type;

    private:
        alignas(alignof(SRWLOCK)) SRWLOCK mLock;

public:
    ~SystemRWLockWindows() noexcept;

    SystemRWLockWindows() noexcept;

    SystemRWLockWindows(const SystemRWLockWindows&) noexcept = delete;

    SystemRWLockWindows(SystemRWLockWindows&&) noexcept = delete;

    SystemRWLockWindows& operator=(const SystemRWLockWindows&) noexcept = delete;

    SystemRWLockWindows& operator=(SystemRWLockWindows&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};

#endif



/*-----------------------------------------------------------------------------
 * Sharable R/W Lock With Fair Ordering
 * Currently implemented as a spinlock with OS-based yielding
-----------------------------------------------------------------------------*/
class alignas(alignof(uint64_t)) FairRWLock
{
  public:
    typedef uint64_t native_handle_type;

  private:
    enum LockFlags : uint8_t
    {
        LOCK_WRITE_BIT = 0x01,
        LOCK_TRY_WRITE_BIT = 0x03
    };

    struct alignas(alignof(uint64_t)) LockFields
    {
        std::atomic_uint8_t lockType;
        std::atomic_uint8_t typePadding;
        std::atomic_uint16_t shareCount;
        std::atomic_uint16_t nextLockId;
        std::atomic_uint16_t currentLockId;
    };

    union
    {
        uint64_t mLockBits;
        LockFields mLockFields;
    };

  public:
    ~FairRWLock() noexcept = default;

    FairRWLock() noexcept;

    FairRWLock(const FairRWLock&) noexcept = delete;

    FairRWLock(FairRWLock&&) noexcept = delete;

    FairRWLock& operator=(const FairRWLock&) noexcept = delete;

    FairRWLock& operator=(FairRWLock&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};



/*-----------------------------------------------------------------------------
 * Shared Lock Guard
-----------------------------------------------------------------------------*/
template <typename RWLockType>
class LockGuardShared
{
public:
    typedef RWLockType lock_type;

private:
    RWLockType& mRWLock;

public:
    ~LockGuardShared() noexcept;

    LockGuardShared() noexcept = delete;

    LockGuardShared(RWLockType& rwLock) noexcept;

    LockGuardShared(const LockGuardShared&) noexcept = delete;

    LockGuardShared(LockGuardShared&&) noexcept = delete;

    LockGuardShared& operator=(const LockGuardShared&) noexcept = delete;

    LockGuardShared& operator=(LockGuardShared&&) noexcept = delete;
};



/*-----------------------------------------------------------------------------
 * Exclusive Lock Guard
-----------------------------------------------------------------------------*/
template <typename RWLockType>
class LockGuardExclusive
{
public:
    typedef RWLockType lock_type;

private:
    RWLockType& mRWLock;

public:
    ~LockGuardExclusive() noexcept;

    LockGuardExclusive() noexcept = delete;

    LockGuardExclusive(RWLockType& rwLock) noexcept;

    LockGuardExclusive(const LockGuardExclusive&) noexcept = delete;

    LockGuardExclusive(LockGuardExclusive&&) noexcept = delete;

    LockGuardExclusive& operator=(const LockGuardExclusive&) noexcept = delete;

    LockGuardExclusive& operator=(LockGuardExclusive&&) noexcept = delete;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/RWLockImpl.hpp"

#endif /* LS_UTILS_FAIR_RW_LOCK_HPP */
