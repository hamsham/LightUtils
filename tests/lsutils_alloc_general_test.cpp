
#include <cstring>
#include <iostream>
#include <memory> // std::nothrow
#include <thread>
#include <vector>

#include "lightsky/utils/GeneralAllocator.hpp"
#include "lightsky/utils/SpinLock.hpp"
#include "lightsky/utils/Time.hpp"

namespace utils = ls::utils;

#define TEST_MALLOC_MEM_SRC   true
#define TEST_MALLOC_ALLOCATOR true



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

    MallocMemorySource2(const MallocMemorySource2& src) noexcept = delete;

    MallocMemorySource2(MallocMemorySource2&& src) noexcept :
        ThreadSafeAllocator{std::move(src)}
    {
    }

    MallocMemorySource2& operator=(const MallocMemorySource2&) noexcept = delete;

    MallocMemorySource2& operator=(MallocMemorySource2&& src) noexcept
    {
        if (this != &src)
        {
            ThreadSafeAllocator::operator=(std::move(src));
        }

        return *this;
    }

    virtual void* allocate(size_type numBytes) noexcept override
    {
        if (!numBytes)
        {
            return nullptr;
        }

        size_type rem = numBytes % block_size;
        numBytes += block_size - (rem ? rem : block_size);
        return ThreadSafeAllocator::allocate(numBytes);
    }

    virtual void free(void* pData) noexcept override
    {
        ThreadSafeAllocator::free(pData, block_size);
    }

    virtual void free(void* pData, size_type numBytes) noexcept override
    {
        if (!pData || !numBytes)
        {
            return;
        }

        size_type rem = numBytes % block_size;
        numBytes += block_size - (rem ? rem : block_size);
        ThreadSafeAllocator::free(pData, numBytes);
    }
};



int test_single_allocations()
{
    // test allocator of 512 bytes in a 256-byte container
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 512u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;

    // test allocator of 64 bytes in a 256-byte container
    #if TEST_MALLOC_MEM_SRC
        utils::MallocMemorySource mallocSrc{};
    #else
        utils::SystemMemorySource mallocSrc{};
    #endif

    utils::ConstrainedAllocator<alloc_table_size> memLimiter{mallocSrc};

    #if TEST_MALLOC_ALLOCATOR
        MallocMemorySource2<block_size+32u> testAllocator{memLimiter};
    #else
        utils::GeneralAllocator<alloc_table_size> testAllocator{memLimiter};
    #endif

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
            p = testAllocator.allocate(block_size);
            if (!p && i < max_allocations/2)
            {
                std::cerr << "Error: ran out of single memory blocks at allocation #" << i << '/' << max_allocations << std::endl;
                return -1;
            }

            #if 0//TEST_MALLOC_ALLOCATOR == false
                LS_ASSERT(p == nullptr || last < p);
            #endif

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
        allocations[max_allocations/2] = testAllocator.allocate(block_size);
        LS_ASSERT(allocations[max_allocations/2] != nullptr);
        LS_ASSERT(allocations[max_allocations/2] == p);
        */

        // we should be able to retrieve and reallocate
        testAllocator.free(allocations[2]);
        allocations[2] = nullptr;

        p = testAllocator.allocate(block_size);
        if (!p)
        {
            std::cerr << "Error: Unable to reallocate block 2!" << std::endl;
            return -3;
        }
        else
        {
            allocations[2] = p;
        }

        p = testAllocator.allocate(block_size);
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



int test_array_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u*3u;
    constexpr unsigned block_size = 32u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;
    //constexpr unsigned mid_allocation = (max_allocations / 3u - 1u) / 2u;

    #if TEST_MALLOC_MEM_SRC
        utils::MallocMemorySource mallocSrc{};
    #else
        utils::SystemMemorySource mallocSrc{};
    #endif

    utils::ConstrainedAllocator<alloc_table_size> memLimiter{mallocSrc};

    #if TEST_MALLOC_ALLOCATOR
        MallocMemorySource2<block_size> testAllocator{memLimiter};
    #else
        utils::GeneralAllocator<alloc_table_size> testAllocator{memLimiter};
    #endif

    void** allocations = new void*[max_allocations];
    void* p = nullptr;

    for (unsigned i = 0; i < max_allocations; ++i)
    {
        allocations[i] = nullptr;
    }

    for (unsigned testRuns = 0; testRuns < 5; ++testRuns)
    {
        for (unsigned i = 0; i < max_allocations/2; ++i)
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

        p = testAllocator.allocate(block_size);
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



static constexpr unsigned THREAD_ALLOC_TABLE_SIZE = 1024u*1024u*1024u;
static constexpr unsigned THREAD_ALLOC_BLOCK_SIZE = 32u;

#if TEST_MALLOC_ALLOCATOR
inline LS_INLINE MallocMemorySource2<THREAD_ALLOC_BLOCK_SIZE>& get_allocator() noexcept
#else
inline LS_INLINE utils::GeneralAllocator<THREAD_ALLOC_TABLE_SIZE, false>& get_allocator() noexcept
#endif
{
    // test allocator of 64 bytes in a 256-byte container
    #if TEST_MALLOC_MEM_SRC
        static utils::MallocMemorySource mallocSrc{};
    #else
        static utils::SystemMemorySource mallocSrc{};
    #endif

    #if TEST_MALLOC_ALLOCATOR
        static MallocMemorySource2<THREAD_ALLOC_BLOCK_SIZE> testAllocator{mallocSrc};
    #else
        static utils::GeneralAllocator<THREAD_ALLOC_TABLE_SIZE, true> internalAllocator{mallocSrc};
        static utils::AtomicAllocator atomicAllocator{internalAllocator};
        static thread_local utils::GeneralAllocator<THREAD_ALLOC_TABLE_SIZE, false> testAllocator{atomicAllocator};
    #endif

    return testAllocator;
}



int test_threaded_allocations()
{
    // test allocator of 32 bytes in a 256-byte container
    constexpr unsigned max_allocations = THREAD_ALLOC_TABLE_SIZE / THREAD_ALLOC_BLOCK_SIZE;

    auto threadFunc = [&]()->void
    {
        void** allocations = new void* [max_allocations];
        void* p = nullptr;

        for (unsigned i = 0; i < max_allocations; ++i)
        {
            allocations[i] = nullptr;
        }

        for (unsigned testRuns = 0; testRuns < 8; ++testRuns)
        {
            for (unsigned i = 0; i < max_allocations; ++i)
            {
                p = get_allocator().allocate(THREAD_ALLOC_BLOCK_SIZE);
                if (!p)
                {
                    std::cerr << "Error: ran out of single memory blocks at allocation #" << i << std::endl;
                    return;
                }
                else
                {
                    std::memset(p, 0xD3ADB33F, THREAD_ALLOC_BLOCK_SIZE/2);
                }

                allocations[i] = p;
            }

            // free all chunks and try again
            for (unsigned i = 0; i < max_allocations; ++i)
            {
                get_allocator().free(allocations[i]);
                allocations[i] = nullptr;
            }
        }

        delete[] allocations;
    };

    std::thread t0{threadFunc};
    std::thread t1{threadFunc};
    std::thread t2{threadFunc};
    threadFunc();

    t0.join();
    t1.join();
    t2.join();

    return 0;
}



int main()
{
    int ret = 0;
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;

    std::cout << "Running allocator benchmark..." << std::endl;
    ticks.start();

    #if 0
        ret = test_single_allocations();
        if (ret != 0)
        {
            return ret;
        }
    #endif

    #if 0
        ret = test_array_allocations();
        if (ret != 0)
        {
            return ret;
        }
    #endif

    #if 1
        ret = test_threaded_allocations();
    #endif

    ticks.tick();
    std::cout << "\tDone." << std::endl;

    std::cout << "Allocator time: " << ticks.tick_time().count() << "ms" << std::endl;

    return ret;
}