
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
#if 1
    const unsigned concurrency = (unsigned)std::thread::hardware_concurrency();

    if (concurrency <= 2)
    {
        return 2;
    }

    // ensure an even number of test threads
    return concurrency - (concurrency % 2);
#else

    return 4;
#endif
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
system_duration run_mtx_test(const char* testName, unsigned numTests)
{
    std::cout << "Running exclusive test with " << testName << "..." << std::endl;

    system_duration runTime{0};
    const unsigned concurrency = num_test_threads();
    std::condition_variable cv;
    std::mutex mtx;
    std::vector<std::thread> threads;
    utils::SpinLock ioLock;
    MutexType rwLock;

    threads.resize(concurrency);

    for (unsigned testId = 0; testId < numTests; ++testId)
    {
        std::atomic_uint numThreadsRunning{concurrency};

        for (unsigned i = 0; i < concurrency; ++i)
        {
            threads[i] = std::thread{
                mutex_lock_func<MutexType>,
                std::ref(rwLock),
                std::ref(ioLock),
                std::ref(cv),
                std::ref(numThreadsRunning)
            };
        }

        const system_time_point&& startTime = std::chrono::system_clock::now();

        std::unique_lock<std::mutex> lk{mtx};
        while (numThreadsRunning.load(std::memory_order_relaxed) > 0)
        {
            cv.wait(lk);
        }

        runTime += system_duration{std::chrono::system_clock::now() - startTime};

        for (std::thread& t : threads)
        {
            t.join();
        }
    }

    std::cout << "\tDone." << std::endl;
    return runTime;
}



// ----------------------------------------------------------------------------
// Launch threads to test a shared lock
// ----------------------------------------------------------------------------
template <typename SharedMutexType>
system_duration run_rw_test(const char* testName, unsigned numTests)
{
    std::cout << "Running RW test with " << testName << "..." << std::endl;

    system_duration runTime{0};
    const unsigned concurrency = num_test_threads();
    std::condition_variable cv;
    std::mutex mtx;
    std::vector<std::thread> threads;
    utils::SpinLock ioLock;
    SharedMutexType rwLock;

    threads.resize(concurrency);

    for (unsigned testId = 0; testId < numTests; ++testId)
    {
        std::atomic_uint numThreadsRunning{concurrency};

        for (unsigned i = 0; i < concurrency; ++i)
        {
            if (i & 1)
            {
                threads[i] = std::thread{
                    exclusive_lock_func<SharedMutexType>,
                    std::ref(rwLock),
                    std::ref(ioLock),
                    std::ref(cv),
                    std::ref(numThreadsRunning)
                };
            }
            else
            {
                threads[i] = std::thread{
                    shared_lock_func<SharedMutexType>,
                    std::ref(rwLock),
                    std::ref(ioLock),
                    std::ref(cv),
                    std::ref(numThreadsRunning)
                };
            }
        }

        const system_time_point&& startTime = std::chrono::system_clock::now();

        std::unique_lock<std::mutex> lk{mtx};
        while (numThreadsRunning.load(std::memory_order_relaxed) > 0)
        {
            cv.wait(lk);
        }

        runTime += system_duration{std::chrono::system_clock::now() - startTime};

        for (std::thread& t : threads)
        {
            t.join();
        }
    }

    std::cout << "\tDone." << std::endl;
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
    const system_duration&& mutexRunTime = run_mtx_test<std::mutex>("std::mutex", numTests);
    const system_duration&& futexRunTime = run_mtx_test<utils::Futex>("utils::Futex", numTests);
    const system_duration&& spinlockRunTime = run_mtx_test<utils::SpinLock>("utils::SpinLock", numTests);

    /*
     * Shared-lock benchmarks
     */
    const system_duration&& rwMutex2RunTime = run_rw_test<utils::SharedMutex2>("SharedMutex2", numTests);
    const system_duration&& rwMutexRunTime = run_rw_test<utils::SharedMutex>("SharedMutex", numTests);
    const system_duration&& rwFutexRunTime = run_rw_test<utils::SharedFutex>("SharedFutex", numTests);
    const system_duration&& rwSpinRunTime = run_rw_test<utils::SharedSpinLock>("SharedSpinLock", numTests);
    //const system_duration&& fairMutexRunTime = run_rw_test<utils::FairRWMutex>("FairRWMutex", numTests);
    //const system_duration&& fairFutexRunTime = run_rw_test<utils::FairRWFutex>("FairRWFutex", numTests);
    //const system_duration&& fairSpinlockRunTime = run_rw_test<utils::FairRWSpinLock>("FairRWSpinLock", numTests);

    std::cout
        << "Results:"
        << "\n\tstd::mutex Time:      " << mutexRunTime.count() << "ms"
        << "\n\tutils::Futex Time:    " << futexRunTime.count() << "ms"
        << "\n\tutils::SpinLock Time: " << spinlockRunTime.count() << "ms"
        << "\n\tShared Mutex2 Time:   " << rwMutex2RunTime.count() << "ms"
        << "\n\tShared Mutex Time:    " << rwMutexRunTime.count() << "ms"
        << "\n\tShared Futex Time:    " << rwFutexRunTime.count() << "ms"
        << "\n\tShared SpinLock Time: " << rwSpinRunTime.count() << "ms"
        //<< "\n\tFair Mutex Time:      " << fairMutexRunTime.count() << "ms"
        //<< "\n\tFair Futex Time:      " << fairFutexRunTime.count() << "ms"
        //<< "\n\tFair SpinLock Time:   " << fairSpinlockRunTime.count() << "ms"
        << std::endl;

    return 0;
}
