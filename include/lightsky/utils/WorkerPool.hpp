/*
 * File:   WorkerPool
 * Author: Miles Lacey
 *
 * Created on July 13, 2020, 9:42 pM
 */

#ifndef LS_UTILS_WORKER_POOL_HPP
#define LS_UTILS_WORKER_POOL_HPP

#include <condition_variable>
#include <mutex> // std::mutex, std::lock_guard
#include <thread>
#include <utility> // std::move
#include <vector>

#include "lightsky/setup/Arch.h" // LS_ARCH_X86
#include "lightsky/setup/Macros.h" // LS_DECLARE_CLASS_TYPE()
#include "lightsky/setup/OS.h" // LS_OS_WINDOWS

#include "lightsky/utils/SpinLock.hpp"
#include "lightsky/utils/RingBuffer.hpp"



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief WorkerPool is an asynchronous thread which can have tasks pushed
 * to it for execution.
-----------------------------------------------------------------------------*/
template <class WorkerTaskType>
class WorkerPool
{
  public:
    typedef WorkerTaskType value_type;

  private:
    std::atomic_bool mBusyWait;

    std::atomic_bool mIsPaused;

    std::atomic<size_t> mThreadsRunning;

    mutable utils::SpinLock mPushLock;

    utils::RingBuffer<WorkerTaskType> mTasks;

    mutable std::mutex mWaitMtx;

    mutable std::condition_variable mWaitCond;

    std::condition_variable mExecCond;

    std::vector<std::thread> mThreads;

    void execute_tasks() noexcept;

    void thread_loop() noexcept;

    void stop_threads() noexcept;

  public:
    ~WorkerPool() noexcept;

    WorkerPool(size_t numThreads = 1);

    WorkerPool(const WorkerPool&) noexcept;

    WorkerPool(WorkerPool&&) noexcept;

    WorkerPool& operator=(const WorkerPool&) noexcept;

    WorkerPool& operator=(WorkerPool&&) noexcept;

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

    const std::vector<std::thread>& threads() const noexcept;

    std::vector<std::thread>& threads() noexcept;
};


/*-------------------------------------
 * Convenience Types
-------------------------------------*/
LS_DECLARE_CLASS_TYPE(DefaultWorkerPool, WorkerPool, void (*)());



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/WorkerPoolImpl.hpp"

#endif /* LS_UTILS_WORKER_POOL_HPP */
