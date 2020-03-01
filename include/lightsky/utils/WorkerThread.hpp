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
 * @brief A Worker object represents a queue which can be run either on the
 * current thread or tasks which are executed on another thread.
-----------------------------------------------------------------------------*/
template <class WorkerTaskType>
class Worker
{
  public:
    typedef WorkerTaskType value_type;

  protected:
    std::atomic_bool mBusyWait;

    std::atomic_bool mIsPaused;

    int mCurrentBuffer;

    mutable ls::utils::SpinLock mPushLock;

    std::vector<WorkerTaskType> mTasks[2];

    mutable std::mutex mWaitMtx;

    mutable std::condition_variable mWaitCond;

    virtual void execute_tasks() noexcept;

  public:
    virtual ~Worker() noexcept;

    Worker() noexcept;

    Worker(const Worker&) noexcept;

    Worker(Worker&&) noexcept;

    Worker& operator=(const Worker&) noexcept;

    Worker& operator=(Worker&&) noexcept;

    void push(const WorkerTaskType& task) noexcept;

    void emplace(WorkerTaskType&& task) noexcept;

    virtual bool ready() const noexcept;

    const std::vector<WorkerTaskType>& tasks() const noexcept;

    std::vector<WorkerTaskType>& tasks() noexcept;

    std::size_t num_pending() const noexcept;

    bool have_pending() const noexcept;

    void clear_pending() noexcept;

    virtual void flush() noexcept;

    void wait() const noexcept;

    bool busy_waiting() const noexcept;

    void busy_waiting(bool useBusyWait) noexcept;

    virtual size_t concurrency(size_t inNumThreads) noexcept;

    virtual size_t concurrency() const noexcept;
};


/*-------------------------------------
 * Convenience Types
-------------------------------------*/
LS_DECLARE_CLASS_TYPE(DefaultWorker, Worker, void (*)());



/**----------------------------------------------------------------------------
 * @brief WorkerThread is an asynchronous thread which can have tasks pushed
 * to it for execution.
-----------------------------------------------------------------------------*/
template <class WorkerTaskType>
class WorkerThread : public Worker<WorkerTaskType>
{
  private:
    std::condition_variable mExecCond;

    std::thread mThread;

    void thread_loop() noexcept;

  public:
    virtual ~WorkerThread() noexcept override;

    WorkerThread() noexcept;

    WorkerThread(const WorkerThread&) noexcept;

    WorkerThread(WorkerThread&&) noexcept;

    WorkerThread& operator=(const WorkerThread&) noexcept;

    WorkerThread& operator=(WorkerThread&&) noexcept;

    virtual void flush() noexcept override;
};


/*-------------------------------------
 * Convenience Types
-------------------------------------*/
LS_DECLARE_CLASS_TYPE(DefaultWorkerThread, WorkerThread, void (*)());



/**----------------------------------------------------------------------------
 * @brief WorkerThreadGroup peforms like the WorkerThread class, but will run
 * each task on several threads in parallel.
-----------------------------------------------------------------------------*/
template <class WorkerTaskType>
class WorkerThreadGroup : public Worker<WorkerTaskType>
{
  private:
    long long mNumThreads;

    std::atomic_llong mThreadsRunning;

    std::condition_variable mExecCond;

    ls::utils::Pointer<std::thread[]> mThreads;

    virtual void execute_tasks() noexcept override;

    void thread_loop() noexcept;

    void stop_thread_loop() noexcept;

  public:
    virtual ~WorkerThreadGroup() noexcept override;

    WorkerThreadGroup() noexcept;

    WorkerThreadGroup(const WorkerThreadGroup&) noexcept;

    WorkerThreadGroup(WorkerThreadGroup&&) noexcept;

    WorkerThreadGroup& operator=(const WorkerThreadGroup&) noexcept;

    WorkerThreadGroup& operator=(WorkerThreadGroup&&) noexcept;

    virtual void flush() noexcept override;

    virtual size_t concurrency(size_t inNumThreads) noexcept override;

    virtual size_t concurrency() const noexcept override;
};



/*-------------------------------------
 * Convenience Types
-------------------------------------*/
LS_DECLARE_CLASS_TYPE(DefaultWorkerThreadGroup, WorkerThreadGroup, void (*)());



} // end utils namespace
} // end ls namespace



#include "lightsky/utils/generic/WorkerThreadImpl.hpp"

#endif /* LS_UTILS_WORKER_THREAD_HPP */
