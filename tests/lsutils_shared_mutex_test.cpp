
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



inline unsigned num_test_threads() noexcept
{
    const unsigned concurrency = (unsigned)std::thread::hardware_concurrency();

    if (concurrency <= 2)
    {
        return 2;
    }

    // ensure an even number of test threads
    return concurrency - (concurrency % 2);
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

    std::cout
        << "Results:"
        << "\n\tShared Mutex Time: " << mutexRunTime.count() << "ms"
        << "\n\tShared Futex Time: " << futexRunTime.count() << "ms"
        << std::endl;

    return 0;
}
