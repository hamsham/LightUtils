
#include <iostream>
#include <memory> // std::nothrow
#include <mutex>
#include <thread>
#include <vector>

#include "lightsky/utils/GeneralAllocator.hpp"
#include "lightsky/utils/SpinLock.hpp"

namespace utils = ls::utils;



template <unsigned maxNumBytes>
class LimitedMemoryAllocator final : public utils::ThreadSafeAllocator
{
  private:
    size_type mMaxAllocSize;
    size_type mBytesAllocated;

  public:
    virtual ~LimitedMemoryAllocator() noexcept override {}

    LimitedMemoryAllocator(utils::MallocMemorySource& memorySource) noexcept :
        ThreadSafeAllocator{static_cast<utils::MemorySource&>(memorySource)},
        mMaxAllocSize{maxNumBytes},
        mBytesAllocated{0}
    {}

    LimitedMemoryAllocator(const LimitedMemoryAllocator& src) noexcept :
        ThreadSafeAllocator{src},
        mMaxAllocSize{src.mMaxAllocSize},
        mBytesAllocated{src.mBytesAllocated}
    {}

    LimitedMemoryAllocator(LimitedMemoryAllocator&& src) noexcept :
        ThreadSafeAllocator{std::move(src)},
        mMaxAllocSize{src.mMaxAllocSize},
        mBytesAllocated{src.mBytesAllocated}
    {
        src.mMaxAllocSize = 0;
        src.mBytesAllocated = 0;
    }

    LimitedMemoryAllocator& operator=(const LimitedMemoryAllocator&) noexcept = default;
    LimitedMemoryAllocator& operator=(LimitedMemoryAllocator&&) noexcept = default;

    virtual void* allocate() noexcept override
    {
        LS_ASSERT(false);
        return nullptr;
    }

    virtual void* allocate(size_type numBytes) noexcept override
    {
        if (mBytesAllocated+numBytes > mMaxAllocSize)
        {
            return nullptr;
        }

        void* pData = this->memory_source().allocate(numBytes);
        if (pData)
        {
            mBytesAllocated += numBytes;
        }

        return  pData;
    }

    virtual void free(void* pData) noexcept override
    {
        (void)pData;
        LS_ASSERT(false);
    }

    virtual void free(void* pData, size_type numBytes) noexcept override
    {
        if (pData && numBytes)
        {
            mBytesAllocated -= numBytes;
            this->memory_source().free(pData, numBytes);
        }
    }
};



int test_single_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 512u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;

    // test allocator of 64 bytes in a 256-byte container
    utils::MallocMemorySource mallocSrc{};
    LimitedMemoryAllocator<alloc_table_size> memLimiter{mallocSrc};
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



template <unsigned block_size, unsigned maxNumBytes>
class MallocMemorySource2 final : public utils::ThreadSafeAllocator
{
  private:
    static constexpr size_type header_size = block_size;
    size_type mMaxAllocSize;
    size_type mBytesAllocated;

  public:
    virtual ~MallocMemorySource2() noexcept override {}

    MallocMemorySource2(utils::MallocMemorySource& memorySource) noexcept :
        ThreadSafeAllocator{static_cast<utils::MemorySource&>(memorySource)},
        mMaxAllocSize{maxNumBytes},
        mBytesAllocated{0}
    {}

    MallocMemorySource2(const MallocMemorySource2& src) noexcept :
        ThreadSafeAllocator{src},
        mMaxAllocSize{src.mMaxAllocSize},
        mBytesAllocated{src.mBytesAllocated}
    {}

    MallocMemorySource2(MallocMemorySource2&& src) noexcept :
        ThreadSafeAllocator{std::move(src)},
        mMaxAllocSize{src.mMaxAllocSize},
        mBytesAllocated{src.mBytesAllocated}
    {
        src.mMaxAllocSize = 0;
        src.mBytesAllocated = 0;
    }

    MallocMemorySource2& operator=(const MallocMemorySource2&) noexcept = default;
    MallocMemorySource2& operator=(MallocMemorySource2&&) noexcept = default;

    virtual void* allocate() noexcept override
    {
        return this->allocate(block_size);
    }

    virtual void* allocate(size_type numBytes) noexcept override
    {
        numBytes += header_size;
        const size_type blocksFreed = (numBytes / block_size) + ((numBytes % block_size) ? 1 : 0);
        const size_type n = blocksFreed * block_size;

        if (mBytesAllocated+n > mMaxAllocSize)
        {
            return nullptr;
        }

        mBytesAllocated += n;
        return this->memory_source().allocate(n);
    }

    virtual void free(void* pData) noexcept override
    {
        this->free(pData, block_size);
    }

    virtual void free(void* pData, size_type numBytes) noexcept override
    {
        numBytes += header_size;
        const size_type blocksFreed = (numBytes / block_size) + ((numBytes % block_size) ? 1 : 0);
        const size_type n = blocksFreed * block_size;

        if (pData)
        {
            mBytesAllocated -= n;
            this->memory_source().free(pData, numBytes);
        }
    }
};



int test_array_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 16u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;
    //constexpr unsigned mid_allocation = (max_allocations / 3u - 1u) / 2u;
    utils::MallocMemorySource mallocSrc{};

    constexpr unsigned allocSizeOffset = sizeof(utils::ThreadedMemoryCache<utils::GeneralAllocator<block_size, alloc_table_size>>::AllocatorList);
    LimitedMemoryAllocator<alloc_table_size+allocSizeOffset> memLimiter{mallocSrc};
    //utils::GeneralAllocator<block_size, alloc_table_size> testAllocator{memLimiter, alloc_table_size};

    // test allocator of 64 bytes in a 256-byte container
    //utils::GeneralAllocator<block_size> internalAllocator{mallocSrc, alloc_table_size};
    //utils::AtomicAllocator atomicAllocator{internalAllocator};
    //MallocMemorySource2<block_size, alloc_table_size+block_size*2> atomicAllocator{mallocSrc};
    //utils::ThreadedAllocator<utils::Allocator> testAllocator{atomicAllocator};

    utils::ThreadedAllocator<utils::GeneralAllocator<block_size, alloc_table_size>> testAllocator{memLimiter};

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

    return 0;
}