
#include <cassert>
#include <chrono> // std::seconds
#include <cstddef> // ptrdiff_t
#include <iostream>
#include <memory>

#include "lightsky/utils/WorkerThread.hpp"

using ls::utils::WorkerThread;
using ls::utils::WorkerThreadGroup;



struct SampleTask
{
    volatile int x = rand() % 2 + 1;

    void operator()() noexcept
    {
        std::cout << " -- This task is running on external thread " << std::this_thread::get_id() << " for " << x << " seconds." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(x));
    }
};



void test_single_worker()
{
    std::cout << "Testing a single worker" << std::endl;

    WorkerThread<SampleTask> thread{};

    thread.push(SampleTask());
    thread.push(SampleTask());
    thread.push(SampleTask());
    thread.push(SampleTask());
    thread.flush();

    while (!thread.ready())
    {
        std::cout << "Waiting for the worker thread to finish..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        // push tasks while waiting for the test to complete
        thread.push(SampleTask());
    }

    thread.flush();

    std::cout << "Thread ready state: " << thread.ready() << std::endl;
    while (!thread.ready())
    {
        std::cout << "\tWaiting for tasks to finish..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    WorkerThread<SampleTask> thread2;
    thread2 = std::move(thread);

    thread2.flush();
    std::cout << "Done. Thread ready state: " << thread2.ready() << std::endl;
}



void test_grouped_workers()
{
    constexpr unsigned numWorkers = 11;
    std::cout << "Testing " << numWorkers << " grouped workers." << std::endl;

    WorkerThreadGroup<SampleTask> thread{};
    thread.concurrency(numWorkers);

    thread.push(SampleTask());
    //thread.push(SampleTask());
    //thread.push(SampleTask());
    //thread.push(SampleTask());
    thread.flush();

    do
    {
        std::cout << "Waiting for the worker thread to finish..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        // push tasks while waiting for the test to complete
        thread.push(SampleTask());
    }
    while (!thread.ready());

    thread.flush();

    std::cout << "Thread ready state: " << thread.ready() << std::endl;
    while (!thread.ready())
    {
        std::cout << "\tWaiting for tasks to finish..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    {
        WorkerThreadGroup<SampleTask> thread2;
        thread2 = std::move(thread);
        thread2.flush();

        std::cout << "Done. Thread ready state: " << thread2.ready() << std::endl;
    }
}



int main()
{
    srand(time(nullptr));
    std::cout
        << "Data Alignment:"
        << "\n\tMutex:       " << sizeof(std::mutex)
        << "\n\tSpinLock:    " << sizeof(ls::utils::SpinLock)
        << "\n\tWorker (ST): " << sizeof(ls::utils::DefaultWorker)
        << "\n\tWorker (MT): " << sizeof(ls::utils::DefaultWorkerThread)
        << std::endl;

    //test_single_worker();
    test_grouped_workers();

    return 0;
}
