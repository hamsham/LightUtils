
#include <cassert>
#include <chrono> // std::seconds
#include <cstddef> // ptrdiff_t
#include <iostream>
#include <memory>

#include "lightsky/utils/WorkerThread.hpp"

using ls::utils::WorkerThread;



struct SampleTask
{
    volatile int x = rand() % 2 + 1;

    void operator()() noexcept
    {
        std::cout << " -- This task is running on another thread for " << x << " seconds." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(x));
    }
};



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
        std::cout << "Waiting for tasks to finish..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    WorkerThread<SampleTask> thread2;
    thread2 = std::move(thread);

    thread2.flush();
    std::cout << "Thread ready state: " << thread2.ready() << std::endl;

    return 0;
}
