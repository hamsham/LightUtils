
#include <thread>

#include "lightsky/setup/Macros.h"
#include "lightsky/utils/Futex.hpp"

#if defined(LS_UTILS_USE_LINUX_FUTEX)
    extern "C"
    {
        #include <linux/futex.h>
        #include <sys/syscall.h>
        #include <sys/time.h>
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
 * SystemFutex
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemFutex::~SystemFutex() noexcept
{
    #if defined(LS_UTILS_USE_LINUX_FUTEX)
        __atomic_store_n(&mLock, 0u, __ATOMIC_RELEASE);
    #endif
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemFutex::SystemFutex(FutexPauseCount maxPauses) noexcept :
    #if defined(LS_UTILS_USE_LINUX_FUTEX)
        mLock{0},
    #endif
        mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses}
{
    #if defined(LS_UTILS_USE_WINDOWS_FUTEX)
        InitializeSRWLock(&mLock);
    #endif
}



/*-------------------------------------
 * SystemFutex Lock
-------------------------------------*/
void SystemFutex::lock() noexcept
{
    #if defined(LS_UTILS_USE_LINUX_FUTEX)
        const unsigned maxPauses = static_cast<unsigned>(mMaxPauseCount);
        unsigned currentPauses = 1;
        const pid_t tid = gettid();

        do
        {
            uint32_t tmp = 0u;
            if (__atomic_compare_exchange_n(&mLock, &tmp, tid, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
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

        while (syscall(SYS_futex, &mLock, FUTEX_LOCK_PI_PRIVATE, 0u, nullptr, nullptr, 0) != 0) {}

    #elif defined(LS_UTILS_USE_WINDOWS_FUTEX)
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

    #endif
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
bool SystemFutex::try_lock() noexcept
{
    #if defined(LS_UTILS_USE_LINUX_FUTEX)
        const pid_t tid = gettid();
        uint32_t tmp = 0u;

        #if 0
            bool haveLock = true;

            if (!__atomic_compare_exchange_n(&mLock, &tmp, tid, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
            {
                haveLock = syscall(SYS_futex, &mLock, FUTEX_TRYLOCK_PI_PRIVATE, 0u, nullptr, nullptr, 0) == 0;
            }

            return haveLock;
        #else
            return __atomic_compare_exchange_n(&mLock, &tmp, tid, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
        #endif

    #elif defined(LS_UTILS_USE_WINDOWS_FUTEX)
        return 0 != TryAcquireSRWLockExclusive(&mLock);

    #endif
}



/*-------------------------------------
 * SystemFutex unlock
-------------------------------------*/
void SystemFutex::unlock() noexcept
{
    #if defined(LS_UTILS_USE_LINUX_FUTEX)
        const uint32_t tid = (uint32_t)gettid();
        uint32_t tmp = tid;

        if (!__atomic_compare_exchange_n(&mLock, &tmp, 0u, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
        {
            syscall(SYS_futex, &mLock, FUTEX_UNLOCK_PI_PRIVATE, 0u, nullptr, nullptr, 0);
        }

    #elif defined(LS_UTILS_USE_WINDOWS_FUTEX)
        ReleaseSRWLockExclusive(&mLock);

    #endif
}



} // end utils namespace
} // end ls namespace
