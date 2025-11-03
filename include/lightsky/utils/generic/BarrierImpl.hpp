/*
 * File:   BarrierImpl.hpp
 * Author: miles
 * Created on November 02, 2025, at 1:49 p.m.
 */

#ifndef LS_UTILS_BARRIER_IMPL_HPP
#define LS_UTILS_BARRIER_IMPL_HPP

#include <thread>

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Common Barrier
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Current Thread Count
-------------------------------------*/
inline uint32_t Barrier::num_waiting_threads() const noexcept
{
    return mBarrierCount.load(std::memory_order_acquire);
}



/*-------------------------------------
 * Required Thread Count
-------------------------------------*/
inline uint32_t Barrier::num_required_threads() const noexcept
{
    return mThreadCount;
}



/*-------------------------------------
 * Native Handle (const)
-------------------------------------*/
inline const Barrier::native_handle_type& Barrier::native_handle() const noexcept
{
    return mBarrierCount;
}



/*-----------------------------------------------------------------------------
 * PThreads R/W Semaphore
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_PTHREAD_BARRIER

/*-------------------------------------
 * Current Thread Count
-------------------------------------*/
inline uint32_t SystemBarrierPThread::num_waiting_threads() const noexcept
{
    return mBarrierCount.load(std::memory_order_acquire);
}



/*-------------------------------------
 * Required Thread Count
-------------------------------------*/
inline uint32_t SystemBarrierPThread::num_required_threads() const noexcept
{
    return mThreadCount;
}



/*-------------------------------------
 * Native Handle (const)
-------------------------------------*/
inline const SystemBarrierPThread::native_handle_type& SystemBarrierPThread::native_handle() const noexcept
{
    return mBarrier;
}

#endif



/*-----------------------------------------------------------------------------
 * Windows R/W Semaphore
-----------------------------------------------------------------------------*/
#if LS_UTILS_USE_WINDOWS_BARRIER

/*-------------------------------------
 * Current Thread Count
-------------------------------------*/
inline uint32_t SystemBarrierWindows::num_waiting_threads() const noexcept
{
    return mBarrierCount.load(std::memory_order_acquire);
}



/*-------------------------------------
 * Required Thread Count
-------------------------------------*/
inline uint32_t SystemBarrierWindows::num_required_threads() const noexcept
{
    return mThreadCount;
}



/*-------------------------------------
 * Native Handle (const)
-------------------------------------*/
inline const SystemBarrierWindows::native_handle_type& SystemBarrierWindows::native_handle() const noexcept
{
    return mBarrier;
}

#endif /* LS_OS_WINDOWS */



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_BARRIER_IMPL_HPP */
