
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
void WorkerPool<WorkerTaskType>::execute_tasks(size_t id, int currentBufferId) noexcept
{
    // Exit the thread if mCurrentBuffer is less than 0.
    if (currentBufferId >= 0)
    {
        currentBufferId = (currentBufferId + 1) % 2;
        std::vector<WorkerTaskType>& inputQueue = mTasks[currentBufferId][id];

        for (WorkerTaskType& pTask : inputQueue)
        {
            pTask();
        }

        mTasks[currentBufferId][id].clear();
    }
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerPool<WorkerTaskType>::thread_loop() noexcept
{
    while (mCurrentBuffer >= 0)
    {
        // Busy waiting can be disabled at any time, but waiting on the
        // condition variable will remain in-place until the next flush.
        if (mIsPaused.load(std::memory_order_acquire))
        {
            if (mBusyWait.load(std::memory_order_acquire))
            {
                continue;
            }
            else
            {
                std::unique_lock<std::mutex> cvLock{mWaitMtx};
                mExecCond.wait(cvLock, [this]()->bool
                {
                    return !mIsPaused.load(std::memory_order_acquire) || mCurrentBuffer < 0;
                });
            }
        }

        // The current I/O buffers were swapped when "flush()" was
        // called. Swap again to read and write to the buffers used on
        // this thread.
        int currentBufferId;
        mPushLock.lock();
        currentBufferId = mCurrentBuffer;
        mPushLock.unlock();

        if (currentBufferId < 0)
        {
            return;
        }

        size_t id = mActiveThreads.fetch_add(1, std::memory_order_acq_rel);
        while (mActiveThreads.load(std::memory_order_consume) < mThreads.size())
        {
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #endif
        }

        if (!id)
        {
            mWaitMtx.lock();
        }

        execute_tasks(id, currentBufferId);

        // last thread unlocks
        id = mActiveThreads.fetch_add(1, std::memory_order_acq_rel);
        if (id == mThreads.size()*2-1)
        {
            mIsPaused.store(true, std::memory_order_release);
            mActiveThreads.store(0, std::memory_order_release);
            mWaitMtx.unlock();
        }
        else
        {
            while (!mIsPaused.load(std::memory_order_consume))
            {
                #if defined(LS_ARCH_X86)
                    _mm_pause();
                #endif
            }
        }
    }
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::~WorkerPool() noexcept
{
    wait();

    mPushLock.lock();
    mCurrentBuffer = -1;
    mPushLock.unlock();

    mWaitMtx.lock();
    mBusyWait.store(true, std::memory_order_release);
    mIsPaused.store(false, std::memory_order_release);
    mExecCond.notify_all();
    mWaitMtx.unlock();

    for (std::thread& t : mThreads)
    {
        t.join();
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::WorkerPool(size_t inNumThreads) noexcept :
    mBusyWait{false},
    mIsPaused{true},
    mActiveThreads{0},
    mCurrentBuffer{0}, // must be greater than or equal to 0 for *this to run.
    mPushLock{},
    mTasks{},
    mWaitMtx{},
    mWaitCond{},
    mExecCond{},
    mThreads{}
{
    concurrency(inNumThreads);
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::WorkerPool(const WorkerPool& w) noexcept :
    mBusyWait{false},
    mIsPaused{true},
    mActiveThreads{0},
    mCurrentBuffer{0}, // must be greater than or equal to 0 for *this to run.
    mPushLock{},
    mTasks{},
    mWaitMtx{},
    mWaitCond{},
    mExecCond{},
    mThreads{}
{
    *this = w;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerPool<WorkerTaskType>::WorkerPool(WorkerPool&& w) noexcept :
    mBusyWait{false},
    mIsPaused{true},
    mActiveThreads{0},
    mCurrentBuffer{0}, // must be greater than or equal to 0 for *this to run.
    mPushLock{},
    mTasks{},
    mWaitMtx{},
    mWaitCond{},
    mExecCond{},
    mThreads{}
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

    concurrency(w.concurrency());
    mIsPaused.store(true, std::memory_order_release);

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);

    mCurrentBuffer = w.mCurrentBuffer;

    for (unsigned i = mThreads.size(); i--;)
    {
        mTasks[0][i] = w.mTasks[0][i];
        mTasks[1][i] = w.mTasks[1][i];
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

    concurrency(w.concurrency());
    mIsPaused.store(true, std::memory_order_release);

    mBusyWait.store(w.mBusyWait.load(std::memory_order_acquire), std::memory_order_release);

    mCurrentBuffer = w.mCurrentBuffer;
    w.mCurrentBuffer = -1;

    for (unsigned i = mThreads.size(); i--;)
    {
        mTasks[0][i] = w.mTasks[0][i];
        mTasks[1][i] = w.mTasks[1][i];
    }

    return *this;
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline const std::vector<WorkerTaskType>& WorkerPool<WorkerTaskType>::tasks(size_t threadId) const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer][threadId];
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& WorkerPool<WorkerTaskType>::tasks(size_t threadId) noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer][threadId];
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline std::size_t WorkerPool<WorkerTaskType>::num_pending(size_t threadId) const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer][threadId].size();
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerPool<WorkerTaskType>::have_pending(size_t threadId) const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return !mTasks[mCurrentBuffer][threadId].empty();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::clear_pending(size_t threadId) noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks[mCurrentBuffer][threadId].clear();
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::push(const WorkerTaskType& task, size_t threadId) noexcept
{
    mPushLock.lock();
    mTasks[mCurrentBuffer][threadId].push_back(task);
    mPushLock.unlock();
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::emplace(WorkerTaskType&& task, size_t threadId) noexcept
{
    mPushLock.lock();
    mTasks[mCurrentBuffer][threadId].emplace_back(std::move(task));
    mPushLock.unlock();
}



/*-------------------------------------
 * Check if the thread has finished all tasks and can be flushed.
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerPool<WorkerTaskType>::ready() const noexcept
{
    return mIsPaused.load(std::memory_order_consume);
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerPool<WorkerTaskType>::flush() noexcept
{
    mPushLock.lock();
    const int currentBuffer = mCurrentBuffer;
    mCurrentBuffer = (currentBuffer + 1) & 1;
    mPushLock.unlock();

    mWaitMtx.lock();
    mIsPaused.store(false, std::memory_order_release);
    mExecCond.notify_all();
    mWaitMtx.unlock();
}



/*-------------------------------------
 * Wait for the current thread to finish execution.
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerPool<WorkerTaskType>::wait() const noexcept
{
    // Own the worker's mutex until the intended thread pauses. This should
    // effectively block the current thread of execution.

    #if 0
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

    #else
        while (!mIsPaused.load(std::memory_order_consume) && mActiveThreads.load(std::memory_order_consume) != 0)
        {
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #endif
        }

    #endif
}



/*-------------------------------------
 * Determine if busy waiting is enabled
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerPool<WorkerTaskType>::busy_waiting() const noexcept
{
    return mBusyWait.load(std::memory_order_consume);
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
    wait();

    mPushLock.lock();
    int lastBuffer = mCurrentBuffer;
    mCurrentBuffer = -1;
    mPushLock.unlock();

    mWaitMtx.lock();
    bool busyWaiting = mBusyWait.load(std::memory_order_consume);
    mBusyWait.store(true, std::memory_order_release);
    mIsPaused.store(false, std::memory_order_release);
    mWaitCond.notify_all();
    mWaitMtx.unlock();

    for (std::thread& t : mThreads)
    {
        t.join();
    }

    mThreads.clear();
    mBusyWait.store(busyWaiting, std::memory_order_release);

    mTasks[0].resize(inNumThreads, std::vector<WorkerTaskType>{});
    mTasks[1].resize(inNumThreads, std::vector<WorkerTaskType>{});

    mIsPaused.store(true, std::memory_order_release);
    mCurrentBuffer = lastBuffer;

    mWaitMtx.lock();
    for (size_t i = inNumThreads; i--;)
    {
        mThreads.emplace_back(std::thread{&WorkerPool<WorkerTaskType>::thread_loop, this});
    }
    mWaitMtx.unlock();

    return mThreads.size();
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t WorkerPool<WorkerTaskType>::concurrency() const noexcept
{
    return mThreads.size();
}



} // end utils namespace
} // end ls namespace
