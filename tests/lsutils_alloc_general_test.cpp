
#include <iostream>
#include <memory> // std::nothrow
#include <mutex>
#include <thread>
#include <vector>

#include "lightsky/utils/GeneralAllocator.hpp"
#include "lightsky/utils/SpinLock.hpp"

namespace utils = ls::utils;



int test_single_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 512u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;

    // test allocator of 64 bytes in a 256-byte container
    //utils::MallocMemorySource mallocSrc{};
    utils::SystemAllocator mallocSrc{};
    utils::ConstrainedAllocator<alloc_table_size> memLimiter{mallocSrc};
    utils::GeneralAllocator<block_size, alloc_table_size> testAllocator{memLimiter, alloc_table_size};
    void** allocations = new void*[max_allocations];
    void* p = nullptr;
    void* last = nullptr;
    (void)last;

    for (unsigned i = 0; i < max_allocations; ++i)
    {
        allocations[i] = nullptr;
    }

    for (unsigned testRuns = 0; testRuns < 8; ++testRuns)
    {
        for (unsigned i = 0; i <= max_allocations+1; ++i)
        {
            p = testAllocator.allocate();
            if (!p && i < max_allocations)
            {
                std::cerr << "Error: ran out of single memory blocks at allocation #" << i << std::endl;
                return -1;
            }

            LS_ASSERT(p == nullptr || last < p);
            last = p;

            //std::cout << "Allocated chunk " << i << ": " << p << std::endl;

            if (i >= max_allocations)
            {
                if (p != nullptr)
                {
                    std::cerr << "Error: test pointer 4 provided unknown memory!" << std::endl;
                    return -2;
                }
            }
            else
            {
                allocations[i] = p;
            }
        }

        /*
        p = allocations[max_allocations/2];
        testAllocator.free(allocations[max_allocations/2]);
        allocations[max_allocations/2] = testAllocator.allocate();
        LS_ASSERT(allocations[max_allocations/2] != nullptr);
        LS_ASSERT(allocations[max_allocations/2] == p);
        */

        // we should be able to retrieve and reallocate
        testAllocator.free(allocations[2]);
        allocations[2] = nullptr;

        p = testAllocator.allocate();
        if (!p)
        {
            std::cerr << "Error: Unable to reallocate block 2!" << std::endl;
            return -3;
        }
        else
        {
            allocations[2] = p;
        }

        p = testAllocator.allocate();
        if (p)
        {
            std::cerr << "Error: Allocated too many blocks!" << std::endl;
            return -4;
        }

        // free all chunks and try again
        for (unsigned i = 0; i < max_allocations; ++i)
        {
            testAllocator.free(allocations[i]);
            allocations[i] = nullptr;
        }
    }

    delete [] allocations;
    return 0;
}



template <unsigned block_size>
  class MallocMemorySource2 final : virtual public utils::ThreadSafeAllocator
{
  private:
    static constexpr size_type header_size = block_size;

  public:
    virtual ~MallocMemorySource2() noexcept override {}

    MallocMemorySource2(utils::MemorySource& memorySource) noexcept :
        ThreadSafeAllocator{static_cast<utils::MemorySource&>(memorySource)}
    {}

    MallocMemorySource2(const MallocMemorySource2& src) noexcept :
        ThreadSafeAllocator{src}
    {}

    MallocMemorySource2(MallocMemorySource2&& src) noexcept :
        ThreadSafeAllocator{std::move(src)}
    {
    }

    MallocMemorySource2& operator=(const MallocMemorySource2&) noexcept = delete;
    MallocMemorySource2& operator=(MallocMemorySource2&&) noexcept = default;

    virtual void* allocate() noexcept override
    {
        return this->allocate(block_size);
    }

    virtual void* allocate(size_type numBytes) noexcept override
    {
        if (!numBytes)
        {
            return nullptr;
        }

        numBytes += block_size - (numBytes % block_size);
        return ThreadSafeAllocator::allocate(numBytes);
    }

    virtual void free(void* pData) noexcept override
    {
        this->free(pData, block_size);
    }

    virtual void free(void* pData, size_type numBytes) noexcept override
    {
        if (!pData || !numBytes)
        {
            return;
        }

        numBytes += block_size - (numBytes % block_size);
        ThreadSafeAllocator::free(pData, numBytes);
    }
};



int test_array_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 16u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;
    //constexpr unsigned mid_allocation = (max_allocations / 3u - 1u) / 2u;
    //utils::MallocMemorySource mallocSrc{};
    utils::SystemAllocator mallocSrc{};

    //constexpr unsigned allocSizeOffset = sizeof(utils::ThreadedMemoryCache<utils::GeneralAllocator<block_size, alloc_table_size>>::AllocatorList);
    utils::ConstrainedAllocator<alloc_table_size> memLimiter{mallocSrc};
    utils::GeneralAllocator<block_size, alloc_table_size> testAllocator{memLimiter};
    //MallocMemorySource2<block_size> testAllocator{memLimiter};

    // test allocator of 64 bytes in a 256-byte container
    //utils::ConstrainedAllocator<alloc_table_size+block_size*5> memLimiter{mallocSrc};
    //utils::GeneralAllocator<block_size, alloc_table_size+block_size*5> internalAllocator{memLimiter};
    //utils::AtomicAllocator atomicAllocator{internalAllocator};
    //utils::ThreadedAllocator<utils::GeneralAllocator<block_size, alloc_table_size>> testAllocator{atomicAllocator};

    //utils::ConstrainedAllocator<alloc_table_size+block_size*2> memLimiter{mallocSrc};
    //MallocMemorySource2<block_size> atomicAllocator{memLimiter};
    //utils::ThreadedAllocator<utils::Allocator> testAllocator{atomicAllocator};

    void** allocations = new void*[max_allocations];
    void* p = nullptr;

    for (unsigned i = 0; i < max_allocations; ++i)
    {
        allocations[i] = nullptr;
    }

    for (unsigned testRuns = 0; testRuns < 5; ++testRuns)
    {
        for (unsigned i = 0; i < max_allocations/3; ++i)
        {
            p = testAllocator.allocate(block_size * 2);
            if (!p && i < max_allocations)
            {
                std::cerr << "Error: ran out of memory at block " << i << '/' << (max_allocations/3) << std::endl;
                return -1;
            }

            //std::cout << "Allocated chunk " << i << ": " << p << std::endl;

            if (i >= max_allocations)
            {
                if (p != nullptr)
                {
                    std::cerr << "Error: test pointer 4 provided unknown memory!" << std::endl;
                    return -2;
                }
            }
            else
            {
                allocations[i] = p;
            }
        }

        /*
        p = allocations[mid_allocation];
        testAllocator.free(allocations[mid_allocation], block_size * 2);
        allocations[mid_allocation] = testAllocator.allocate(block_size * 2);
        LS_ASSERT(allocations[mid_allocation] != nullptr);
        LS_ASSERT(allocations[mid_allocation] == p);
        */

        // we should be able to retrieve and reallocate
        //testAllocator.free(allocations[2], block_size * 2);
        testAllocator.free(allocations[2], block_size * 2);
        allocations[2] = nullptr;

        p = testAllocator.allocate(block_size * 2);
        if (!p)
        {
            std::cerr << "Error: Unable to reallocate block 2!" << std::endl;
            return -3;
        }
        else
        {
            allocations[2] = p;
        }

        p = testAllocator.allocate(block_size * 2);
        if (p)
        {
            std::cerr << "Error: Allocated too many chunks!" << std::endl;
            return -4;
        }

        // free all chunks and try again
        for (unsigned i = 0; i < max_allocations; ++i)
        {
            testAllocator.free(allocations[i], block_size * 2);
            allocations[i] = nullptr;
        }
    }

    delete [] allocations;
    return 0;
}



int main()
{
    int ret = 0;

    ret = test_single_allocations();
    if (ret != 0)
    {
        return ret;
    }

    ret = test_array_allocations();
    if (ret != 0)
    {
        return ret;
    }

    /*
    std::cout << std::endl;
    std::cout << "Testing a threaded malloc cache:" << std::endl;
    utils::MallocMemorySource mallocSrc;
    utils::AtomicAllocator atomicAllocator{mallocSrc};
    utils::ThreadedAllocator<utils::Allocator> threadedAllocator{atomicAllocator};
    std::mutex lock;

    const auto allocFunc = [&]()->void {
        void* pData = threadedAllocator.allocate(32);

        lock.lock();
        std::cout << "Allocated 32 bytes from thread " << std::this_thread::get_id() << ": " << pData << std::endl;
        lock.unlock();

        threadedAllocator.free(pData, 32);

        lock.lock();
        std::cout << "Free'd 32 bytes from thread " << std::this_thread::get_id() << std::endl;
        lock.unlock();
    };

    std::thread t0{allocFunc};
    std::thread t1{allocFunc};
    std::thread t2{allocFunc};

    t0.join();
    t1.join();
    t2.join();
    */

    return 0;
}