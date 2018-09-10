
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

    WorkerThread<SampleTask> thread{};
    std::vector<SampleTask> results;
    std::vector<SampleTask>::size_type numExtraResults = 0;

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
        ++numExtraResults;
    }

    thread.flush();
    results = std::move(thread.outputs());
    assert(results.size() == 4);

    std::cout << "Received " << results.size() << " result tasks." << std::endl;
    std::cout << "Thread ready state: " << thread.ready() << std::endl;
    while (!thread.ready())
    {
        std::cout << "Waiting " << numExtraResults << " more tasks to finish..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    WorkerThread<SampleTask> thread2;
    thread2 = std::move(thread);

    thread2.flush();
    results = std::move(thread2.outputs());
    assert(results.size() == numExtraResults);

    std::cout << "Received " << results.size() << " more results." << std::endl;
    std::cout << "Thread ready state: " << thread2.ready() << std::endl;

    return 0;
}
