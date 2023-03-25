
#include <iostream>

#include <thread>
#include <vector>

#include "lightsky/utils/SpinLock.hpp"
#include "lightsky/utils/SharedMutex.hpp"

namespace utils = ls::utils;

typedef std::chrono::system_clock::time_point system_time_point;
typedef std::chrono::duration<long double, std::milli> system_duration;

constexpr std::chrono::milliseconds::rep THREAD_PAUSE_TIME = 1;

constexpr bool ENABLE_LOGGING = false;



// ----------------------------------------------------------------------------
class SharedRWMutex
{
  public:
    typedef ls::utils::SpinLock native_handle_type;
    //typedef ls::utils::Futex native_handle_type;
    //typedef std::mutex native_handle_type;

    struct alignas(128) RWLockNode
    {
        native_handle_type mLock;
        std::atomic<native_handle_type*> mNextLock;
        std::atomic<RWLockNode*> pNext;
        std::atomic<RWLockNode*> pPrev;
    };

    static_assert(sizeof(RWLockNode) == 128, "Misaligned locking node.");

  private:
    RWLockNode mHead;
    RWLockNode mTail;
    std::atomic_llong mNumUsers;

    void _insert_node(RWLockNode& lock) noexcept;

    bool _try_insert_node(RWLockNode& lock) noexcept;

    void _lock_shared_impl(RWLockNode& lock) noexcept;

    void _lock_impl(RWLockNode& lock) noexcept;

  public:
    ~SharedRWMutex() noexcept = default;

    SharedRWMutex() noexcept;

    SharedRWMutex(const SharedRWMutex&) noexcept = delete;

    SharedRWMutex(SharedRWMutex&&) noexcept = delete;

    SharedRWMutex& operator=(const SharedRWMutex&) noexcept = delete;

    SharedRWMutex& operator=(SharedRWMutex&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};



/*-------------------------------------
 * Constructor
-------------------------------------*/
SharedRWMutex::SharedRWMutex() noexcept :
    mHead{{}, {nullptr}, {nullptr}, {nullptr}},
    mTail{{}, {nullptr}, {nullptr}, {&mHead}},
    mNumUsers{0}
{
    mHead.mNextLock = &mTail.mLock;
    mHead.pNext.store(&mTail);
}



/*-------------------------------------
 * Enqueue Lock
-------------------------------------*/
void SharedRWMutex::_insert_node(RWLockNode& lock) noexcept
{
    RWLockNode* const pTail = &mTail;
    RWLockNode* pPrev;

    lock.mNextLock.store(&pTail->mLock, std::memory_order_relaxed);
    lock.pNext.store(pTail, std::memory_order_release);

    pTail->mLock.lock();
    lock.mLock.lock();

    pPrev = pTail->pPrev.load(std::memory_order_acquire);
    lock.pPrev.store(pPrev, std::memory_order_release);
    pTail->pPrev.store(&lock, std::memory_order_release);

    pPrev->pNext.store(&lock, std::memory_order_release);
    pPrev->mNextLock.store(&lock.mLock, std::memory_order_relaxed);

    lock.mLock.unlock();
    pTail->mLock.unlock();
}



/*-------------------------------------
 * Enqueue Lock
-------------------------------------*/
bool SharedRWMutex::_try_insert_node(RWLockNode& lock) noexcept
{
    RWLockNode* const pTail = &mTail;
    RWLockNode* pPrev;

    bool haveLock = pTail->mLock.try_lock();
    if (haveLock)
    {
        lock.mLock.lock();
        lock.mNextLock.store(&pTail->mLock, std::memory_order_relaxed);
        lock.pNext.store(pTail, std::memory_order_release);

        pPrev = pTail->pPrev.load(std::memory_order_acquire);
        lock.pPrev.store(pPrev, std::memory_order_release);
        pTail->pPrev.store(&lock, std::memory_order_release);

        pPrev->pNext.store(&lock, std::memory_order_release);
        pPrev->mNextLock.store(&lock.mLock, std::memory_order_relaxed);

        lock.mLock.unlock();
        pTail->mLock.unlock();
    }

    return haveLock;
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void SharedRWMutex::_lock_shared_impl(RWLockNode& lock) noexcept
{
    do
    {
        if (mHead.pNext.load(std::memory_order_acquire) == &lock)
        {
            if (mNumUsers.load(std::memory_order_acquire) >= 0)
            {
                mNumUsers.fetch_add(1, std::memory_order_acq_rel);
                break;
            }
        }
    }
    while (true);

    RWLockNode* pNext;
    native_handle_type* mtx = lock.mNextLock.load(std::memory_order_relaxed);

    mtx->lock();

    pNext = lock.pNext.load(std::memory_order_acquire);
    pNext->pPrev.store(&mHead, std::memory_order_release);

    mHead.mNextLock.store(mtx, std::memory_order_relaxed);
    mHead.pNext.store(pNext, std::memory_order_release);

    mtx->unlock();
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
void SharedRWMutex::_lock_impl(RWLockNode& lock) noexcept
{
    do
    {
        if (mHead.pNext.load(std::memory_order_acquire) == &lock)
        {
            if (mNumUsers.load(std::memory_order_acquire) == 0)
            {
                mNumUsers.store(-1, std::memory_order_release);
                break;
            }
        }
    }
    while (true);

    RWLockNode* pNext;
    native_handle_type* mtx = lock.mNextLock.load(std::memory_order_relaxed);

    mtx->lock();

    pNext = lock.pNext.load(std::memory_order_acquire);
    pNext->pPrev.store(&mHead, std::memory_order_release);

    mHead.mNextLock.store(mtx, std::memory_order_relaxed);
    mHead.pNext.store(pNext, std::memory_order_release);

    mtx->unlock();
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void SharedRWMutex::lock_shared() noexcept
{
    RWLockNode waitNode{{}, {nullptr}, {nullptr}, {nullptr}};

    _insert_node(waitNode);
    _lock_shared_impl(waitNode);
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
void SharedRWMutex::lock() noexcept
{
    RWLockNode waitNode{{}, {nullptr}, {nullptr}, {nullptr}};

    _insert_node(waitNode);
    _lock_impl(waitNode);
}



/*-------------------------------------
 * Non-Exclusive Lock Attempt
-------------------------------------*/
bool SharedRWMutex::try_lock_shared() noexcept
{
    #if 1
        RWLockNode waitNode{{}, {nullptr}, {nullptr}, {nullptr}};
        bool acquiredLock = false;

        if (mNumUsers.load(std::memory_order_relaxed) == 0)
        {
            acquiredLock = _try_insert_node(waitNode);
            if (acquiredLock)
            {
                _lock_shared_impl(waitNode);
            }
        }

        return acquiredLock;

    #else
        return false;
    #endif
}



/*-------------------------------------
 * Exclusive Lock Attempt
-------------------------------------*/
bool SharedRWMutex::try_lock() noexcept
{
    #if 1
        RWLockNode waitNode{{}, {nullptr}, {nullptr}, {nullptr}};
        bool acquiredLock = false;

        if (mNumUsers.load(std::memory_order_relaxed) == 0)
        {
            acquiredLock = _try_insert_node(waitNode);
            if (acquiredLock)
            {
                _lock_impl(waitNode);
            }
        }

        return acquiredLock;

    #else
        return false;
    #endif
}



/*-------------------------------------
 * Non-Exclusive Unlock
-------------------------------------*/
void SharedRWMutex::unlock_shared() noexcept
{
    long long amShared = mNumUsers.fetch_sub(1, std::memory_order_acq_rel);
    (void)amShared;
    LS_DEBUG_ASSERT(amShared > 0);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
void SharedRWMutex::unlock() noexcept
{
    LS_DEBUG_ASSERT(-1 == mNumUsers.load(std::memory_order_acquire));
    mNumUsers.store(0, std::memory_order_release);
}



/*-------------------------------------
 * Retrieve the internal lock
-------------------------------------*/
const typename SharedRWMutex::native_handle_type& SharedRWMutex::native_handle() const noexcept
{
    return mHead.mLock;
}



// ----------------------------------------------------------------------------



inline unsigned num_test_threads() noexcept
{
    const unsigned concurrency = (unsigned)std::thread::hardware_concurrency();

    if (concurrency <= 2)
    {
        return 2;
    }

    // ensure an even number of test threads
    return concurrency - (concurrency % 2);
    //return 4;
}



inline void log_lock_state(utils::SpinLock& ioLock, const char* msg, const std::thread::id& id)
{
    if (ENABLE_LOGGING)
    {
        std::lock_guard<utils::SpinLock> lock{ioLock};
        std::cout << msg << id << '\n';
    }
}



template <typename SharedMutexType>
void exclusive_lock_func(
    SharedMutexType& rwLock,
    utils::SpinLock& ioLock,
    std::atomic_uint& numThreadsWaiting,
    std::atomic_uint& numThreadsFinished)
{
    constexpr std::chrono::milliseconds pauseTime{THREAD_PAUSE_TIME};

    numThreadsWaiting.fetch_sub(1u);
    while (numThreadsWaiting.load() > 0); // spin

    for (unsigned j = 0; j < 3; ++j)
    {
        rwLock.lock();
        log_lock_state(ioLock, "exclusive lock: ", std::this_thread::get_id());
        std::this_thread::sleep_for(pauseTime);
        log_lock_state(ioLock, "exclusive unlock: ", std::this_thread::get_id());

        LS_ASSERT(!rwLock.try_lock_shared());

        rwLock.unlock();

        if (rwLock.try_lock_shared())
        {
            log_lock_state(ioLock, "try_lock_shared: ", std::this_thread::get_id());
            rwLock.unlock_shared();
        }
    }

    numThreadsFinished.fetch_add(1u);
}



template <typename SharedMutexType>
void shared_lock_func(
    SharedMutexType& rwLock,
    utils::SpinLock& ioLock,
    std::atomic_uint& numThreadsWaiting,
    std::atomic_uint& numThreadsFinished)
{
    constexpr std::chrono::milliseconds pauseTime{THREAD_PAUSE_TIME};

    numThreadsWaiting.fetch_sub(1u);
    while (numThreadsWaiting.load() > 0); // spin

    for (unsigned j = 0; j < 3; ++j)
    {
        rwLock.lock_shared();
        log_lock_state(ioLock, "shared lock: ", std::this_thread::get_id());
        std::this_thread::sleep_for(pauseTime);
        log_lock_state(ioLock, "shared unlock: ", std::this_thread::get_id());

        LS_ASSERT(!rwLock.try_lock());

        rwLock.unlock_shared();

        if (rwLock.try_lock())
        {
            log_lock_state(ioLock, "try_lock: ", std::this_thread::get_id());
            rwLock.unlock();
        }
    }

    numThreadsFinished.fetch_add(1u);
}



template <typename SharedMutexType>
system_duration run_test()
{
    const unsigned concurrency = num_test_threads();
    std::atomic_uint numThreadsWaiting{concurrency};
    std::atomic_uint numThreadsFinished{0};
    std::vector<std::thread> threads;
    utils::SpinLock ioLock;
    SharedMutexType rwLock;

    threads.reserve(concurrency);

    for (unsigned i = 0; i < concurrency; ++i)
    {
        if (i & 1)
        {
            threads.emplace_back(
                exclusive_lock_func<SharedMutexType>,
                std::ref(rwLock),
                std::ref(ioLock),
                std::ref(numThreadsWaiting),
                std::ref(numThreadsFinished));
        }
        else
        {
            threads.emplace_back(
                shared_lock_func<SharedMutexType>,
                std::ref(rwLock),
                std::ref(ioLock),
                std::ref(numThreadsWaiting),
                std::ref(numThreadsFinished));
        }
    }

    std::this_thread::yield();

    const system_time_point&& startTime = std::chrono::system_clock::now();

    for (std::thread& t : threads)
    {
        t.join();
    }

    return system_duration{std::chrono::system_clock::now() - startTime};
}



// ----------------------------------------------------------------------------
int main()
{
    const unsigned concurrency = num_test_threads();
    const unsigned numTests = concurrency*2;

    std::cout
        << "Starting test:"
        << "\n\tThread Count:  " << concurrency
        << "\n\tRead Threads:  " << (concurrency/2)
        << "\n\tWrite Threads: " << (concurrency/2)
        << std::endl;

    std::cout << "Running Test with SharedMutex..." << std::endl;
    system_duration mutexRunTime{0};
    #if 1
        for (unsigned i = 0; i < numTests; ++i)
        {
            mutexRunTime += run_test<utils::SharedMutex>();
        }
    #endif
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with SharedFutex..." << std::endl;
    system_duration futexRunTime{0};
    #if 1
        for (unsigned i = 0; i < numTests; ++i)
        {
            futexRunTime += run_test<utils::SharedFutex>();
        }
    #endif
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with SharedRWMutex..." << std::endl;
    system_duration rwLockRunTime{0};
    #if 1
        for (unsigned i = 0; i < numTests; ++i)
        {
            rwLockRunTime += run_test<SharedRWMutex>();
        }
    #endif
    std::cout << "\tDone." << std::endl;

    std::cout
        << "Results:"
        << "\n\tShared Mutex Time: " << mutexRunTime.count() << "ms"
        << "\n\tShared Futex Time: " << futexRunTime.count() << "ms"
        << "\n\tShared Futex Time: " << rwLockRunTime.count() << "ms"
        << std::endl;

    return 0;
}
