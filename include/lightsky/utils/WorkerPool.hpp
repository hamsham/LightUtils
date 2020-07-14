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

#include "lightsky/utils/SpinLock.hpp"
#include "lightsky/utils/Pointer.h"



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

    std::atomic_size_t mActiveThreads;

    int mCurrentBuffer;

    mutable ls::utils::SpinLock mPushLock;

    std::vector<std::vector<WorkerTaskType>> mTasks[2];

    mutable std::mutex mWaitMtx;

    mutable std::condition_variable mWaitCond;

    std::condition_variable mExecCond;

    std::vector<std::thread> mThreads;

    void execute_tasks(size_t id, int bufferId) noexcept;

    void thread_loop() noexcept;

  public:
    ~WorkerPool() noexcept;

    WorkerPool(size_t numThreads = 1) noexcept;

    WorkerPool(const WorkerPool&) noexcept;

    WorkerPool(WorkerPool&&) noexcept;

    WorkerPool& operator=(const WorkerPool&) noexcept;

    WorkerPool& operator=(WorkerPool&&) noexcept;

    const std::vector<WorkerTaskType>& tasks(size_t threadId) const noexcept;

    std::vector<WorkerTaskType>& tasks(size_t threadId) noexcept;

    std::size_t num_pending(size_t threadId) const noexcept;

    bool have_pending(size_t threadId) const noexcept;

    void clear_pending(size_t threadId) noexcept;

    void push(const WorkerTaskType& task, size_t threadIndex) noexcept;

    void emplace(WorkerTaskType&& task, size_t threadIndex) noexcept;

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
LS_DECLARE_CLASS_TYPE(DefaultWorkerPool, WorkerPool, void (*)());



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/WorkerPoolImpl.hpp"

#endif /* LS_UTILS_WORKER_POOL_HPP */
