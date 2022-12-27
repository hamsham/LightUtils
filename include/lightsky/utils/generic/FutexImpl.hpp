
#ifndef LS_UTILS_FUTEX_IMPL_HPP
#define LS_UTILS_FUTEX_IMPL_HPP

#include <limits>

#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    extern "C"
    {
        #include <linux/futex.h>
        #include <sys/syscall.h>
        #include <sys/time.h>
        #include <unistd.h>
    }
#endif

#include "lightsky/setup/CPU.h"



namespace ls
{
namespace utils
{



/*-------------------------------------
 * Futex Lock
-------------------------------------*/
inline void Futex::lock() noexcept
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
            const int32_t status = syscall(SYS_futex, &mLock, FUTEX_WAIT_BITSET_PRIVATE, 1, currentPauses, nullptr, FUTEX_BITSET_MATCH_ANY);

            if (status != 0)
            {
                currentPauses <<= 1;
            }
        }
        else
        {
            syscall(SYS_futex, &mLock, FUTEX_WAIT_PRIVATE, 1, nullptr);
        }
    }
    while (true);

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
inline bool Futex::try_lock() noexcept
{
#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    int32_t tmp = 0;
    return __atomic_compare_exchange_n(&mLock, &tmp, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

#else
    int32_t tmp = 0;
    return mLock.compare_exchange_strong(tmp, 1, std::memory_order_seq_cst, std::memory_order_relaxed);

#endif
}



/*-------------------------------------
 * Set the maximum consecutive pauses before retrying a lock
-------------------------------------*/
inline void Futex::pause_count(FutexPauseCount maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline FutexPauseCount Futex::pause_count() const noexcept
{
    return mMaxPauseCount;
}



/*-------------------------------------
 * Futex unlock
-------------------------------------*/
inline void Futex::unlock() noexcept
{
#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    __atomic_store_n(&mLock, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &mLock, FUTEX_WAKE_PRIVATE, 1);

#else
    mLock.store(0, std::memory_order_release);

#endif
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FUTEX_IMPL_HPP */