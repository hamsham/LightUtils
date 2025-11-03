/*
 * File:   Barrier.cpp
 * Author: miles
 * Created on November 02, 2025, at 1:49 p.m.
 */

#include "lightsky/utils/Barrier.hpp"

#include "lightsky/setup/CPU.h"



namespace ls
{
namespace utils
{

/*-----------------------------------------------------------------------------
 * Common Barrier
-----------------------------------------------------------------------------*/
#ifndef LS_UTILS_BARRIER_CPU_YIELD
    #define LS_UTILS_BARRIER_CPU_YIELD 0
#endif



/*-------------------------------------
 * Constructor
-------------------------------------*/
Barrier::Barrier(uint32_t numThreads) noexcept :
    mBarrierCount{0},
    mThreadCount{numThreads}
{}



/*-------------------------------------
 * Wait until all required threads have arrived
-------------------------------------*/
void Barrier::wait() noexcept
{
    if (num_waiting_threads() >= mThreadCount)
    {
        return;
    }

    uint32_t numWaiters = mBarrierCount.fetch_add(1, std::memory_order_acq_rel);
    while (numWaiters < mThreadCount)
    {
        #if LS_UTILS_BARRIER_CPU_YIELD
            ls::setup::cpu_yield();
        #else
            std::this_thread::yield();
        #endif

        numWaiters = mBarrierCount.load(std::memory_order_acquire);
    }
}



/*-----------------------------------------------------------------------------
 * PThreads R/W Semaphore
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_BARRIER

/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemBarrierPThread::~SystemBarrierPThread() noexcept
{
    pthread_barrier_destroy(&mBarrier);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemBarrierPThread::SystemBarrierPThread(uint32_t numThreads) noexcept :
    mBarrier{},
    mBarrierCount{0},
    mThreadCount{numThreads}
{
    pthread_barrier_init(&mBarrier, nullptr, (unsigned)numThreads);
}



/*-------------------------------------
 * Wait until all required threads have arrived
-------------------------------------*/
void SystemBarrierPThread::wait() noexcept
{
    if (num_waiting_threads() >= mThreadCount)
    {
        return;
    }

    mBarrierCount.fetch_add(1, std::memory_order_acq_rel);

    while (true)
    {
        int result = pthread_barrier_wait(&mBarrier);
        if (result == 0 || result == PTHREAD_BARRIER_SERIAL_THREAD)
        {
            break;
        }

        #if LS_UTILS_BARRIER_CPU_YIELD
            ls::setup::cpu_yield();
        #else
            const int err = sched_yield();
            if (err != 0)
            {
                const char* errMsg = strerror(err);
                perror(errMsg);
            }
        #endif
    }
}

#endif



/*-----------------------------------------------------------------------------
 * Windows R/W Semaphore
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_BARRIER

/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemBarrierWindows::~SystemBarrierWindows() noexcept
{
    DeleteSynchronizationBarrier(&mBarrier);
}

/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemBarrierWindows::SystemBarrierWindows(uint32_t numThreads) noexcept :
    mBarrier{},
    mBarrierCount{0},
    mThreadCount{numThreads}
{
    InitializeSynchronizationBarrier(&mBarrier, (unsigned)numThreads, -1);
}



/*-------------------------------------
 * Wait until all required threads have arrived
-------------------------------------*/
void SystemBarrierWindows::wait() noexcept
{
    if (num_waiting_threads() >= mThreadCount)
    {
        return;
    }

    mBarrierCount.fetch_add(1, std::memory_order_acq_rel);
    EnterSynchronizationBarrier(&mBarrier, 0);
}

#endif

} // end utils namespace
} // end ls namespace
