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
#include <memory> // std::unique_ptr
#include <mutex> // std::mutex, std::lock_guard
#include <thread>
#include <utility> // std::move
#include <vector>

#include "lightsky/setup/Macros.h"



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
    std::atomic_bool mIsPaused;

    bool mStoreResults;

    int mCurrentBuffer;

    std::atomic_flag mPushLock;

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

    void push(WorkerTaskType&& task) noexcept;

    virtual void flush() noexcept;

    bool ready() const noexcept;

    std::vector<WorkerTaskType>& results() noexcept;

    void store_results(bool keepTaskOutputs) noexcept;

    bool stores_results() noexcept;

    std::size_t num_pending_tasks() const noexcept;

    void clear_pending_tasks() noexcept;
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
    mIsPaused{true},
    mStoreResults{true},
    mCurrentBuffer{0}, // must be greater than or equal to 0 for *this to run.
    mPushLock{false},
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

    mIsPaused.store(true, std::memory_order_relaxed);
    mStoreResults = w.mStoreResults;

    mCurrentBuffer = w.mCurrentBuffer;

    mPushLock.clear(std::memory_order_release);

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

    mIsPaused.store(true, std::memory_order_relaxed);

    mStoreResults = w.mStoreResults;
    w.mStoreResults = true;

    mCurrentBuffer = w.mCurrentBuffer;
    w.mCurrentBuffer = -1;

    mPushLock.clear(std::memory_order_release);
    w.mPushLock.clear(std::memory_order_release);

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
void Worker<WorkerTaskType>::execute_tasks() noexcept
{
    // The current I/O buffers were swapped when "flush()" was
    // called. Swap again to read and write to the buffers used on
    // this thread.
    int currentBuffer;
    std::vector<WorkerTaskType>* inputQueue;
    std::vector<WorkerTaskType>* outputQueue;

    // The current buffer is only locked only when flushing.
    {
        // Exit the thread if mCurrentBuffer is less than 0.
        if (mCurrentBuffer < 0)
        {
            return;
        }

        currentBuffer = (mCurrentBuffer + 1) % 2;
        inputQueue = mInputs + currentBuffer;
        outputQueue = mOutputs + currentBuffer;
    }

    for (WorkerTaskType& pTask : *inputQueue)
    {
        pTask();
    }

    // tasks are their own results
    if (mStoreResults)
    {
        *outputQueue = std::move(*inputQueue);
    }

    inputQueue->clear();
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::push(const WorkerTaskType& task) noexcept
{
    while (mPushLock.test_and_set(std::memory_order_consume))
    {
    }

    mInputs[mCurrentBuffer].push_back(task);

    mPushLock.clear(std::memory_order_release);
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::push(WorkerTaskType&& task) noexcept
{
    while (mPushLock.test_and_set(std::memory_order_consume))
    {
    }

    mInputs[mCurrentBuffer].push_back(task);

    mPushLock.clear(std::memory_order_release);
}



/*-------------------------------------
 * Check if the thread has finished all tasks and can be flushed.
-------------------------------------*/
template <class WorkerTaskType>
inline bool Worker<WorkerTaskType>::ready() const noexcept
{
    return mIsPaused.load(std::memory_order_relaxed);
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void Worker<WorkerTaskType>::flush() noexcept
{
    if (mIsPaused.load(std::memory_order_consume))
    {
        mMutex.lock();
        const int currentBuffer = mCurrentBuffer;
        const int nextBuffer = (currentBuffer + 1) % 2;

        mCurrentBuffer = nextBuffer;

        const bool haveTasks = !mInputs[currentBuffer].empty();
        mMutex.unlock();

        if (haveTasks)
        {
            mIsPaused.store(false, std::memory_order_release);
            execute_tasks();
        }

        mIsPaused.store(true, std::memory_order_release);
    }
}



/*-------------------------------------
 * Retrieve the last set of tasks performed asynchronously (should be called
 * after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& Worker<WorkerTaskType>::results() noexcept
{
    std::lock_guard<std::mutex> lock{mMutex};
    return mOutputs[mCurrentBuffer];
}



/*-------------------------------------
 * Determine whether the task queues store their results in an output buffer.
-------------------------------------*/
template <class WorkerTaskType>
void Worker<WorkerTaskType>::store_results(bool keepTaskOutputs) noexcept
{
    mStoreResults = keepTaskOutputs;
}



/*-------------------------------------
 * Check if task queues store their results in an output buffer
-------------------------------------*/
template <class WorkerTaskType>
bool Worker<WorkerTaskType>::stores_results() noexcept
{
    return mStoreResults;
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
std::size_t Worker<WorkerTaskType>::num_pending_tasks() const noexcept
{
    std::lock_guard<std::mutex> lock{mMutex};
    return mInputs[mCurrentBuffer].size();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
void Worker<WorkerTaskType>::clear_pending_tasks() noexcept
{
    std::lock_guard<std::mutex> lock{mMutex};
    mInputs[mCurrentBuffer].clear();
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
    this->mMutex.lock();
    this->mCurrentBuffer = -1;
    this->mIsPaused.store(false, std::memory_order_relaxed);
    this->mStoreResults = false;
    this->mMutex.unlock();

    mCondition.notify_one();
    mThread.join();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::WorkerThread() noexcept :
    Worker<WorkerTaskType>{},
    mCondition{},
    mThread{&WorkerThread::thread_loop, this}
{}



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
        // Lock the condition, wait until we're ready to execute.
        // A while loop should also protect against spurious wake-ups
        // by the condition variable.
        //while (mIsPaused)
        {
            std::unique_lock<std::mutex> cvLock{this->mMutex};
            this->mCondition.wait(cvLock, [this]
            {
                return !(this->mIsPaused.load(std::memory_order_relaxed));
            });

            //std::this_thread::yield();
        }

        Worker<WorkerTaskType>::execute_tasks();

        // Pause the current thread again.
        this->mIsPaused.store(true, std::memory_order_relaxed);
    }
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThread<WorkerTaskType>::flush() noexcept
{
    bool bufferSwapped = false;

    {
        if (this->mIsPaused.load(std::memory_order_relaxed))
        {
            std::lock_guard<std::mutex> lock{this->mMutex};

            const int currentBuffer = this->mCurrentBuffer;
            const int nextBuffer    = (currentBuffer + 1) % 2;
            this->mCurrentBuffer    = nextBuffer;

            this->mIsPaused.store(this->mInputs[currentBuffer].empty(), std::memory_order_relaxed);

            bufferSwapped = true;
        }
    }

    if (bufferSwapped)
    {
        this->mCondition.notify_one();
    }
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_WORKER_THREAD_HPP */
