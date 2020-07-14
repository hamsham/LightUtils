
namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Threaded Worker Object
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Execute the tasks in the queue.
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThread<WorkerTaskType>::execute_tasks() noexcept
{
    // The current I/O buffers were swapped when "flush()" was
    // called. Swap again to read and write to the buffers used on
    // this thread.
    int currentBufferId;
    mPushLock.lock();
    currentBufferId = mCurrentBuffer;
    mPushLock.unlock();

    // Exit the thread if mCurrentBuffer is less than 0.
    mWaitMtx.lock();

    if (currentBufferId >= 0)
    {
        currentBufferId = (currentBufferId + 1) % 2;
        std::vector<WorkerTaskType>& inputQueue = mTasks[currentBufferId];

        for (WorkerTaskType& pTask : inputQueue)
        {
            pTask();
        }

        this->mTasks[currentBufferId].clear();
    }

    // Pause the current thread again.
    mIsPaused.store(true, std::memory_order_release);
    mWaitCond.notify_all();
    mWaitMtx.unlock();
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
                std::unique_lock<std::mutex> cvLock{this->mWaitMtx};
                this->mExecCond.wait(cvLock, [this]()->bool
                {
                    return !this->mIsPaused.load(std::memory_order_acquire);
                });
            }
        }

        this->execute_tasks();
    }
}



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
    this->mWaitMtx.lock();
    this->mCurrentBuffer = -1;
    this->mIsPaused.store(false, std::memory_order_release);
    this->mWaitMtx.unlock();
    this->mPushLock.unlock();
    this->mExecCond.notify_all();
    this->mThread.join();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::WorkerThread() noexcept :
    mBusyWait{false},
    mIsPaused{true},
    mCurrentBuffer{0}, // must be greater than or equal to 0 for *this to run.
    mPushLock{},
    mTasks{},
    mWaitMtx{},
    mWaitCond{},
    mExecCond{},
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
    if (this == &w)
    {
        return *this;
    }

    w.wait();
    this->wait();

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);

    mIsPaused.store(true, std::memory_order_release);

    mCurrentBuffer = w.mCurrentBuffer;

    mTasks[0] = w.mTasks[0];
    mTasks[1] = w.mTasks[1];

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>& WorkerThread<WorkerTaskType>::operator=(WorkerThread&& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    w.wait();
    this->wait();

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);
    w.mBusyWait.store(false, std::memory_order_release);

    mIsPaused.store(true, std::memory_order_release);

    mCurrentBuffer = w.mCurrentBuffer;
    w.mCurrentBuffer = -1;

    mTasks[0] = std::move(w.mTasks[0]);
    mTasks[1] = std::move(w.mTasks[1]);

    return *this;
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline const std::vector<WorkerTaskType>& WorkerThread<WorkerTaskType>::tasks() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer];
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& WorkerThread<WorkerTaskType>::tasks() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer];
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline std::size_t WorkerThread<WorkerTaskType>::num_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer].size();
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerThread<WorkerTaskType>::have_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return !mTasks[mCurrentBuffer].empty();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::clear_pending() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks[mCurrentBuffer].clear();
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::push(const WorkerTaskType& task) noexcept
{
    mPushLock.lock();
    mTasks[mCurrentBuffer].push_back(task);
    mPushLock.unlock();
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::emplace(WorkerTaskType&& task) noexcept
{
    mPushLock.lock();
    mTasks[mCurrentBuffer].emplace_back(std::move(task));
    mPushLock.unlock();
}



/*-------------------------------------
 * Check if the thread has finished all tasks and can be flushed.
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerThread<WorkerTaskType>::ready() const noexcept
{
    return mIsPaused.load(std::memory_order_consume);
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
    const int swapped = !this->mTasks[currentBuffer].empty();
    this->mCurrentBuffer = (currentBuffer + swapped) & 1;
    this->mPushLock.unlock();

    // Don't bother waking up the thread if there's nothing to do.
    if (swapped)
    {
        this->mWaitMtx.lock();
        this->mTasks[this->mCurrentBuffer].clear();
        this->mIsPaused.store(false, std::memory_order_release);
        this->mExecCond.notify_one();
        this->mWaitMtx.unlock();
    }
}



/*-------------------------------------
 * Wait for the current thread to finish execution.
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::wait() const noexcept
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
        std::unique_lock<std::mutex> cvLock{this->mWaitMtx};
        this->mWaitCond.wait(cvLock, [this]()->bool
        {
            return this->mIsPaused.load(std::memory_order_acquire);
        });
    }
}



/*-------------------------------------
 * Determine if busy waiting is enabled
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerThread<WorkerTaskType>::busy_waiting() const noexcept
{
    return mBusyWait.load(std::memory_order_consume);
}



/*-------------------------------------
 * Toggle busy waiting
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::busy_waiting(bool useBusyWait) noexcept
{
    mBusyWait.store(useBusyWait, std::memory_order_release);
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t WorkerThread<WorkerTaskType>::concurrency(size_t inNumThreads) noexcept
{
    (void)inNumThreads;
    return 1;
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t WorkerThread<WorkerTaskType>::concurrency() const noexcept
{
    return 1;
}



} // end utils namespace
} // end ls namespace
