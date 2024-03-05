/*
 * File:   MemorySource.hpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:23 PM
 */

#ifndef LS_UTILS_MEMORY_SOURCE_HPP
#define LS_UTILS_MEMORY_SOURCE_HPP


namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Memory Source
-----------------------------------------------------------------------------*/
class MemorySource
{
  public:
    typedef unsigned long long size_type;

  public:
    virtual ~MemorySource() noexcept = 0;

    MemorySource() noexcept = default;
    MemorySource(const MemorySource&) noexcept = default;
    MemorySource(MemorySource&&) noexcept = default;

    MemorySource& operator=(const MemorySource&) noexcept = default;
    MemorySource& operator=(MemorySource&&) noexcept = default;

    virtual void* allocate(size_type numBytes, size_type* pOutNumBytes = nullptr) noexcept = 0;

    virtual void free(void* pData) noexcept = 0;
    virtual void free(void* pData, size_type numBytes) noexcept = 0;
};



/*-----------------------------------------------------------------------------
 * Malloc-based Memory Source
-----------------------------------------------------------------------------*/
class MallocMemorySource final : public MemorySource
{
  public:
    virtual ~MallocMemorySource() noexcept override;

    MallocMemorySource() noexcept;

    MallocMemorySource(const MallocMemorySource& src) noexcept;

    MallocMemorySource(MallocMemorySource&& src) noexcept;

    MallocMemorySource& operator=(const MallocMemorySource&) noexcept = default;
    MallocMemorySource& operator=(MallocMemorySource&&) noexcept = default;

    virtual void* allocate(size_type numBytes, size_type* pOutNumBytes = nullptr) noexcept override;

    virtual void free(void* pData) noexcept override;
    virtual void free(void* pData, size_type numBytes) noexcept override;
};



/*-----------------------------------------------------------------------------
 * System-based Memory Source (currently mmap on posix systems)
-----------------------------------------------------------------------------*/
class SystemMemorySource final : public MemorySource
{
  public:
    static size_type page_size() noexcept;

  public:
    virtual ~SystemMemorySource() noexcept override;

    SystemMemorySource() noexcept;

    SystemMemorySource(const SystemMemorySource&) noexcept;

    SystemMemorySource(SystemMemorySource&& allocator) noexcept;

    SystemMemorySource& operator=(const SystemMemorySource& allocator) noexcept;

    SystemMemorySource& operator=(SystemMemorySource&& allocator) noexcept;

    virtual void* allocate(size_type numBytes, size_type* pOutNumBytes = nullptr) noexcept override;

    virtual void free(void* pData) noexcept override;
    virtual void free(void* pData, size_type numBytes) noexcept override;
};



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_MEMORY_SOURCE_HPP */
