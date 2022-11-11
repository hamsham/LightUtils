/*
 * File:   MemorySource.cpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:23 PM
 */

#include "lightsky/setup/OS.h"

#if defined(LS_OS_UNIX)
    extern "C"
    {
        #include <errno.h>
        #include <string.h> // strerror()
        #include <sys/mman.h> // mmap
        #include <unistd.h> // sysconf(_SC_PAGESIZE)
    }
#endif

#include <cstdlib> // malloc, free
#include <utility> // std::move

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/MemorySource.hpp"



namespace ls
{
namespace utils
{

/*-----------------------------------------------------------------------------
 * Memory Source
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
MemorySource::~MemorySource() noexcept
{
}



/*-----------------------------------------------------------------------------
 * Malloc-based Memory Source
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
MallocMemorySource::~MallocMemorySource() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
MallocMemorySource::MallocMemorySource() noexcept :
    MemorySource{}
{}



/*-------------------------------------
 * Copy Constrcuctor
-------------------------------------*/
MallocMemorySource::MallocMemorySource(const MallocMemorySource& src) noexcept :
    MemorySource{src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
MallocMemorySource::MallocMemorySource(MallocMemorySource&& src) noexcept :
    MemorySource{std::move(src)}
{}



/*-------------------------------------
 * Allocate
-------------------------------------*/
void* MallocMemorySource::allocate() noexcept
{
    return nullptr;
}



/*-------------------------------------
 * Allocate
-------------------------------------*/
void* MallocMemorySource::allocate(size_type numBytes) noexcept
{
    return std::malloc(numBytes);
}



/*-------------------------------------
 * Free
-------------------------------------*/
void MallocMemorySource::free(void* pData) noexcept
{
    (void)pData;
}



/*-------------------------------------
 * Free
-------------------------------------*/
void MallocMemorySource::free(void* pData, size_type numBytes) noexcept
{
    (void)numBytes;
    std::free(pData);
}



/*-----------------------------------------------------------------------------
 * System-based Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Get the system page size
-------------------------------------*/
SystemAllocator::size_type SystemAllocator::page_size() noexcept
{
    #if !defined(LS_OS_UNIX)
        return 4096ull;

    #else
        const long temp = sysconf(_SC_PAGESIZE);
        if (temp < 0)
        {
            runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
            return 0;
        }

        return (size_type)temp;
    #endif
}


/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemAllocator::~SystemAllocator() noexcept
{
}


/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemAllocator::SystemAllocator() noexcept
{
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SystemAllocator::SystemAllocator(const SystemAllocator& src) noexcept :
    MemorySource{src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SystemAllocator::SystemAllocator(SystemAllocator&& allocator) noexcept :
    MemorySource{std::move(allocator)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SystemAllocator& SystemAllocator::operator=(const SystemAllocator& allocator) noexcept
{
    if (&allocator != this)
    {
        MemorySource::operator=(allocator);
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SystemAllocator& SystemAllocator::operator=(SystemAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        MemorySource::operator=(std::move(allocator));
    }

    return *this;
}



/*-------------------------------------
 * Allocate
-------------------------------------*/
void* SystemAllocator::allocate() noexcept
{
    return this->allocate(page_size());
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
void* SystemAllocator::allocate(size_type numBytes) noexcept
{
    if (!numBytes)
    {
        return nullptr;
    }

    static const unsigned long long pageSize = page_size();
    numBytes += pageSize - (numBytes % pageSize);
    void* p;

    #if !defined(LS_OS_UNIX)
        p = std::malloc(numBytes, sizeof(char));

    #else
        constexpr int mapFlags = MAP_PRIVATE | MAP_ANON;
        p = mmap(nullptr, numBytes, PROT_READ|PROT_WRITE, mapFlags, -1, 0);
        if (p == MAP_FAILED)
        {
            runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
            p = nullptr;
        }
    #endif

    return p;
}



/*-------------------------------------
 * Free
-------------------------------------*/
void SystemAllocator::free(void* pData) noexcept
{
    static const unsigned long long pageSize = page_size();
    this->free(pData, pageSize);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
void SystemAllocator::free(void* pData, size_type numBytes) noexcept
{
    if (!pData || !numBytes)
    {
        return;
    }

    #if !defined(LS_OS_UNIX)
        (void)numBytes;
        std::free(pData);

    #else
        //int err = madvise(pData, numBytes, MADV_DONTNEED);
        int err = munmap(pData, numBytes);
        if (err != 0)
        {
            runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
        }
    #endif
}



} // end utils namespace
} // end ls namespace
