
namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Basic Worker Object (Non-Threaded)
-----------------------------------------------------------------------------*/
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
    mTasks{},
    mWaitMtx{},
    mWaitCond{}
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
Worker<WorkerTaskType>& Worker<WorkerTaskType>::operator=(Worker&& w) noexcept
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
 * Execute the tasks in the queue.
-------------------------------------*/
template <class WorkerTaskType>
void Worker<WorkerTaskType>::execute_tasks() noexcept
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
inline void Worker<WorkerTaskType>::push(const WorkerTaskType& task) noexcept
{
    mPushLock.lock();
    mTasks[mCurrentBuffer].push_back(task);
    mPushLock.unlock();
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::emplace(WorkerTaskType&& task) noexcept
{
    mPushLock.lock();
    mTasks[mCurrentBuffer].emplace_back(std::move(task));
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
inline const std::vector<WorkerTaskType>& Worker<WorkerTaskType>::tasks() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer];
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline std::vector<WorkerTaskType>& Worker<WorkerTaskType>::tasks() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer];
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline std::size_t Worker<WorkerTaskType>::num_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks[mCurrentBuffer].size();
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline bool Worker<WorkerTaskType>::have_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return !mTasks[mCurrentBuffer].empty();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
inline void Worker<WorkerTaskType>::clear_pending() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks[mCurrentBuffer].clear();
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
    const bool unpause = !this->mTasks[currentBuffer].empty();
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
inline void Worker<WorkerTaskType>::wait() const noexcept
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
    /*
    while (!mIsPaused.load(std::memory_order_consume))
    {
        if (mBusyWait.load(std::memory_order_consume))
        {
            continue;
        }
        else
        {
            mWaitMtx.lock();
            mWaitMtx.unlock();
        }
    }
    */
}



/*-------------------------------------
 * Determine if busy waiting is enabled
-------------------------------------*/
template <class WorkerTaskType>
inline bool Worker<WorkerTaskType>::busy_waiting() const noexcept
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



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t Worker<WorkerTaskType>::concurrency(size_t inNumThreads) noexcept
{
    (void)inNumThreads;
    return 1;
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t Worker<WorkerTaskType>::concurrency() const noexcept
{
    return 1;
}



/*-----------------------------------------------------------------------------
 * Threaded Worker Object
-----------------------------------------------------------------------------*/
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
    Worker<WorkerTaskType>{},
    mExecCond{},
    mThread{}
{
    mThread = std::thread{&WorkerThread::thread_loop, this};

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
        this->mExecCond.notify_all();
        this->mWaitMtx.unlock();
    }
}



/*-----------------------------------------------------------------------------
 * Threaded Worker Object
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThreadGroup<WorkerTaskType>::~WorkerThreadGroup() noexcept
{
    stop_thread_loop();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThreadGroup<WorkerTaskType>::WorkerThreadGroup() noexcept :
    Worker<WorkerTaskType>{},
    mNumThreads{1},
    mThreadsRunning{0},
    mExecCond{},
    mThreads{new(std::nothrow) std::thread[1]{std::move(std::thread{&WorkerThreadGroup::thread_loop, this})}}
{
    // let the thread's loop run until it hits the condition variable
    std::this_thread::yield();
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThreadGroup<WorkerTaskType>::WorkerThreadGroup(const WorkerThreadGroup& w) noexcept :
    WorkerThreadGroup{} // delegate constructor
{
    *this = w;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThreadGroup<WorkerTaskType>::WorkerThreadGroup(WorkerThreadGroup&& w) noexcept :
    WorkerThreadGroup{} // delegate constructor
{
    *this = std::move(w);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerThreadGroup<WorkerTaskType>& WorkerThreadGroup<WorkerTaskType>::operator=(const WorkerThreadGroup& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    Worker<WorkerTaskType>::operator=(w);

    if (this->concurrency() != w.concurrency())
    {
        this->concurrency(w.concurrency());
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <class WorkerTaskType>
WorkerThreadGroup<WorkerTaskType>& WorkerThreadGroup<WorkerTaskType>::operator=(WorkerThreadGroup&& w) noexcept
{
    if (this == &w)
    {
        return *this;
    }

    Worker<WorkerTaskType>::operator=(std::move(w));

    if (this->concurrency() != w.concurrency())
    {
        const size_t inNumThreads = w.concurrency();
        w.stop_thread_loop();

        this->concurrency(inNumThreads);
    }

    return *this;
}



/*-------------------------------------
 * Execute tasks in the thread loop
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThreadGroup<WorkerTaskType>::execute_tasks() noexcept
{
    // The current I/O buffers were swapped when "flush()" was
    // called. Swap again to read and write to the buffers used on
    // this thread.
    int currentBufferId;
    this->mPushLock.lock();
    currentBufferId = this->mCurrentBuffer;
    this->mPushLock.unlock();

    // Exit the thread if mCurrentBuffer is less than 0.
    long long threadId = this->mThreadsRunning.fetch_add(1);
    if (threadId == this->mNumThreads-1)
    {
        this->mWaitMtx.lock();
        this->mThreadsRunning.store(-this->mNumThreads - 1);
    }
    else
    {
        while (this->mThreadsRunning.load() > 0)
        {
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #endif
        }
    }

    if (currentBufferId >= 0)
    {
        currentBufferId = (currentBufferId + 1) % 2;
        const std::vector<WorkerTaskType>& inputQueue = this->mTasks[currentBufferId];

        for (WorkerTaskType pTask : inputQueue)
        {
            pTask();
        }
    }

    if (this->mThreadsRunning.fetch_add(1) == -2)
    {
        // Pause the current thread again.
        this->mIsPaused.store(true);
        this->mThreadsRunning.store(0);

        if (currentBufferId >= 0)
        {
            this->mTasks[currentBufferId].clear();
        }

        this->mWaitCond.notify_all();
        this->mWaitMtx.unlock();
    }
    else
    {
        while (this->mThreadsRunning.load() < 0)
        {
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #endif
        }
    }
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThreadGroup<WorkerTaskType>::thread_loop() noexcept
{
    while (this->mCurrentBuffer >= 0)
    {
        // Busy waiting can be disabled at any time, but waiting on the
        // condition variable will remain in-place until the next flush.
        if (this->mIsPaused.load())
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
                    return !this->mIsPaused.load();
                });
            }
        }

        this->execute_tasks();
    }
}



/*-------------------------------------
 * Stop the thread loop in case we're changing the number of threads
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThreadGroup<WorkerTaskType>::stop_thread_loop() noexcept
{
    while (!this->ready())
    {
    }

    this->mPushLock.lock();
    this->mWaitMtx.lock();

    this->mCurrentBuffer = -1;
    this->mIsPaused.store(false, std::memory_order_release);
    this->mExecCond.notify_all();

    this->mWaitMtx.unlock();
    this->mPushLock.unlock();

    for (long long i = 0; i < this->mNumThreads; ++i)
    {
        this->mThreads[i].join();
    }

    this->mNumThreads = 0;
    this->mThreads.reset();

    this->mTasks[0].clear();
    this->mTasks[1].clear();
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThreadGroup<WorkerTaskType>::flush() noexcept
{
    this->mPushLock.lock();
    const int currentBuffer = this->mCurrentBuffer;
    const int swapped = !this->mTasks[currentBuffer].empty();
    this->mCurrentBuffer = (currentBuffer + swapped) % 2;
    this->mPushLock.unlock();

    // Don't bother waking up the thread if there's nothing to do.
    if (swapped)
    {
        this->mWaitMtx.lock();
        this->mIsPaused.store(false, std::memory_order_release);
        this->mExecCond.notify_all();
        this->mWaitMtx.unlock();
    }
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
size_t WorkerThreadGroup<WorkerTaskType>::concurrency(size_t inNumThreads) noexcept
{
    // Don't do anything if we couldn't change the number of threads.
    ls::utils::Pointer<std::thread[]> threads;

    if ((long long)inNumThreads == this->mNumThreads)
    {
        return inNumThreads;
    }

    int oldBuffer = this->mCurrentBuffer >= 0 ? this->mCurrentBuffer : 0;
    stop_thread_loop();
    this->mThreads.reset(nullptr);

    if (inNumThreads > 0)
    {
        threads.reset(new(std::nothrow) std::thread[inNumThreads]);
        if (!threads)
        {
            inNumThreads = 0;
        }
        else
        {
            this->mNumThreads = (long long)inNumThreads;
            this->mThreads = std::move(threads);
            this->mCurrentBuffer = oldBuffer;
        }
    }

    // Make sure the old threads have stopped
    this->mIsPaused.store(true, std::memory_order_release);

    for (size_t i = 0; i < inNumThreads; ++i)
    {
        this->mThreads[i] = std::move(std::thread{&WorkerThreadGroup::thread_loop, this});
    }

    std::this_thread::yield();

    return inNumThreads;
}



/*-------------------------------------
 * Thread count
-------------------------------------*/
template <class WorkerTaskType>
inline size_t WorkerThreadGroup<WorkerTaskType>::concurrency() const noexcept
{
    return (size_t)mNumThreads;
}



} // end utils namespace
} // end ls namespace
