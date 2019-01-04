/*
 * File:   WorkerThread
 * Author: Miles Lacey
 *
 * Created on March 30, 2018, 4:42 pM
 */

#ifndef LS_UTILS_WORKER_THREAD_HPP
#define LS_UTILS_WORKER_THREAD_HPP

#include <atomic> // std::atomic_flag
#include <condition_variable>
#include <mutex> // std::mutex, std::lock_guard
#include <thread>
#include <utility> // std::move
#include <vector>

#include "lightsky/setup/Arch.h" // LS_ARCH_X86
#include "lightsky/setup/Macros.h" // LS_DECLARE_CLASS_TYPE()



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief A SpinLock attempts to mimic a mutex object but will allow the CPU
 * to continue running while it is locked.
-----------------------------------------------------------------------------*/
class SpinLock
{
  private:
    std::atomic_flag mLock;

  public:
    ~SpinLock() noexcept;

    SpinLock() noexcept;

    SpinLock(const SpinLock&) = delete;

    SpinLock(SpinLock&&) = delete;

    SpinLock& operator=(const SpinLock&) = delete;

    SpinLock& operator=(SpinLock&&) = delete;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};



/*-------------------------------------
 * Mutex Lock
-------------------------------------*/
inline void SpinLock::lock() noexcept
{
    while (mLock.test_and_set(std::memory_order_consume))
    {
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool SpinLock::try_lock() noexcept
{
    return !mLock.test_and_set(std::memory_order_consume);
}



/*-------------------------------------
 * Mutex unlock
-------------------------------------*/
inline void SpinLock::unlock() noexcept
{
    mLock.clear(std::memory_order_release);
}



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

    std::vector<WorkerTaskType> mInputs[2];

    std::vector<WorkerTaskType> mOutputs[2];

    mutable std::mutex mMutex;

    void execute_tasks() noexcept;

  public:
    virtual ~Worker() noexcept;

    Worker() noexcept;

    Worker(const Worker&) noexcept;

    Worker(Worker&&) noexcept;

    Worker& operator=(const Worker&) noexcept;

    Worker& operator=(Worker&&) noexcept;

    void push(const WorkerTaskType& task) noexcept;

    void emplace(WorkerTaskType&& task) noexcept;

    void push_active(const WorkerTaskType& task) noexcept;

    void emplace_active(WorkerTaskType&& task) noexcept;

    void push_output(const WorkerTaskType& task) noexcept;

    void emplace_output(WorkerTaskType&& task) noexcept;

    bool ready() const noexcept;

    const std::vector<WorkerTaskType>& inputs() const noexcept;

    std::vector<WorkerTaskType>& inputs() noexcept;

    const std::vector<WorkerTaskType>& active_inputs() const noexcept;

    std::vector<WorkerTaskType>& active_inputs() noexcept;

    const std::vector<WorkerTaskType>& outputs() const noexcept;

    std::vector<WorkerTaskType>& outputs() noexcept;

    const std::vector<WorkerTaskType>& active_outputs() const noexcept;

    std::vector<WorkerTaskType>& active_outputs() noexcept;

    std::size_t num_pending_tasks() const noexcept;

    bool have_pending_tasks() const noexcept;

    void clear_pending_tasks() noexcept;

    virtual void flush() noexcept;

    void wait() noexcept;

    bool busy_wait_enabled() const noexcept;

    void busy_waiting(bool useBusyWait) noexcept;
};


/*-------------------------------------
 * Convenience Types
-------------------------------------*/
LS_DECLARE_CLASS_TYPE(DefaultWorker, Worker, void (*)());



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <class WorkerTaskType>
Worker<WorkerTaskType>::~Worker() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
Worker<WorkerTaskType>::Worker() noexcept :
    mBusyWait{false},
    mIsPaused{true},
    mCurrentBuffer{0}, // must be greater than or equal to 0 for *this to run.
    mPushLock{},
    mInputs{},
    mOutputs{},
    mMutex{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <class WorkerTaskType>
Worker<WorkerTaskType>::Worker(const Worker& w) noexcept :
    Worker{} // delegate constructor
{
    *this = w;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <class WorkerTaskType>
Worker<WorkerTaskType>::Worker(Worker&& w) noexcept :
    Worker{} // delegate constructor
{
    *this = std::move(w);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <class WorkerTaskType>
Worker<WorkerTaskType>& Worker<WorkerTaskType>::operator=(const Worker& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    while (!w.ready())
    {
        // busy wait
    }

    while (!this->ready())
    {
        // busy wait
    }

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);

    mIsPaused.store(true, std::memory_order_release);

    mCurrentBuffer = w.mCurrentBuffer;

    mInputs[0] = w.mInputs[0];
    mInputs[1] = w.mInputs[1];

    mOutputs[0] = w.mOutputs[0];
    mOutputs[1] = w.mOutputs[1];

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <class WorkerTaskType>
Worker<WorkerTaskType>& Worker<WorkerTaskType>::operator=(Worker&& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    while (!w.ready())
    {
        // busy wait
    }

    while (!this->ready())
    {
        // busy wait
    }

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);
    w.mBusyWait.store(false, std::memory_order_release);

    mIsPaused.store(true, std::memory_order_release);

    mCurrentBuffer = w.mCurrentBuffer;
    w.mCurrentBuffer = -1;

    mInputs[0] = std::move(w.mInputs[0]);
    mInputs[1] = std::move(w.mInputs[1]);

    mOutputs[0] = std::move(w.mOutputs[0]);
    mOutputs[1] = std::move(w.mOutputs[1]);

    return *this;
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::execute_tasks() noexcept
{
    // The current I/O buffers were swapped when "flush()" was
    // called. Swap again to read and write to the buffers used on
    // this thread.
    mPushLock.lock();

    // Exit the thread if mCurrentBuffer is less than 0.
    if (mCurrentBuffer < 0)
    {
        mPushLock.unlock();
        mMutex.unlock();
        return;
    }

    int currentBuffer = (mCurrentBuffer + 1) % 2;
    std::vector<WorkerTaskType>& inputQueue = mInputs[currentBuffer];
    //std::vector<WorkerTaskType>& outputQueue = mOutputs[currentBuffer];

    mPushLock.unlock();
    mMutex.lock();

    for (WorkerTaskType& pTask : inputQueue)
    {
        pTask();
    }

    inputQueue.clear();

    // Pause the current thread again.
    mIsPaused.store(true, std::memory_order_release);
    mMutex.unlock();
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::push(const WorkerTaskType& task) noexcept
{
    mPushLock.lock();
    mInputs[mCurrentBuffer].push_back(task);
    mPushLock.unlock();
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::emplace(WorkerTaskType&& task) noexcept
{
    mPushLock.lock();
    mInputs[mCurrentBuffer].emplace_back(std::move(task));
    mPushLock.unlock();
}



/*-------------------------------------
 * Push a task to the active task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::push_active(const WorkerTaskType& task) noexcept
{
    mInputs[(mCurrentBuffer+1)%2].push_back(task);
}



/*-------------------------------------
 * Push a task to the active task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::emplace_active(WorkerTaskType&& task) noexcept
{
    mInputs[(mCurrentBuffer+1)%2].emplace_back(std::move(task));
}



/*-------------------------------------
 * Push a task to the output task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::push_output(const WorkerTaskType& task) noexcept
{
    mPushLock.lock();
    const int currentBuffer = this->mCurrentBuffer;
    const int nextBuffer    = (currentBuffer + 1) % 2;
    mOutputs[nextBuffer].push_back(task);
    mPushLock.unlock();
}



/*-------------------------------------
 * Push a task to the output task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::emplace_output(WorkerTaskType&& task) noexcept
{
    mPushLock.lock();
    const int currentBuffer = this->mCurrentBuffer;
    const int nextBuffer    = (currentBuffer + 1) % 2;
    mOutputs[nextBuffer].emplace_back(std::move(task));
    mPushLock.unlock();
}



/*-------------------------------------
 * Check if the thread has finished all tasks and can be flushed.
-------------------------------------*/
template <class WorkerTaskType>
inline bool Worker<WorkerTaskType>::ready() const noexcept
{
    return mIsPaused.load(std::memory_order_consume);
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline const std::vector<WorkerTaskType>& Worker<WorkerTaskType>::inputs() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mInputs[mCurrentBuffer];
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& Worker<WorkerTaskType>::inputs() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mInputs[mCurrentBuffer];
}



/*-------------------------------------
 * Retrieve the current set of tasks actively being worked on.
-------------------------------------*/
template <class WorkerTaskType>
inline const std::vector<WorkerTaskType>& Worker<WorkerTaskType>::active_inputs() const noexcept
{
    const int currentBuffer = mCurrentBuffer;
    const int nextBuffer = (currentBuffer + 1) % 2;
    return mInputs[nextBuffer];
}



/*-------------------------------------
 * Retrieve the current set of tasks actively being worked on.
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& Worker<WorkerTaskType>::active_inputs() noexcept
{
    const int currentBuffer = mCurrentBuffer;
    const int nextBuffer = (currentBuffer + 1) % 2;
    return mInputs[nextBuffer];
}



/*-------------------------------------
 * Retrieve the last set of tasks performed asynchronously (should be called
 * after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline const std::vector<WorkerTaskType>& Worker<WorkerTaskType>::outputs() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mOutputs[mCurrentBuffer];
}



/*-------------------------------------
 * Retrieve the last set of tasks performed asynchronously (should be called
 * after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& Worker<WorkerTaskType>::outputs() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mOutputs[mCurrentBuffer];
}



/*-------------------------------------
 * Retrieve the last set of tasks performed asynchronously (should be called
 * only if this thread is ready for execution).
-------------------------------------*/
template <class WorkerTaskType>
inline const std::vector<WorkerTaskType>& Worker<WorkerTaskType>::active_outputs() const noexcept
{
    const int currentBuffer = mCurrentBuffer;
    const int nextBuffer = (currentBuffer + 1) % 2;
    return mOutputs[nextBuffer];
}



/*-------------------------------------
 * Retrieve the last set of tasks performed asynchronously (should be called
 * only if this thread is ready for execution).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& Worker<WorkerTaskType>::active_outputs() noexcept
{
    const int currentBuffer = mCurrentBuffer;
    const int nextBuffer = (currentBuffer + 1) % 2;
    return mOutputs[nextBuffer];
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline std::size_t Worker<WorkerTaskType>::num_pending_tasks() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mInputs[mCurrentBuffer].size();
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline bool Worker<WorkerTaskType>::have_pending_tasks() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return !mInputs[mCurrentBuffer].empty();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::clear_pending_tasks() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mInputs[mCurrentBuffer].clear();
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::flush() noexcept
{
    mPushLock.lock();
    const int currentBuffer = mCurrentBuffer;
    const int nextBuffer = (currentBuffer + 1) % 2;

    mCurrentBuffer = nextBuffer;
    const bool unpause = !this->mInputs[currentBuffer].empty();
    mPushLock.unlock();

    if (unpause)
    {
        mIsPaused.store(unpause, std::memory_order_release);
        this->execute_tasks();
    }
}



/*-------------------------------------
 * Wait for the current thread to finish execution.
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::wait() noexcept
{
    // Own the worker's mutex until the intended thread pauses. This should
    // effectively block the current thread of execution.
    /*
    while (!mIsPaused.load(std::memory_order_consume))
    {
        if (!mBusyWait.load(std::memory_order_consume))
        {
            mMutex.lock();
            mMutex.unlock();
        }
    }
    */

    /*
    if (mBusyWait.load(std::memory_order_acquire))
    {
        while (!mIsPaused.load(std::memory_order_consume))
        {
        }
    }

    while (!mIsPaused.load(std::memory_order_consume))
    {
        mMutex.lock();
        mMutex.unlock();
    }
    */

    while (!mIsPaused.load(std::memory_order_consume))
    {
        if (mBusyWait.load(std::memory_order_consume))
        {
            continue;
        }
        else
        {
            mMutex.lock();
            mMutex.unlock();
        }
    }
}



/*-------------------------------------
 * Determine if busy waiting is enabled
-------------------------------------*/
template <class WorkerTaskType>
inline bool Worker<WorkerTaskType>::busy_wait_enabled() const noexcept
{
    return mBusyWait.load(std::memory_order_consume);
}



/*-------------------------------------
 * Toggle busy waiting
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::busy_waiting(bool useBusyWait) noexcept
{
    mBusyWait.store(useBusyWait, std::memory_order_release);
}



/**----------------------------------------------------------------------------
 * @brief WorkerThread is an asynchronous thread which can have tasks pushed
 * to it for execution.
-----------------------------------------------------------------------------*/
template <class WorkerTaskType>
class WorkerThread : public Worker<WorkerTaskType>
{
  private:
    std::condition_variable mCondition;

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



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::~WorkerThread() noexcept
{
    while (!this->ready())
    {
    }

    this->mPushLock.lock();
    this->mMutex.lock();
    this->mCurrentBuffer = -1;
    this->mIsPaused.store(false, std::memory_order_release);
    this->mMutex.unlock();
    this->mPushLock.unlock();
    this->mCondition.notify_all();
    this->mThread.join();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::WorkerThread() noexcept :
    Worker<WorkerTaskType>{},
    mCondition{},
    mThread{&WorkerThread::thread_loop, this}
{
    // let the thread's loop run until it hits the condition variable
    std::this_thread::yield();
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::WorkerThread(const WorkerThread& w) noexcept :
    WorkerThread{} // delegate constructor
{
    *this = w;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::WorkerThread(WorkerThread&& w) noexcept :
    WorkerThread{} // delegate constructor
{
    *this = std::move(w);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>& WorkerThread<WorkerTaskType>::operator=(const WorkerThread& w) noexcept
{
    Worker<WorkerTaskType>::operator=(w);

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>& WorkerThread<WorkerTaskType>::operator=(WorkerThread&& w) noexcept
{
    Worker<WorkerTaskType>::operator=(std::move(w));

    return *this;
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThread<WorkerTaskType>::thread_loop() noexcept
{
    while (this->mCurrentBuffer >= 0)
    {
        // Busy waiting can be disabled at any time, but waiting on the
        // condition variable will remain in-place until the next flush.
        if (this->mIsPaused.load(std::memory_order_acquire))
        {
            if (this->mBusyWait.load(std::memory_order_acquire))
            {
                continue;
            }
            else
            {
                std::unique_lock<std::mutex> cvLock{this->mMutex};
                this->mCondition.wait(cvLock, [this]()->bool
                {
                    return !this->mIsPaused.load(std::memory_order_acquire);
                });
            }
        }

        this->execute_tasks();
    }
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThread<WorkerTaskType>::flush() noexcept
{
    this->mPushLock.lock();
    const int currentBuffer = this->mCurrentBuffer;
    const int swapped = !this->mInputs[currentBuffer].empty();
    this->mCurrentBuffer = (currentBuffer + swapped) & 1;
    this->mPushLock.unlock();

    // Don't bother waking up the thread if there's nothing to do.
    if (swapped)
    {
        this->mMutex.lock();
        this->mIsPaused.store(false, std::memory_order_release);
        this->mCondition.notify_one();
        this->mMutex.unlock();
    }
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_WORKER_THREAD_HPP */
