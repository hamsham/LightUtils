
#ifndef LS_UTILS_WORKER_POOL_IMPL_HPP
#define LS_UTILS_WORKER_POOL_IMPL_HPP

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Thread Pool Object
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Execute the tasks in the queue.
-------------------------------------*/
template <class WorkerTaskType>
void WorkerPool<WorkerTaskType>::execute_tasks() noexcept
{
    mThreadsRunning.fetch_add(1, std::memory_order_acq_rel);

    // only run while there are tasks.
    while (true)
    {
        mPushLock.lock();
        if (mTasks.empty())
        {
            mPushLock.unlock();
            break;
        }

        WorkerTaskType&& task = mTasks.pop_unchecked();
        mPushLock.unlock();

        task();
    }

    // Pause the current thread again.
    if (mThreadsRunning.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        std::lock_guard <std::mutex> waitLock{mWaitMtx};
        std::lock_guard<utils::SpinLock> pushLock{mPushLock};

        if (mTasks.empty())
        {
            mIsPaused.store(true, std::memory_order_release);
            mWaitCond.notify_all();
        }
    }
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerPool<WorkerTaskType>::thread_loop() noexcept
{
    while (true)
    {
        bool amDone;
        bool outOfTasks;

        {
            std::lock_guard<utils::SpinLock> pushLock{mPushLock};
            amDone = mTasks.capacity() == 0;
            outOfTasks = mTasks.empty();
        }

        if (amDone)
        {
            break;
        }

        if (outOfTasks || mIsPaused.load(std::memory_order_acquire))
        {
            // Busy waiting can be disabled at any time, but waiting on the
            // condition variable will remain in-place until the next flush.
            if (!mBusyWait.load(std::memory_order_acquire))
            {
                std::unique_lock<std::mutex> cvLock{mExecMtx};
                mExecCond.wait(cvLock);
            }
        }
        else
        {
            execute_tasks();
        }
    }
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <class WorkerTaskType>
void WorkerPool<WorkerTaskType>::stop_threads() noexcept
{
    while (!ready())
    {
    }

    {
        std::lock_guard<std::mutex> waitLock{mExecMtx};
        {
            std::lock_guard<utils::SpinLock> pushLock{mPushLock};
            mTasks.clear();
            mTasks.shrink_to_fit();
        }

        mIsPaused.store(false, std::memory_order_release);
        mExecCond.notify_all();
    }

    for (std::thread& t : mThreads)
    {
        t.join();
    }

    mThreads.clear();
    mIsPaused.store(true, std::memory_order_release);
    mThreadsRunning.store(0, std::memory_order_release);
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::~WorkerPool() noexcept
{
    stop_threads();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::WorkerPool(size_t inNumThreads) :
    mBusyWait{false},
    mIsPaused{true},
    mThreadsRunning{0},
    mPushLock{},
    mTasks{2},
    mWaitMtx{},
    mWaitCond{},
    mExecMtx{},
    mExecCond{},
    mThreads{}
{
    mThreads.reserve(inNumThreads);
    for (std::size_t threadId = 0; threadId < inNumThreads; ++threadId)
    {
        mThreads.emplace_back(&WorkerPool::thread_loop, this);
    }
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::WorkerPool(const WorkerPool& w) noexcept :
    WorkerPool{} // delegate constructor
{
    *this = w;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::WorkerPool(WorkerPool&& w) noexcept :
    WorkerPool{} // delegate constructor
{
    *this = std::move(w);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>& WorkerPool<WorkerTaskType>::operator=(const WorkerPool& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    w.wait();
    wait();

    stop_threads();

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);
    mTasks = w.mTasks;

    std::size_t numThreads = w.mThreads.size();
    mThreads.reserve(numThreads);

    for (std::size_t threadId = 0; threadId < numThreads; ++threadId)
    {
        mThreads.emplace_back(&WorkerPool::thread_loop, this);
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>& WorkerPool<WorkerTaskType>::operator=(WorkerPool&& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    w.wait();
    wait();

    std::size_t numThreads = w.mThreads.size();
    stop_threads();

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);
    w.mBusyWait.store(false, std::memory_order_release);

    mTasks = std::move(w.mTasks);
    mThreads.reserve(numThreads);

    for (std::size_t threadId = 0; threadId < numThreads; ++threadId)
    {
        mThreads.emplace_back(&WorkerPool::thread_loop, this);
    }

    return *this;
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline const ls::utils::RingBuffer<WorkerTaskType>& WorkerPool<WorkerTaskType>::tasks() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks;
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline ls::utils::RingBuffer<WorkerTaskType>& WorkerPool<WorkerTaskType>::tasks() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks;
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline std::size_t WorkerPool<WorkerTaskType>::num_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks.size();
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerPool<WorkerTaskType>::have_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return !mTasks.empty();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::clear_pending() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks.clear();
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::push(const WorkerTaskType& task) noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks.push(task);
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::emplace(WorkerTaskType&& task) noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks.emplace(std::forward<WorkerTaskType>(task));
}



/*-------------------------------------
 * Check if the thread has finished all tasks and can be flushed.
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerPool<WorkerTaskType>::ready() const noexcept
{
    return mIsPaused.load(std::memory_order_acquire);
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerPool<WorkerTaskType>::flush() noexcept
{
    bool haveTasks;

    {
        std::lock_guard<utils::SpinLock> pushLock{mPushLock};
        haveTasks = !mTasks.empty();
    }

    // Don't bother waking up the thread if there's nothing to do.
    if (haveTasks)
    {
        std::lock_guard<std::mutex> waitLock{mExecMtx};
        mIsPaused.store(false, std::memory_order_release);
        mExecCond.notify_all();
    }
}



/*-------------------------------------
 * Wait for the current thread to finish execution.
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::wait() const noexcept
{
    // Own the worker's mutex until the intended thread pauses. This should
    // effectively block the current thread of execution.
    if (mBusyWait.load(std::memory_order_consume))
    {
        while (!mIsPaused.load(std::memory_order_consume))
        {
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #endif
        }
    }
    else
    {
        std::unique_lock<std::mutex> cvLock{mWaitMtx};
        mWaitCond.wait(cvLock, [this]()->bool
        {
            return mIsPaused.load(std::memory_order_acquire);
        });
    }
}



/*-------------------------------------
 * Determine if busy waiting is enabled
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerPool<WorkerTaskType>::busy_waiting() const noexcept
{
    return mBusyWait.load(std::memory_order_acquire);
}



/*-------------------------------------
 * Toggle busy waiting
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::busy_waiting(bool useBusyWait) noexcept
{
    mBusyWait.store(useBusyWait, std::memory_order_release);
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
size_t WorkerPool<WorkerTaskType>::concurrency(size_t inNumThreads) noexcept
{
    if (inNumThreads == mThreads.size())
    {
        return inNumThreads;
    }

    wait();
    stop_threads();

    if (!inNumThreads)
    {
        return 0;
    }

    mTasks.reserve(2);
    mThreads.reserve(inNumThreads);

    for (std::size_t threadId = 0; threadId < inNumThreads; ++threadId)
    {
        mThreads.emplace_back(&WorkerPool::thread_loop, this);
    }

    return inNumThreads;
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t WorkerPool<WorkerTaskType>::concurrency() const noexcept
{
    return mThreads.size();
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
const std::vector<std::thread>& WorkerPool<WorkerTaskType>::threads() const noexcept
{
    return mThreads;
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
std::vector<std::thread>& WorkerPool<WorkerTaskType>::threads() noexcept
{
    return mThreads;
}


} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_WORKER_POOL_IMPL_HPP */
