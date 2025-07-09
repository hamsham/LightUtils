
#include <thread>

#include "lightsky/setup/Macros.h"
#include "lightsky/utils/Futex.hpp"

#if LS_UTILS_USE_LINUX_FUTEX
    extern "C"
    {
        #include <linux/futex.h>
        #include <sys/syscall.h>
        #include <unistd.h>
        #include <sched.h>
    }

#endif

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Futex
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
Futex::~Futex() noexcept
{
    mLock.store(0, std::memory_order_release);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Futex::Futex(FutexPauseCount maxPauses) noexcept :
    mLock{0},
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses}
{
}



/*-------------------------------------
 * Futex Lock
-------------------------------------*/
void Futex::lock() noexcept
{
    const uint32_t maxPauses = static_cast<uint32_t>(mMaxPauseCount);
    uint32_t currentPauses = 1;
    uint32_t tmp;

    do
    {
        tmp = 0;
        if (mLock.compare_exchange_strong(tmp, 1, std::memory_order_acquire, std::memory_order_relaxed))
        {
            return;
        }

        for (uint32_t i = 0; i < currentPauses; ++i)
        {
            std::this_thread::yield();
        }

        currentPauses <<= 1u;
    }
    while (currentPauses <= maxPauses);

    while (true)
    {
        tmp = 0;
        if (mLock.compare_exchange_strong(tmp, 1, std::memory_order_acquire, std::memory_order_relaxed))
        {
            break;
        }

        for (uint32_t i = 0; i < maxPauses; ++i)
        {
            ls::setup::cpu_yield();
            std::this_thread::yield();
        }
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
bool Futex::try_lock() noexcept
{
    uint32_t tmp = 0;
    return mLock.compare_exchange_strong(tmp, 1, std::memory_order_seq_cst, std::memory_order_relaxed);
}



/*-------------------------------------
 * Futex unlock
-------------------------------------*/
void Futex::unlock() noexcept
{
    mLock.store(0, std::memory_order_release);
}



/*-----------------------------------------------------------------------------
 * SystemFutexLinux
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_LINUX_FUTEX

/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemFutexLinux::~SystemFutexLinux() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemFutexLinux::SystemFutexLinux(FutexPauseCount maxPauses) noexcept :
    mLock{0},
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses}
{
}



/*-------------------------------------
 * SystemFutexLinux Lock
-------------------------------------*/
void SystemFutexLinux::lock() noexcept
{
    const unsigned maxPauses = static_cast<unsigned>(mMaxPauseCount);
    unsigned currentPauses = 1;

    do
    {
        if (try_lock())
        {
            return;
        }

        int yieldMask = 0;
        for (unsigned i = 0; i < currentPauses; ++i)
        {
            yieldMask |= sched_yield();
        }

        if (!yieldMask)
        {
            currentPauses <<= 1;
        }
    }
    while (currentPauses <= maxPauses);

    while (!try_lock())
    {
        syscall(SYS_futex, &mLock, FUTEX_WAIT_PRIVATE, 1u, nullptr, nullptr, 0);
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
bool SystemFutexLinux::try_lock() noexcept
{
    uint32_t tmp = 0u;
    return __atomic_compare_exchange_n(&mLock, &tmp, 1u, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}



/*-------------------------------------
 * SystemFutexLinux unlock
-------------------------------------*/
void SystemFutexLinux::unlock() noexcept
{
    __atomic_store_n(&mLock, 0u, __ATOMIC_RELEASE);
    syscall(SYS_futex, &mLock, FUTEX_WAKE_PRIVATE, 1u, nullptr, nullptr, 0);
}


#endif /* LS_UTILS_USE_LINUX_FUTEX */



/*-----------------------------------------------------------------------------
 * SystemFutexPthread
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_FUTEX

/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemFutexPthread::~SystemFutexPthread() noexcept
{
    pthread_mutex_destroy(&mLock);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemFutexPthread::SystemFutexPthread(FutexPauseCount maxPauses) noexcept :
    mLock{},
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses}
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    #if defined(LS_OS_LINUX)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
    #else
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    #endif

    pthread_mutex_init(&mLock, &attr);
    pthread_mutexattr_destroy(&attr);
}



/*-------------------------------------
 * SystemFutexPthread Lock
-------------------------------------*/
void SystemFutexPthread::lock() noexcept
{
#if 1
    const unsigned maxPauses = static_cast<unsigned>(mMaxPauseCount);
    unsigned currentPauses = 1;
    do
    {
        if (pthread_mutex_trylock(&mLock) == 0)
        {
            return;
        }

        int yieldMask = 0;
        for (unsigned i = 0; i < currentPauses; ++i)
        {
            yieldMask |= sched_yield();
        }

        if (!yieldMask)
        {
            currentPauses <<= 1;
        }
    }
    while (currentPauses <= maxPauses);
#endif

    while (pthread_mutex_lock(&mLock) != 0)
    {
        std::this_thread::yield();
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
bool SystemFutexPthread::try_lock() noexcept
{
    return pthread_mutex_trylock(&mLock) == 0;
}



/*-------------------------------------
 * SystemFutexPthread unlock
-------------------------------------*/
void SystemFutexPthread::unlock() noexcept
{
    pthread_mutex_unlock(&mLock);
}



#endif /* LS_UTILS_USE_PTHREAD_FUTEX */



/*-----------------------------------------------------------------------------
 * SystemFutexWin32
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_FUTEX

/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemFutexWin32::~SystemFutexWin32() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemFutexWin32::SystemFutexWin32(FutexPauseCount maxPauses) noexcept :
    mLock{},
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses}
{
    InitializeSRWLock(&mLock);
}



/*-------------------------------------
 * SystemFutexWin32 Lock
-------------------------------------*/
void SystemFutexWin32::lock() noexcept
{
    const int32_t maxPauses = static_cast<int32_t>(mMaxPauseCount);
    int32_t currentPauses = 1;

    while (!TryAcquireSRWLockExclusive(&mLock))
    {
        if (currentPauses < maxPauses)
        {
            for (int32_t i = 0; i < currentPauses; ++i)
            {
                ls::setup::cpu_yield();
            }

            currentPauses <<= 1;
        }
        else
        {
            AcquireSRWLockExclusive(&mLock);
            break;
        }
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
bool SystemFutexWin32::try_lock() noexcept
{
    return 0 != TryAcquireSRWLockExclusive(&mLock);
}



/*-------------------------------------
 * SystemFutexWin32 unlock
-------------------------------------*/
void SystemFutexWin32::unlock() noexcept
{
    ReleaseSRWLockExclusive(&mLock);
}



#endif /* LS_UTILS_USE_WINDOWS_FUTEX */



} // end utils namespace
} // end ls namespace
