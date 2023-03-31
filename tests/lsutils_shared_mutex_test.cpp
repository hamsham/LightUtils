
#include <iostream>

#include <thread>
#include <vector>

#include "lightsky/utils/FairRWLock.hpp"
#include "lightsky/utils/SharedMutex.hpp"

namespace utils = ls::utils;

typedef std::chrono::system_clock::time_point system_time_point;
typedef std::chrono::duration<long double, std::milli> system_duration;

constexpr bool ENABLE_LOGGING = false;
constexpr unsigned LOCK_CONTENTION_COUNT = 1000;



// ----------------------------------------------------------------------------
// Gather the number of threads to test with
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



// ----------------------------------------------------------------------------
// Debug Logging
// ----------------------------------------------------------------------------
inline void log_lock_state(utils::SpinLock& ioLock, const char* msg, const std::thread::id& id)
{
    if (ENABLE_LOGGING)
    {
        std::lock_guard<utils::SpinLock> lock{ioLock};
        std::cout << msg << id << '\n';
    }
}



// ----------------------------------------------------------------------------
// Test for exclusive locking of a mutex
// ----------------------------------------------------------------------------
template <typename MutexType>
void mutex_lock_func(
    MutexType& rwLock,
    utils::SpinLock& ioLock,
    std::condition_variable& cv,
    std::atomic_uint& numThreadsRunning)
{
    for (unsigned j = 0; j < LOCK_CONTENTION_COUNT; ++j)
    {
        rwLock.lock();
        log_lock_state(ioLock, "MTX exclusive lock: ", std::this_thread::get_id());

        LS_ASSERT(!rwLock.try_lock());

        rwLock.unlock();
        log_lock_state(ioLock, "MTX exclusive unlock: ", std::this_thread::get_id());

        if (rwLock.try_lock())
        {
            log_lock_state(ioLock, "MTX try_lock: ", std::this_thread::get_id());

            rwLock.unlock();
            log_lock_state(ioLock, "MTX unlock try_lock: ", std::this_thread::get_id());
        }
    }

    unsigned threadId = numThreadsRunning.fetch_sub(1u);
    if (threadId == 1)
    {
        cv.notify_one();
    }
}



// ----------------------------------------------------------------------------
// Test for exclusive locking of a rw lock
// ----------------------------------------------------------------------------
template <typename SharedMutexType>
void exclusive_lock_func(
    SharedMutexType& rwLock,
    utils::SpinLock& ioLock,
    std::condition_variable& cv,
    std::atomic_uint& numThreadsRunning)
{
    for (unsigned i = 0; i < LOCK_CONTENTION_COUNT; ++i)
    {
        rwLock.lock();
        log_lock_state(ioLock, "exclusive lock: ", std::this_thread::get_id());

        LS_ASSERT(!rwLock.try_lock_shared());
        log_lock_state(ioLock, "exclusive unlock: ", std::this_thread::get_id());

        rwLock.unlock();

        if (rwLock.try_lock())
        {
            log_lock_state(ioLock, "try_lock: ", std::this_thread::get_id());

            rwLock.unlock();
            log_lock_state(ioLock, "unlock try_lock: ", std::this_thread::get_id());
        }
    }

    unsigned threadId = numThreadsRunning.fetch_sub(1u);
    if (threadId == 1)
    {
        cv.notify_one();
    }
}



// ----------------------------------------------------------------------------
// Test for shared locking of a rw lock
// ----------------------------------------------------------------------------
template <typename SharedMutexType>
void shared_lock_func(
    SharedMutexType& rwLock,
    utils::SpinLock& ioLock,
    std::condition_variable& cv,
    std::atomic_uint& numThreadsRunning)
{
    for (unsigned i = 0; i < LOCK_CONTENTION_COUNT; ++i)
    {
        rwLock.lock_shared();
        log_lock_state(ioLock, "shared lock: ", std::this_thread::get_id());

        LS_ASSERT(!rwLock.try_lock());

        rwLock.unlock_shared();
        log_lock_state(ioLock, "shared unlock: ", std::this_thread::get_id());

        if (rwLock.try_lock_shared())
        {
            log_lock_state(ioLock, "try_lock_shared: ", std::this_thread::get_id());

            rwLock.unlock_shared();
            log_lock_state(ioLock, "unlock try_lock_shared: ", std::this_thread::get_id());
        }
    }

    unsigned threadId = numThreadsRunning.fetch_sub(1u);
    if (threadId == 1)
    {
        cv.notify_one();
    }
}



// ----------------------------------------------------------------------------
// Launch threads to test an exclusive lock
// ----------------------------------------------------------------------------
template <typename MutexType>
system_duration run_mtx_test()
{
    const unsigned concurrency = num_test_threads();
    std::atomic_uint numThreadsRunning{concurrency};
    std::condition_variable cv;
    std::mutex mtx;
    std::vector<std::thread> threads;
    utils::SpinLock ioLock;
    MutexType rwLock;

    threads.reserve(concurrency);

    for (unsigned i = 0; i < concurrency; ++i)
    {
        threads.emplace_back(
            mutex_lock_func<MutexType>,
            std::ref(rwLock),
            std::ref(ioLock),
            std::ref(cv),
            std::ref(numThreadsRunning));
    }

    const system_time_point&& startTime = std::chrono::system_clock::now();

    std::unique_lock<std::mutex> lk{mtx};
    while (numThreadsRunning.load(std::memory_order_relaxed) > 0)
    {
        cv.wait(lk);
    }

    const system_duration&& runTime = system_duration{std::chrono::system_clock::now() - startTime};

    for (std::thread& t : threads)
    {
        t.join();
    }

    return runTime;
}



// ----------------------------------------------------------------------------
// Launch threads to test a shared lock
// ----------------------------------------------------------------------------
template <typename SharedMutexType>
system_duration run_rw_test()
{
    const unsigned concurrency = num_test_threads();
    std::atomic_uint numThreadsRunning{concurrency};
    std::condition_variable cv;
    std::mutex mtx;
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
                std::ref(cv),
                std::ref(numThreadsRunning));
        }
        else
        {
            threads.emplace_back(
                shared_lock_func<SharedMutexType>,
                std::ref(rwLock),
                std::ref(ioLock),
                std::ref(cv),
                std::ref(numThreadsRunning));
        }
    }

    const system_time_point&& startTime = std::chrono::system_clock::now();

    std::unique_lock<std::mutex> lk{mtx};
    while (numThreadsRunning.load(std::memory_order_relaxed) > 0)
    {
        cv.wait(lk);
    }

    const system_duration&& runTime = system_duration{std::chrono::system_clock::now() - startTime};

    for (std::thread& t : threads)
    {
        t.join();
    }

    return runTime;
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

    /*
     * Exclusive-lock benchmarks
     */
    std::cout << "Running Test with std::mutex..." << std::endl;
    system_duration mutexRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            mutexRunTime += run_mtx_test<std::mutex>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with utils::Futex..." << std::endl;
    system_duration futexRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            futexRunTime += run_mtx_test<utils::Futex>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with utils::SpinLock..." << std::endl;
    system_duration spinlockRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            spinlockRunTime += run_mtx_test<utils::SpinLock>();
        }
    }
    std::cout << "\tDone." << std::endl;

    /*
     * Shared-lock benchmarks
     */
    std::cout << "Running Test with SharedMutex..." << std::endl;
    system_duration rwMutexRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            rwMutexRunTime += run_rw_test<utils::SharedMutex>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with SharedFutex..." << std::endl;
    system_duration rwFutexRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            rwFutexRunTime += run_rw_test<utils::SharedFutex>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with SharedSpinLock..." << std::endl;
    system_duration rwSpinRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            rwSpinRunTime += run_rw_test<utils::SharedSpinLock>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with FairRWMutex..." << std::endl;
    system_duration fairMutexRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            fairMutexRunTime += run_rw_test<utils::FairRWMutex>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with FairRWFutex..." << std::endl;
    system_duration fairFutexRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            fairFutexRunTime += run_rw_test<utils::FairRWFutex>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout << "Running Test with FairRWSpinLock..." << std::endl;
    system_duration fairSpinlockRunTime{0};
    if (1)
    {
        for (unsigned i = 0; i < numTests; ++i)
        {
            fairSpinlockRunTime += run_rw_test<utils::FairRWSpinLock>();
        }
    }
    std::cout << "\tDone." << std::endl;

    std::cout
        << "Results:"
        << "\n\tstd::mutex Time:      " << mutexRunTime.count() << "ms"
        << "\n\tutils::Futex Time:    " << futexRunTime.count() << "ms"
        << "\n\tutils::SpinLock Time: " << spinlockRunTime.count() << "ms"
        << "\n\tShared Mutex Time:    " << rwMutexRunTime.count() << "ms"
        << "\n\tShared Futex Time:    " << rwFutexRunTime.count() << "ms"
        << "\n\tShared SpinLock Time: " << rwSpinRunTime.count() << "ms"
        << "\n\tFair Mutex Time:      " << fairMutexRunTime.count() << "ms"
        << "\n\tFair Futex Time:      " << fairFutexRunTime.count() << "ms"
        << "\n\tFair SpinLock Time:   " << fairSpinlockRunTime.count() << "ms"
        << std::endl;

    return 0;
}
