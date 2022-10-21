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

    virtual void* allocate() noexcept = 0;
    virtual void* allocate(size_type numBytes) noexcept = 0;

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

    virtual void* allocate() noexcept override;
    virtual void* allocate(size_type numBytes) noexcept override;

    virtual void free(void* pData) noexcept override;
    virtual void free(void* pData, size_type numBytes) noexcept override;
};



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_MEMORY_SOURCE_HPP */
