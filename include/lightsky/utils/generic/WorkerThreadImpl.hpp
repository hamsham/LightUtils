
#ifndef LS_UTILS_WORKER_THREAD_IMPL_HPP
#define LS_UTILS_WORKER_THREAD_IMPL_HPP

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
    #ifndef LS_UTILS_USE_WINDOWS_THREADS
        mWaitMtx.lock();
        mIsPaused.store(true, std::memory_order_release);
        mWaitCond.notify_all();
        mWaitMtx.unlock();
    #else
        EnterCriticalSection(&mWaitMtx);
        mIsPaused.store(true, std::memory_order_release);
        WakeConditionVariable(&mWaitCond);
        LeaveCriticalSection(&mWaitMtx);
    #endif
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThread<WorkerTaskType>::thread_loop() noexcept
{
    while (true)
    {
        this->mPushLock.lock();
        bool amDone = this->mTasks.capacity() == 0;
        bool outOfTasks = this->mTasks.empty();
        this->mPushLock.unlock();

        if (amDone)
        {
            break;
        }

        if (outOfTasks || this->mIsPaused.load(std::memory_order_acquire))
        {
            // Busy waiting can be disabled at any time, but waiting on the
            // condition variable will remain in-place until the next flush.
            if (this->mBusyWait.load(std::memory_order_acquire))
            {
                continue;
            }
            else
            {
                #ifndef LS_UTILS_USE_WINDOWS_THREADS
                    std::unique_lock<std::mutex> cvLock{this->mWaitMtx};
                    this->mExecCond.wait(cvLock, [this]()->bool
                    {
                        return !this->mIsPaused.load(std::memory_order_acquire);
                    });

                #else
                    EnterCriticalSection(&mWaitMtx);
                    while (this->mIsPaused.load(std::memory_order_acquire))
                    {
                        SleepConditionVariableCS(&mExecCond, &mWaitMtx, (DWORD)0xFFFFFFFFFFFFFFFF); // INFINITE
                    }
                    LeaveCriticalSection(&mWaitMtx);

                #endif
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
    this->mTasks.clear();
    this->mTasks.shrink_to_fit();
    this->mPushLock.unlock();

    #ifndef LS_UTILS_USE_WINDOWS_THREADS
        this->mWaitMtx.lock();
        this->mIsPaused.store(false, std::memory_order_release);
        this->mWaitMtx.unlock();
        this->mExecCond.notify_all();
        this->mThread.join();

    #else
        EnterCriticalSection(&mWaitMtx);
        this->mIsPaused.store(false, std::memory_order_release);
        LeaveCriticalSection(&mWaitMtx);
        WakeConditionVariable(&mExecCond);

        this->mThread.join();
        DeleteCriticalSection(&mWaitMtx);

    #endif /* LS_UTILS_USE_WINDOWS_THREADS */
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <class WorkerTaskType>
WorkerThread<WorkerTaskType>::WorkerThread(unsigned affinity) noexcept :
    mBusyWait{false},
    mIsPaused{true},
    mPushLock{},
    mTasks{2},
    #ifndef LS_UTILS_USE_WINDOWS_THREADS
        mWaitMtx{},
        mWaitCond{},
        mExecCond{},
    #endif
    mThread{}
{
    (void)affinity;

    #ifdef LS_UTILS_USE_WINDOWS_THREADS
        InitializeCriticalSectionAndSpinCount(&mWaitMtx, 1);
        InitializeConditionVariable(&mWaitCond);
        InitializeConditionVariable(&mExecCond);
    #endif

    mThread = std::thread{&WorkerThread::thread_loop, this};
    if (affinity != ~0u)
    {
        set_thread_affinity(mThread, affinity);
    }
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
    if (this == &w)
    {
        return *this;
    }

    w.wait();
    this->wait();

    mBusyWait.store(w.mBusyWait.load(std::memory_order_consume), std::memory_order_release);
    mIsPaused.store(true, std::memory_order_release);
    mTasks = w.mTasks;

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
    mTasks = std::move(w.mTasks);

    return *this;
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline const ls::utils::RingBuffer<WorkerTaskType>& WorkerThread<WorkerTaskType>::tasks() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks;
}



/*-------------------------------------
 * Retrieve the current set of tasks to be performed asynchronously (should be
 * called after a flush).
-------------------------------------*/
template <class WorkerTaskType>
inline ls::utils::RingBuffer<WorkerTaskType>& WorkerThread<WorkerTaskType>::tasks() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks;
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline std::size_t WorkerThread<WorkerTaskType>::num_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return mTasks.size();
}



/*-------------------------------------
 * Get the number of tasks currently queued
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerThread<WorkerTaskType>::have_pending() const noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    return !mTasks.empty();
}



/*-------------------------------------
 * Clear the currently pending tasks
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::clear_pending() noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks.clear();
}



/*-------------------------------------
 * Push a task to the pending task queue (copy).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::push(const WorkerTaskType& task) noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks.push(task);
}



/*-------------------------------------
 * Push a task to the pending task queue (move).
-------------------------------------*/
template <class WorkerTaskType>
inline void WorkerThread<WorkerTaskType>::emplace(WorkerTaskType&& task) noexcept
{
    std::lock_guard<utils::SpinLock> lock{mPushLock};
    mTasks.emplace(std::forward<WorkerTaskType>(task));
}



/*-------------------------------------
 * Check if the thread has finished all tasks and can be flushed.
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerThread<WorkerTaskType>::ready() const noexcept
{
    return mIsPaused.load(std::memory_order_acquire);
}



/*-------------------------------------
 * Flush the current thread and swap I/O buffers (must only be called after
 * this->ready() returns true).
-------------------------------------*/
template <class WorkerTaskType>
void WorkerThread<WorkerTaskType>::flush() noexcept
{
    this->mPushLock.lock();
    bool haveTasks = !this->mTasks.empty();
    this->mPushLock.unlock();

    // Don't bother waking up the thread if there's nothing to do.
    if (haveTasks)
    {
        #ifndef LS_UTILS_USE_WINDOWS_THREADS
            this->mWaitMtx.lock();
            this->mIsPaused.store(false, std::memory_order_release);
            this->mExecCond.notify_one();
            this->mWaitMtx.unlock();
        #else
            EnterCriticalSection(&mWaitMtx);
            this->mIsPaused.store(false, std::memory_order_release);
            WakeConditionVariable(&mExecCond);
            LeaveCriticalSection(&mWaitMtx);
        #endif
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
        #ifndef LS_UTILS_USE_WINDOWS_THREADS
            std::unique_lock<std::mutex> cvLock{this->mWaitMtx};
            this->mWaitCond.wait(cvLock, [this]()->bool
            {
                return this->mIsPaused.load(std::memory_order_acquire);
            });

        #else
            EnterCriticalSection(&mWaitMtx);
            while (!mIsPaused.load(std::memory_order_consume))
            {
                SleepConditionVariableCS(&mWaitCond, &mWaitMtx, (DWORD)0xFFFFFFFFFFFFFFFF);
            }
            LeaveCriticalSection(&mWaitMtx);

        #endif
    }
}



/*-------------------------------------
 * Determine if busy waiting is enabled
-------------------------------------*/
template <class WorkerTaskType>
inline bool WorkerThread<WorkerTaskType>::busy_waiting() const noexcept
{
    return mBusyWait.load(std::memory_order_acquire);
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

#endif /* LS_UTILS_WORKER_THREAD_IMPL_HPP */
