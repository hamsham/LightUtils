
#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Futex.hpp"

#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
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

#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    __atomic_store_n(&mLock, 0, __ATOMIC_RELEASE);

#elif defined(LS_UTILS_HAVE_WIN32_FUTEX)
#else
    mLock.store(0, std::memory_order_release);
#endif
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Futex::Futex(FutexPauseCount maxPauses) noexcept :
#if !defined(LS_UTILS_HAVE_WIN32_FUTEX)
    mLock{0},
#endif
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses},
    mPad{0}
{
    #if defined(LS_UTILS_HAVE_WIN32_FUTEX)
        InitializeSRWLock(&mLock);
    #endif
}



/*-------------------------------------
 * Futex Lock
-------------------------------------*/
void Futex::lock() noexcept
{
#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    const int32_t maxPauses = static_cast<int32_t>(mMaxPauseCount);
    int32_t currentPauses = 1;

    do
    {
        int32_t tmp = 0;
        if (__atomic_compare_exchange_n(&mLock, &tmp, 1, true, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
        {
            break;
        }

        if (currentPauses < maxPauses)
        {
            #if 0
                const int32_t status = (int32_t)syscall(SYS_futex, &mLock, FUTEX_WAIT_BITSET_PRIVATE, 1, currentPauses, nullptr, FUTEX_BITSET_MATCH_ANY);
            #else
            timespec timeout;
                timeout.tv_sec = 0;
                timeout.tv_nsec = currentPauses;
                const int32_t status = (int32_t)syscall(SYS_futex, &mLock, FUTEX_WAIT_BITSET_PRIVATE, 1u, &timeout, nullptr, FUTEX_BITSET_MATCH_ANY);
            #endif

            if (status != 0)
            {
                currentPauses <<= 1;
            }
        }
        else
        {
            syscall(SYS_futex, &mLock, FUTEX_WAIT_PRIVATE, 1u, nullptr);
        }
    }
    while (true);

#elif defined(LS_UTILS_HAVE_WIN32_FUTEX)
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
#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    int32_t tmp = 0;
    return __atomic_compare_exchange_n(&mLock, &tmp, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

#elif defined(LS_UTILS_HAVE_WIN32_FUTEX)
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
#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    __atomic_store_n(&mLock, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &mLock, FUTEX_WAKE_PRIVATE, 1u);

#elif defined(LS_UTILS_HAVE_WIN32_FUTEX)
    ReleaseSRWLockExclusive(&mLock);

#else
    mLock.store(0, std::memory_order_release);

#endif
}



} // end utils namespace
} // end ls namespace
