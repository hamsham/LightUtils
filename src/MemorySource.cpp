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
        #include <fcntl.h>
        #include <string.h> // strerror()
        #include <sys/mman.h> // mmap
        #include <unistd.h> // sysconf(_SC_PAGESIZE)
    }

#elif defined(LS_OS_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif /* WIN32_LEAN_AND_MEAN */

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif /* NOMINMAX */

    #include <Windows.h>
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
#if defined(LS_OS_UNIX)

#if 0
class FDZero
{
  private:
    int mFd;

  public:
    ~FDZero() noexcept;
    FDZero() noexcept;
    FDZero(const FDZero& fd) noexcept;
    FDZero(FDZero&& fd) noexcept;
    FDZero& operator=(const FDZero& fd) noexcept;
    FDZero& operator=(FDZero&& fd) noexcept;
    int file_descriptor() const noexcept;
    static FDZero& instance() noexcept;
};

inline FDZero::~FDZero() noexcept
{
    if (mFd > 0)
    {
        const int err = close(mFd);
        if (err == -1)
        {
            runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
        }
    }

    mFd = -1;
}

inline FDZero::FDZero() noexcept :
    mFd{open("/dev/zero", O_RDONLY)}
{
    if (mFd == -1)
    {
        runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
    }
}

inline FDZero::FDZero(const FDZero& fd) noexcept :
    mFd{fd.mFd}
{
}

inline FDZero::FDZero(FDZero&& fd) noexcept :
    mFd{fd.mFd}
{
    fd.mFd = -1;
}

inline FDZero& FDZero::operator=(const FDZero& fd) noexcept
{
    if (this != &fd)
    {
        mFd = fd.mFd;
    }

    return *this;
}

inline FDZero& FDZero::operator=(FDZero&& fd) noexcept
{
    if (this != &fd)
    {
        mFd = fd.mFd;
        fd.mFd = -1;
    }

    return *this;
}

inline int FDZero::file_descriptor() const noexcept
{
    return mFd;
}

inline FDZero& FDZero::instance() noexcept
{
    static FDZero sFd{};
    return sFd;
}

#endif

#endif // LS_OS_UNIX


/*-------------------------------------
 * Get the system page size
-------------------------------------*/
SystemMemorySource::size_type SystemMemorySource::page_size() noexcept
{
    #if defined(LS_OS_UNIX)
        const long temp = sysconf(_SC_PAGESIZE);
        if (temp < 0)
        {
            runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
            return 0;
        }

        return (size_type)temp;

    #elif defined(LS_OS_WINDOWS)
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return (SystemMemorySource::size_type)info.dwPageSize;

    #else
        return 4096ull;

    #endif
}


/*-------------------------------------
 * Destructor
-------------------------------------*/
SystemMemorySource::~SystemMemorySource() noexcept
{
}


/*-------------------------------------
 * Constructor
-------------------------------------*/
SystemMemorySource::SystemMemorySource() noexcept
{
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SystemMemorySource::SystemMemorySource(const SystemMemorySource& src) noexcept :
    MemorySource{src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SystemMemorySource::SystemMemorySource(SystemMemorySource&& allocator) noexcept :
    MemorySource{std::move(allocator)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SystemMemorySource& SystemMemorySource::operator=(const SystemMemorySource& allocator) noexcept
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
SystemMemorySource& SystemMemorySource::operator=(SystemMemorySource&& allocator) noexcept
{
    if (&allocator != this)
    {
        MemorySource::operator=(std::move(allocator));
    }

    return *this;
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
void* SystemMemorySource::allocate(size_type numBytes) noexcept
{
    if (!numBytes)
    {
        return nullptr;
    }

    static const unsigned long long pageSize = page_size();
    const unsigned long long rem = numBytes % pageSize;
    numBytes += pageSize - (rem ? rem : pageSize);
    void* p = nullptr;

    #if defined(LS_OS_UNIX)
        #if 0
        static const FDZero& fdz = FDZero::instance();
        if (fdz.file_descriptor() != -1)
        {
            p = mmap(nullptr, numBytes, PROT_READ | PROT_WRITE, MAP_PRIVATE, fdz.file_descriptor(), 0);
            if (p == MAP_FAILED)
            {
                runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
                p = nullptr;
            }
        }

        #else

        p = mmap(nullptr, numBytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED)
        {
            runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
            p = nullptr;
        }

        #endif

    #elif defined(LS_OS_WINDOWS)
        p = VirtualAlloc(nullptr, numBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    #else
        p = std::malloc(numBytes);

    #endif

    return p;
}



/*-------------------------------------
 * Free
-------------------------------------*/
void SystemMemorySource::free(void* pData) noexcept
{
    static const unsigned long long pageSize = page_size();
    this->free(pData, pageSize);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
void SystemMemorySource::free(void* pData, size_type numBytes) noexcept
{
    if (!pData)
    {
        return;
    }

    #if defined(LS_OS_UNIX)
        const int err = munmap(pData, numBytes);
        if (err != 0)
        {
            //runtime_assert(false, ErrorLevel::LS_WARNING, strerror(errno));
            runtime_assert(false, ErrorLevel::LS_WARNING, "Invalid pointer detected on munmap().");
        }

    #elif defined(LS_OS_WINDOWS)
        (void)numBytes;
        BOOL success = VirtualFree(pData, 0, MEM_RELEASE);
        if (!success)
        {
            /*
            LPVOID lpMsgBuf;
            DWORD dw = GetLastError();

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0,
                nullptr
            );

            runtime_assert(false, ErrorLevel::LS_WARNING, reinterpret_cast<const char*>(lpMsgBuf));

            LocalFree(lpMsgBuf);
            */

            runtime_assert(false, ErrorLevel::LS_WARNING, "Invalid pointer detected on VirtualFree().");
        }

    #else
        (void)numBytes;
        std::free(pData);

    #endif
}



} // end utils namespace
} // end ls namespace
