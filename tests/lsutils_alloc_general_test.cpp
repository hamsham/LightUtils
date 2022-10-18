
#include <iostream>
#include <memory> // std::nothrow
#include <mutex>
#include <thread>

#include "lightsky/setup/Types.h"

#include "lightsky/utils/GeneralAllocator.hpp"
#include "lightsky/utils/SpinLock.hpp"



class MemorySource
{
  public:
    virtual ~MemorySource() noexcept = 0;

    MemorySource() noexcept {}
    MemorySource(const MemorySource&) noexcept {}
    MemorySource(MemorySource&&) noexcept {}

    MemorySource& operator=(const MemorySource&) noexcept = default;
    MemorySource& operator=(MemorySource&&) noexcept = default;

    virtual void* allocate(size_t numBytes) noexcept = 0;
    virtual void free(void* pData, size_t numBytes) noexcept = 0;
};

MemorySource::~MemorySource() noexcept
{
}



class MallocMemorySource final : public MemorySource
{
  public:
    virtual ~MallocMemorySource() noexcept override {}

    MallocMemorySource() noexcept :
        MemorySource{}
    {}

    MallocMemorySource(const MallocMemorySource& src) noexcept :
        MemorySource{src}
    {}

    MallocMemorySource(MallocMemorySource&& src) noexcept :
        MemorySource{std::move(src)}
    {}

    MallocMemorySource& operator=(const MallocMemorySource&) noexcept = default;
    MallocMemorySource& operator=(MallocMemorySource&&) noexcept = default;

    virtual void* allocate(size_t numBytes) noexcept override
    {
        return std::malloc(numBytes);
    }

    virtual void free(void* pData, size_t numBytes) noexcept override
    {
        (void)numBytes;
        std::free(pData);
    }
};



class IAllocator : public MemorySource
{
  protected:
    virtual const MemorySource& memory_source() const noexcept = 0;

    virtual MemorySource& memory_source() noexcept = 0;

  public:
    virtual ~IAllocator() noexcept = 0;

    virtual void* allocate(size_t n) noexcept
    {
        return this->memory_source().allocate(n);
    }

    virtual void free(void* p, size_t n) noexcept
    {
        this->memory_source().free(p, n);
    }
};

IAllocator::~IAllocator() noexcept
{
}



class Allocator : public IAllocator
{
  private:
    MemorySource* mMemSource;

  protected:
    const MemorySource& memory_source() const noexcept override
    {
        return *mMemSource;
    }

    MemorySource& memory_source() noexcept override
    {
        return *mMemSource;
    }

  public:
    virtual ~Allocator() noexcept override {}

    Allocator() noexcept = delete;

    Allocator(MemorySource& src) noexcept :
        mMemSource{&src}
    {}

    Allocator(const Allocator&) = delete;

    Allocator(Allocator&& allocator) noexcept :
        IAllocator{allocator},
        mMemSource{allocator.mMemSource}
    {
        allocator.mMemSource = nullptr;
    }

    Allocator& operator=(const Allocator& allocator) noexcept = delete;

    Allocator& operator=(Allocator&& allocator) noexcept
    {
        if (&allocator != this)
        {
            IAllocator::operator=(std::move(allocator));

            mMemSource = allocator.mMemSource;
            allocator.mMemSource = nullptr;
        }

        return *this;
    }
};



class AtomicAllocator : public Allocator
{
  private:
    ls::utils::SpinLock mLock;

  public:
    virtual ~AtomicAllocator() noexcept override {}

    AtomicAllocator() noexcept = delete;

    AtomicAllocator(MemorySource& src) noexcept :
        Allocator{src},
        mLock{}
    {}

    AtomicAllocator(const AtomicAllocator&) = delete;

    AtomicAllocator(AtomicAllocator&& allocator) noexcept :
        Allocator{std::move(allocator)},
        mLock{}
    {
    }

    AtomicAllocator& operator=(const AtomicAllocator& allocator) noexcept = delete;

    AtomicAllocator& operator=(AtomicAllocator&& allocator) noexcept
    {
        if (&allocator != this)
        {
            mLock.lock();
            Allocator::operator=(std::move(allocator));
            mLock.unlock();
        }

        return *this;
    }

    virtual void* allocate(size_t n) noexcept override
    {
        mLock.lock();
        void* const pMem = Allocator::allocate(n);
        mLock.unlock();

        return pMem;
    }

    virtual void free(void* p, size_t n) noexcept override
    {
        Allocator::free(p, n);
    }
};



template <typename ThreadedCacheType>
class ThreadedMemoryCache
{
  private:
    // There may be more than one allocator per-thread. Here we maintain a list
    // of them, and their associated per-thread caches.
    struct AllocatorList
    {
        AtomicAllocator* mAllocator;
        AllocatorList* pNext;
        ThreadedCacheType mMemCache;
    };

    AllocatorList* mAllocators;

  public:
    ~ThreadedMemoryCache() noexcept
    {
        // All allocators should have been freed by now
        LS_ASSERT(mAllocators == nullptr);
    }

    // When an allocator goes out of scope, we remove it from the allocator
    // list. There's absolutely no need to keep it's references around.
    void remove_allocator(AtomicAllocator* allocator) noexcept
    {
        AllocatorList* pPrev = nullptr;
        AllocatorList* pIter = mAllocators;

        while (pIter != nullptr)
        {
            if (pIter->mAllocator == allocator)
            {
                if (pPrev)
                {
                    pPrev->pNext = pIter->pNext;
                }
                else
                {
                    mAllocators = pIter->pNext;
                }

                pIter->mAllocator->free(pIter, sizeof(AllocatorList));
                break;
            }

            pPrev = pIter;
            pIter = pIter->pNext;
        }
    }

    void* allocate(AtomicAllocator* allocator, size_t n) noexcept
    {
        static_assert(ls::setup::IsBaseOf<IAllocator, ThreadedCacheType>::value, "Template allocator type does not implement the IAllocator interface.");
        LS_ASSERT(allocator != nullptr);

        AllocatorList* iter = nullptr;
        for (AllocatorList* pAllocators = mAllocators; pAllocators != nullptr; pAllocators = pAllocators->pNext)
        {
            if (pAllocators->mAllocator == allocator)
            {
                return pAllocators->mMemCache.allocate(n);
            }

            iter = pAllocators;
        }

        // No local allocator exists which corresponds to the current thread,
        // create one using the primary allocator
        void* const pCacheLocation = allocator->allocate(sizeof(AllocatorList));
        if (!pCacheLocation)
        {
            return nullptr;
        }

        // The primary allocator will also need to allocate its own list node
        // for bookkeeping... nobody else will.
        MemorySource* const pMemSrc = static_cast<MemorySource*>(allocator);

        AllocatorList* const pListEntry = new (pCacheLocation) AllocatorList{
            allocator,
            nullptr,
            ThreadedCacheType{*pMemSrc}
        };

        if (iter)
        {
            iter->pNext = pListEntry;
        }
        else
        {
            mAllocators = pListEntry;
        }

        return pListEntry->mMemCache.allocate(n);
    }

    void free(AtomicAllocator* allocator, void* p, size_t n) noexcept
    {
        LS_ASSERT(allocator != nullptr);

        for (AllocatorList* pIter = mAllocators; pIter != nullptr; pIter = pIter->pNext)
        {
            if (pIter->mAllocator == allocator)
            {
                pIter->mMemCache.free(p, n);
                return;
            }
        }
    }
};



template <typename ThreadedCacheType>
class ThreadedAllocator final : public Allocator
{
    static_assert(ls::setup::IsBaseOf<IAllocator, ThreadedCacheType>::value, "Template allocator type does not implement the IAllocator interface.");

  private:
    static thread_local ThreadedMemoryCache<ThreadedCacheType> sThreadCache;

  public:
    virtual ~ThreadedAllocator() noexcept override
    {
        AtomicAllocator& memSrc = static_cast<AtomicAllocator&>(this->memory_source());
        sThreadCache.remove_allocator(&memSrc);
    }

    ThreadedAllocator() noexcept = delete;

    ThreadedAllocator(AtomicAllocator& src) noexcept :
        Allocator{static_cast<MemorySource&>(src)}
    {
    }

    ThreadedAllocator(const ThreadedAllocator&) = delete;

    ThreadedAllocator(ThreadedAllocator&& allocator) noexcept :
        AtomicAllocator{std::move(allocator)}
    {
    }

    ThreadedAllocator& operator=(const ThreadedAllocator&) = delete;

    ThreadedAllocator& operator=(ThreadedAllocator&& allocator) noexcept
    {
        if (this != allocator)
        {
            Allocator::operator=(std::move(allocator));
        }

        return *this;
    }

    virtual void* allocate(size_t n) noexcept override
    {
        AtomicAllocator& memSrc = static_cast<AtomicAllocator&>(this->memory_source());
        return sThreadCache.allocate(&memSrc, n);
    }

    virtual void free(void* p, size_t n) noexcept override
    {
        AtomicAllocator& memSrc = static_cast<AtomicAllocator&>(this->memory_source());
        sThreadCache.free(&memSrc, p, n);
    }
};

template <typename ThreadedCacheType>
thread_local ThreadedMemoryCache<ThreadedCacheType> ThreadedAllocator<ThreadedCacheType>::sThreadCache;



int test_single_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 512u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;

    // test allocator of 64 bytes in a 256-byte container
    ls::utils::GeneralAllocator<block_size> testAllocator{alloc_table_size};
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



class MallocMemorySource2 final : public MemorySource
{
  private:
    static constexpr size_t header_size = 16;
    static constexpr size_t block_size = 16;
    size_t mMaxAllocSize;
    size_t mBytesAllocated;

  public:
    virtual ~MallocMemorySource2() noexcept override {}

    MallocMemorySource2(size_t maxNumBytes) noexcept :
        MemorySource{},
        mMaxAllocSize{maxNumBytes},
        mBytesAllocated{0}
    {}

    MallocMemorySource2(const MallocMemorySource2& src) noexcept :
        MemorySource{src},
        mMaxAllocSize{src.mMaxAllocSize},
        mBytesAllocated{src.mBytesAllocated}
    {}

    MallocMemorySource2(MallocMemorySource2&& src) noexcept :
        MemorySource{std::move(src)},
        mMaxAllocSize{src.mMaxAllocSize},
        mBytesAllocated{src.mBytesAllocated}
    {
        src.mMaxAllocSize = 0;
        src.mBytesAllocated = 0;
    }

    MallocMemorySource2& operator=(const MallocMemorySource2&) noexcept = default;
    MallocMemorySource2& operator=(MallocMemorySource2&&) noexcept = default;

    virtual void* allocate(size_t numBytes) noexcept override
    {
        numBytes += header_size;
        const size_t blocksFreed = (numBytes / block_size) + ((numBytes % block_size) ? 1 : 0);
        const size_t n = blocksFreed * block_size;

        if (mBytesAllocated+n > mMaxAllocSize)
        {
            return nullptr;
        }

        mBytesAllocated += n;
        return new(std::nothrow) char[n];
    }

    virtual void free(void* pData, size_t numBytes) noexcept override
    {
        numBytes += header_size;
        const size_t blocksFreed = (numBytes / block_size) + ((numBytes % block_size) ? 1 : 0);
        const size_t n = blocksFreed * block_size;

        if (pData)
        {
            mBytesAllocated -= n;
            delete [] reinterpret_cast<char*>(pData);
        }
    }
};



class LSUtilsMemorySource final : public MemorySource
{
  private:
    static constexpr size_t block_size = 16u;
    ls::utils::GeneralAllocator<block_size> mAllocator;

  public:
    virtual ~LSUtilsMemorySource() noexcept override {}

    LSUtilsMemorySource(size_t maxNumBytes) noexcept :
        MemorySource{},
        mAllocator{maxNumBytes}
    {}

    LSUtilsMemorySource(const LSUtilsMemorySource& src) noexcept = delete;

    LSUtilsMemorySource(LSUtilsMemorySource&& src) noexcept :
        MemorySource{std::move(src)},
        mAllocator{std::move(src.mAllocator)}
    {
    }

    LSUtilsMemorySource& operator=(const LSUtilsMemorySource&) noexcept = delete;
    LSUtilsMemorySource& operator=(LSUtilsMemorySource&& src) noexcept = delete;

    virtual void* allocate(size_t numBytes) noexcept override
    {
        return mAllocator.allocate(numBytes);
    }

    virtual void free(void* pData, size_t numBytes) noexcept override
    {
        mAllocator.free(pData, numBytes);
    }
};



int test_array_allocations()
{
    constexpr unsigned alloc_table_size = 1024u*1024u*1024u;
    constexpr unsigned block_size = 16u;
    constexpr unsigned max_allocations = alloc_table_size / block_size;
    //constexpr unsigned mid_allocation = (max_allocations / 3u - 1u) / 2u;

    // test allocator of 64 bytes in a 256-byte container
    //ls::utils::GeneralAllocator<block_size> testAllocator{alloc_table_size};

    LSUtilsMemorySource mallocSrc{alloc_table_size};
    //MallocMemorySource2 mallocSrc{alloc_table_size};
    AtomicAllocator atomicAllocator{mallocSrc};
    ThreadedAllocator<Allocator> testAllocator{atomicAllocator};

    void** allocations = new void*[max_allocations];
    void* p = nullptr;

    for (unsigned i = 0; i < max_allocations; ++i)
    {
        allocations[i] = nullptr;
    }

    for (unsigned testRuns = 0; testRuns < 5; ++testRuns)
    {
        for (unsigned i = 0; i < max_allocations/3 - 1; ++i)
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
    MallocMemorySource mallocSrc;
    AtomicAllocator atomicAllocator{mallocSrc};
    ThreadedAllocator<Allocator> threadedAllocator{atomicAllocator};
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