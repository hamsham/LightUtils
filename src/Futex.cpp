
#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Futex.hpp"

#include <cstdio>
#include <cstring>

#if defined(LS_UTILS_USE_LINUX_FUTEX)
    extern "C"
    {
        #include <linux/futex.h>
        #include <sys/syscall.h>
        #include <sys/time.h>
        #include <unistd.h>
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
    (void)mPad;

#if defined(LS_UTILS_USE_LINUX_FUTEX)
    __atomic_store_n(&mLock, 0u, __ATOMIC_RELEASE);

#elif defined(LS_UTILS_USE_WINDOWS_FUTEX)
#else
    mLock.store(0, std::memory_order_release);
#endif
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Futex::Futex(FutexPauseCount maxPauses) noexcept :
#if !defined(LS_UTILS_USE_WINDOWS_FUTEX)
    mLock{0},
#endif
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses},
    mPad{0}
{
    #if defined(LS_UTILS_USE_WINDOWS_FUTEX)
        InitializeSRWLock(&mLock);
    #endif
}



/*-------------------------------------
 * Futex Lock
-------------------------------------*/
void Futex::lock() noexcept
{
#if defined(LS_UTILS_USE_LINUX_FUTEX)
    const unsigned maxPauses = static_cast<unsigned>(mMaxPauseCount);
    unsigned currentPauses = 1;

    while (!try_lock())
    {
        if (currentPauses <= maxPauses)
        {
            for (unsigned i = 0; i < currentPauses; ++i)
            {
                std::this_thread::yield();
            }

            currentPauses <<= 1;
        }
        else
        {
            #if 0
                futex_waitv waitVal;
                waitVal.val = 0;
                waitVal.uaddr = reinterpret_cast<uintptr_t>(&mLock);
                waitVal.flags = FUTEX_PRIVATE_FLAG|FUTEX2_SIZE_U32;
                waitVal.__reserved = 0;
                syscall(SYS_futex_waitv, &waitVal, 1, 0, nullptr);
            #else
                syscall(SYS_futex, &mLock, FUTEX_WAIT_PRIVATE, 1u, nullptr, nullptr, 0);
            #endif
        }
    }

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

#else
    const int32_t maxPauses = static_cast<int32_t>(mMaxPauseCount);
    int32_t currentPauses = 1;
    int32_t tmp;

    do
    {
        while (mLock.load(std::memory_order_acquire))
        {
            switch (currentPauses)
            {
                case 32: ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                case 16: ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                case 8:  ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                case 4:  ls::setup::cpu_yield();
                         ls::setup::cpu_yield();
                case 2:  ls::setup::cpu_yield();
                case 1:  ls::setup::cpu_yield();
                default: currentPauses <<= (int)(currentPauses < maxPauses);
            }
        }

        tmp = 0;
    }
    while (!mLock.compare_exchange_weak(tmp, 1, std::memory_order_acquire, std::memory_order_relaxed));

#endif
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
bool Futex::try_lock() noexcept
{
#if defined(LS_UTILS_USE_LINUX_FUTEX)
    uint32_t tmp = 0u;
    return __atomic_compare_exchange_n(&mLock, &tmp, 1u, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);

#elif defined(LS_UTILS_USE_WINDOWS_FUTEX)
    return 0 != TryAcquireSRWLockExclusive(&mLock);

#else
    int32_t tmp = 0;
    return mLock.compare_exchange_strong(tmp, 1, std::memory_order_seq_cst, std::memory_order_relaxed);

#endif
}



/*-------------------------------------
 * Futex unlock
-------------------------------------*/
void Futex::unlock() noexcept
{
#if defined(LS_UTILS_USE_LINUX_FUTEX)
    __atomic_store_n(&mLock, 0u, __ATOMIC_RELEASE);
    syscall(SYS_futex, &mLock, FUTEX_WAKE_PRIVATE, 1u);

#elif defined(LS_UTILS_USE_WINDOWS_FUTEX)
    ReleaseSRWLockExclusive(&mLock);

#else
    mLock.store(0, std::memory_order_release);

#endif
}



} // end utils namespace
} // end ls namespace
