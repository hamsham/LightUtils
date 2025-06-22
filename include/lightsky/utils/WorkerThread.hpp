/*
 * File:   WorkerThread
 * Author: Miles Lacey
 *
 * Created on March 30, 2018, 4:42 pM
 */

#ifndef LS_UTILS_WORKER_THREAD_HPP
#define LS_UTILS_WORKER_THREAD_HPP

#include <condition_variable>
#include <mutex> // std::mutex, std::lock_guard
#include <thread>
#include <utility> // std::move

#include "lightsky/setup/Arch.h" // LS_ARCH_X86
#include "lightsky/setup/Macros.h" // LS_DECLARE_CLASS_TYPE()
#include "lightsky/setup/OS.h" // LS_OS_WINDOWS

#ifdef LS_OS_WINDOWS
    #ifndef LS_UTILS_USE_WINDOWS_THREADS
        #define LS_UTILS_USE_WINDOWS_THREADS
    #endif
#endif /* LS_OS_WINDOWS */

#ifdef LS_UTILS_USE_WINDOWS_THREADS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif  /* WIN32_LEAN_AND_MEAN */

    // Typycal windows bullshit
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif /* NOMINMAX */

    #include <windows.h>
    #include <synchapi.h>
#endif /* LS_UTILS_USE_WINDOWS_THREADS */

#include "lightsky/utils/RWLock.hpp"
#include "lightsky/utils/RingBuffer.hpp"



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * CPU Utilities
-----------------------------------------------------------------------------*/
size_t get_thread_id() noexcept;

bool set_thread_affinity(size_t threadId, unsigned affinity) noexcept;

bool set_thread_affinity(std::thread& t, unsigned affinity) noexcept;



/**----------------------------------------------------------------------------
 * @brief WorkerThread is an asynchronous thread which can have tasks pushed
 * to it for execution.
-----------------------------------------------------------------------------*/
template <class WorkerTaskType>
class WorkerThread
{
  public:
    typedef WorkerTaskType value_type;

  private:
    std::atomic_bool mBusyWait;

    std::atomic_bool mIsPaused;

    mutable utils::RWLock mPushLock;

    utils::RingBuffer<WorkerTaskType> mTasks;

#ifndef LS_UTILS_USE_WINDOWS_THREADS
    mutable std::mutex mWaitMtx;

    mutable std::condition_variable mWaitCond;

    std::mutex mExecMtx;

    std::condition_variable mExecCond;
#else
    mutable CRITICAL_SECTION mWaitMtx;

    mutable CONDITION_VARIABLE mWaitCond;

    CRITICAL_SECTION mExecMtx;

    CONDITION_VARIABLE mExecCond;
#endif

    std::thread mThread;

    void execute_tasks() noexcept;

    void thread_loop() noexcept;

  public:
    ~WorkerThread() noexcept;

    WorkerThread(unsigned affinity = ~0u) noexcept;

    WorkerThread(const WorkerThread&) noexcept;

    WorkerThread(WorkerThread&&) noexcept;

    WorkerThread& operator=(const WorkerThread&) noexcept;

    WorkerThread& operator=(WorkerThread&&) noexcept;

    const utils::RingBuffer<WorkerTaskType>& tasks() const noexcept;

    utils::RingBuffer<WorkerTaskType>& tasks() noexcept;

    std::size_t num_pending() const noexcept;

    bool have_pending() const noexcept;

    void clear_pending() noexcept;

    void push(const WorkerTaskType& task) noexcept;

    void emplace(WorkerTaskType&& task) noexcept;

    bool ready() const noexcept;

    void flush() noexcept;

    void wait() const noexcept;

    bool busy_waiting() const noexcept;

    void busy_waiting(bool useBusyWait) noexcept;

    size_t concurrency(size_t inNumThreads) noexcept;

    size_t concurrency() const noexcept;
};


/*-------------------------------------
 * Convenience Types
-------------------------------------*/
LS_DECLARE_CLASS_TYPE(DefaultWorkerThread, WorkerThread, void (*)());



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/WorkerThreadImpl.hpp"

#endif /* LS_UTILS_WORKER_THREAD_HPP */
