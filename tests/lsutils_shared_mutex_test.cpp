
#include <iostream>

#include <thread>
#include <vector>

#include "lightsky/utils/SpinLock.hpp"
#include "lightsky/utils/SharedMutex.hpp"

namespace utils = ls::utils;

typedef std::chrono::system_clock::time_point system_time_point;
typedef std::chrono::duration<long double, std::milli> system_duration;

constexpr std::chrono::milliseconds::rep THREAD_PAUSE_TIME = 2;

constexpr bool ENABLE_LOGGING = false;



// ----------------------------------------------------------------------------
class SharedRWMutex
{
  public:
    struct alignas(128) RWLockNode
    {
        utils::SpinLock lock;
        std::atomic<RWLockNode*> pNext;
    };

    static_assert(sizeof(RWLockNode) == 128, "Misaligned locking node.");

    typedef RWLockNode native_handle_type;

  private:
    RWLockNode mHead;
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
    mHead{{}, {nullptr}},
    mNumUsers{0}
{}



/*-------------------------------------
 * Enqueue Lock
-------------------------------------*/
void SharedRWMutex::_insert_node(RWLockNode& lock) noexcept
{
    RWLockNode* pHead = &mHead;
    bool swapped;

    lock.lock.lock();

    do
    {
        swapped = false;
        if (!pHead->lock.try_lock())
        {
            pHead = &mHead;
            continue;
        }

        RWLockNode* pNext = nullptr;
        swapped = pHead->pNext.compare_exchange_strong(pNext, &lock, std::memory_order_acq_rel, std::memory_order_relaxed);
        pHead->lock.unlock();

        pHead = pNext;
    }
    while (!swapped);

    lock.lock.unlock();
}



/*-------------------------------------
 * Enqueue Lock
-------------------------------------*/
bool SharedRWMutex::_try_insert_node(RWLockNode& lock) noexcept
{
    RWLockNode* pHead = &mHead;
    RWLockNode* pNext = nullptr;
    bool swapped = false;

    lock.lock.lock();

    if (pHead->lock.try_lock())
    {
        swapped = pHead->pNext.compare_exchange_strong(pNext, &lock, std::memory_order_acq_rel, std::memory_order_relaxed);
        pHead->lock.unlock();
    }

    lock.lock.unlock();

    return swapped;
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void SharedRWMutex::_lock_shared_impl(RWLockNode& lock) noexcept
{
    while (mHead.pNext.load() != &lock)
    {
        ls::setup::cpu_yield();
    }

    // wait for any remaining writes to complete
    while (mNumUsers.load() < 0)
    {
        ls::setup::cpu_yield();
    }

    mNumUsers.fetch_add(1);

    lock.lock.lock();
    mHead.lock.lock();
    mHead.pNext.store(lock.pNext.load());
    mHead.lock.unlock();
    lock.lock.unlock();
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
void SharedRWMutex::_lock_impl(RWLockNode& lock) noexcept
{
    while (mHead.pNext.load() != &lock)
    {
        ls::setup::cpu_yield();
    }

    #if 1
        // wait for any remaining writes to complete
        while (mNumUsers.load() != 0)
        {
            ls::setup::cpu_yield();
        }

        // wait for all remaining reads/writes to complete
        mNumUsers.store(-1ll);

    #else
        // wait for all remaining reads/writes to complete
        long long writeLock = 0;
        while (!mNumUsers.compare_exchange_strong(writeLock, -1ll, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            writeLock = 0;
        }
    #endif

    lock.lock.lock();
    mHead.lock.lock();
    mHead.pNext.store(lock.pNext.load());
    mHead.lock.unlock();
    lock.lock.unlock();
}



/*-------------------------------------
 * Non-Exclusive Lock
-------------------------------------*/
void SharedRWMutex::lock_shared() noexcept
{
    RWLockNode waitNode{{}, {nullptr}};

    _insert_node(waitNode);
    _lock_shared_impl(waitNode);
}



/*-------------------------------------
 * Exclusive Lock
-------------------------------------*/
void SharedRWMutex::lock() noexcept
{
    RWLockNode waitNode{{}, {nullptr}};

    _insert_node(waitNode);
    _lock_impl(waitNode);
}



/*-------------------------------------
 * Non-Exclusive Lock Attempt
-------------------------------------*/
bool SharedRWMutex::try_lock_shared() noexcept
{
    #if 1
        RWLockNode waitNode{{}, {nullptr}};

        if (mNumUsers.load() < 0)
        {
            return false;
        }

        bool acquiredLock = _try_insert_node(waitNode);
        if (acquiredLock)
        {
            _lock_shared_impl(waitNode);
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
    #if 0
        RWLockNode waitNode{{}, {nullptr}};

        if (mNumUsers.load() < 0)
        {
            return false;
        }

        bool acquiredLock = _try_insert_node(waitNode);
        if (acquiredLock)
        {
            _lock_impl(waitNode);
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
    long long amShared = mNumUsers.fetch_sub(1);
    (void)amShared;
    LS_DEBUG_ASSERT(amShared > 0);
}



/*-------------------------------------
 * Exclusive Unlock
-------------------------------------*/
void SharedRWMutex::unlock() noexcept
{
    long long amExclusive = mNumUsers.exchange(0);
    (void)amExclusive;
    LS_DEBUG_ASSERT(amExclusive == -1);
}



/*-------------------------------------
 * Retrieve the internal lock
-------------------------------------*/
const typename SharedRWMutex::native_handle_type& SharedRWMutex::native_handle() const noexcept
{
    return mHead;
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
    for (unsigned i = 0; i < numTests; ++i)
    {
        mutexRunTime += run_test<utils::SharedMutex>();
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with SharedFutex..." << std::endl;
    system_duration futexRunTime{0};
    for (unsigned i = 0; i < numTests; ++i)
    {
        futexRunTime += run_test<utils::SharedFutex>();
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with SharedRWMutex..." << std::endl;
    system_duration rwLockRunTime{0};
    for (unsigned i = 0; i < numTests; ++i)
    {
        rwLockRunTime += run_test<SharedRWMutex>();
    }
    std::cout << "\tDone." << std::endl;

    std::cout
        << "Results:"
        << "\n\tShared Mutex Time: " << mutexRunTime.count() << "ms"
        << "\n\tShared Futex Time: " << futexRunTime.count() << "ms"
        << "\n\tShared Futex Time: " << rwLockRunTime.count() << "ms"
        << std::endl;

    return 0;
}
