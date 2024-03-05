/*
 * File:   AllocatorImpl.hpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:03 PM
 */

#ifndef LS_UTILS_ALLOCATOR_IMPL_HPP
#define LS_UTILS_ALLOCATOR_IMPL_HPP

#include <new> // new overload
#include <utility> // std::move

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * IAllocator
-----------------------------------------------------------------------------*/
constexpr bool IAllocator::calloc_can_overflow(size_type numElements, size_type numBytesPerElement) noexcept
{
    return numBytesPerElement && numElements != ((numElements * numBytesPerElement) / numBytesPerElement);
}



/*-------------------------------------
 * Allocate (with internal feedback)
-------------------------------------*/
inline void* IAllocator::allocate(size_type n, size_type* pOutNumBytes) noexcept
{
    return this->memory_source().allocate(n, pOutNumBytes);
}



/*-------------------------------------
 * Free
-------------------------------------*/
inline void IAllocator::free(void* pData) noexcept
{
    this->memory_source().free(pData);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
inline void IAllocator::free(void* p, size_type n) noexcept
{
    this->memory_source().free(p, n);
}



/*-----------------------------------------------------------------------------
 * Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Internal Memory Source (const)
-------------------------------------*/
inline const MemorySource& Allocator::memory_source() const noexcept
{
    return *mMemSource;
}



/*-------------------------------------
 * Internal Memory Source
-------------------------------------*/
inline MemorySource& Allocator::memory_source() noexcept
{
    return *mMemSource;
}



/*-----------------------------------------------------------------------------
 * Constrained Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <unsigned long long MaxNumBytes>
ConstrainedAllocator<MaxNumBytes>::~ConstrainedAllocator() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long MaxNumBytes>
ConstrainedAllocator<MaxNumBytes>::ConstrainedAllocator(MemorySource& src) noexcept :
    Allocator{src},
    mBytesAllocated{0}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long MaxNumBytes>
ConstrainedAllocator<MaxNumBytes>::ConstrainedAllocator(ConstrainedAllocator&& allocator) noexcept :
    Allocator{std::move(allocator)},
    mBytesAllocated{allocator.mBytesAllocated}
{
    allocator.mBytesAllocated = 0;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <unsigned long long MaxNumBytes>
ConstrainedAllocator<MaxNumBytes>& ConstrainedAllocator<MaxNumBytes>::operator=(ConstrainedAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        Allocator::operator=(std::move(allocator));

        mBytesAllocated = allocator.mBytesAllocated;
        allocator.mBytesAllocated = 0;
    }

    return *this;
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
template <unsigned long long MaxNumBytes>
inline void* ConstrainedAllocator<MaxNumBytes>::allocate(size_type numBytes, size_type* pOutNumBytes) noexcept
{
    const size_type newByteCount = mBytesAllocated+numBytes;

    if (newByteCount > MaxNumBytes)
    {
        return nullptr;
    }

    void* pData = this->memory_source().allocate(numBytes, pOutNumBytes);
    if (pData)
    {
        mBytesAllocated = newByteCount;
    }

    return  pData;
}



/*-------------------------------------
 * Calloc (sized)
-------------------------------------*/
template <unsigned long long MaxNumBytes>
void* ConstrainedAllocator<MaxNumBytes>::allocate_contiguous(size_type numElements, size_type numBytesPerElement, size_type* pOutNumBytes) noexcept
{
    if (!numElements || !numBytesPerElement)
    {
        return nullptr;
    }

    if (calloc_can_overflow(numElements, numBytesPerElement))
    {
        return nullptr;
    }

    const size_type numBytes = numElements * numBytesPerElement;
    const size_type newByteCount = mBytesAllocated+numBytes;

    if (newByteCount > MaxNumBytes)
    {
        return nullptr;
    }

    void* const pData = this->memory_source().allocate(numBytes, pOutNumBytes);
    if (pData)
    {
        mBytesAllocated = newByteCount;
        fast_memset(pData, '\0', pOutNumBytes ? *pOutNumBytes : numBytes);
    }

    return pData;
}



/*-------------------------------------
 * Free
-------------------------------------*/
template <unsigned long long MaxNumBytes>
void ConstrainedAllocator<MaxNumBytes>::free(void* pData) noexcept
{
    (void)pData;
    LS_ASSERT(false);
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
template <unsigned long long MaxNumBytes>
inline void ConstrainedAllocator<MaxNumBytes>::free(void* pData, size_type numBytes) noexcept
{
    if (pData)
    {
        LS_ASSERT(mBytesAllocated >= numBytes);
        mBytesAllocated -= numBytes;
        this->memory_source().free(pData, numBytes);
    }
}



/*-----------------------------------------------------------------------------
 * Constrained Allocator (specialized for run-time constraints)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
inline void* ConstrainedAllocator<0>::allocate(size_type numBytes, size_type* pOutNumBytes) noexcept
{
    const size_type newByteCount = mBytesAllocated+numBytes;

    if (newByteCount > mMaxAllocSize)
    {
        return nullptr;
    }

    void* const pData = this->memory_source().allocate(numBytes, pOutNumBytes);
    if (pData)
    {
        mBytesAllocated = newByteCount;
    }

    return  pData;
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
inline void ConstrainedAllocator<0>::free(void* pData, size_type numBytes) noexcept
{
    if (pData)
    {
        LS_ASSERT(mBytesAllocated >= numBytes);
        mBytesAllocated -= numBytes;
        this->memory_source().free(pData, numBytes);
    }
}



/*-----------------------------------------------------------------------------
 * Block Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <unsigned long long BlockSize>
BlockAllocator<BlockSize>::~BlockAllocator() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <unsigned long long BlockSize>
BlockAllocator<BlockSize>::BlockAllocator(MemorySource& src) noexcept :
    Allocator{src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <unsigned long long BlockSize>
BlockAllocator<BlockSize>::BlockAllocator(BlockAllocator&& allocator) noexcept :
    Allocator{std::move(allocator)}
{
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <unsigned long long BlockSize>
BlockAllocator<BlockSize>& BlockAllocator<BlockSize>::operator=(BlockAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        Allocator::operator=(std::move(allocator));
    }

    return *this;
}



/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
template <unsigned long long BlockSize>
inline void* BlockAllocator<BlockSize>::allocate(size_type numBytes, size_type* pOutNumBytes) noexcept
{
    const size_type remainder = numBytes % BlockSize;
    numBytes = numBytes + (remainder ? (BlockSize - remainder) : 0);

    return this->memory_source().allocate(numBytes, pOutNumBytes);
}



/*-------------------------------------
 * Calloc (sized)
-------------------------------------*/
template <unsigned long long BlockSize>
void* BlockAllocator<BlockSize>::allocate_contiguous(size_type numElements, size_type numBytesPerElement, size_type* pOutNumBytes) noexcept
{
    if (!numElements || !numBytesPerElement)
    {
        return nullptr;
    }

    if (calloc_can_overflow(numElements, numBytesPerElement))
    {
        return nullptr;
    }

    size_type numBytes = numElements * numBytesPerElement;
    const size_type remainder = numBytes % BlockSize;
    numBytes = numBytes + (remainder ? (BlockSize - remainder) : 0);

    void* const pData = this->memory_source().allocate(numBytes, pOutNumBytes);
    if (pData)
    {
        fast_memset(pData, '\0', pOutNumBytes ? *pOutNumBytes : numBytes);
    }

    return pData;
}



/*-------------------------------------
 * Free
-------------------------------------*/
template <unsigned long long BlockSize>
void BlockAllocator<BlockSize>::free(void* pData) noexcept
{
    if (pData)
    {
        this->memory_source().free(pData);
    }
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
template <unsigned long long BlockSize>
inline void BlockAllocator<BlockSize>::free(void* pData, size_type numBytes) noexcept
{
    if (pData)
    {
        const size_type remainder = numBytes % BlockSize;
        numBytes = numBytes + (remainder ? (BlockSize - remainder) : 0);
        
        this->memory_source().free(pData, numBytes);
    }
}



/*-----------------------------------------------------------------------------
 * Thread-safe Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Thread-Safety Check
-------------------------------------*/
constexpr bool ThreadSafeAllocator::is_thread_safe() noexcept
{
    return true;
}



/*-----------------------------------------------------------------------------
 * Atomic Allocator wrapper
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Allocate (sized)
-------------------------------------*/
inline void* AtomicAllocator::allocate(size_type n, size_type* pOutNumBytes) noexcept
{
    mLock.lock();
    void* const pData = this->memory_source().allocate(n, pOutNumBytes);
    mLock.unlock();

    return pData;
}



/*-------------------------------------
 * Free
-------------------------------------*/
inline void AtomicAllocator::free(void* p) noexcept
{
    mLock.lock();
    this->memory_source().free(p);
    mLock.unlock();
}



/*-------------------------------------
 * Free (sized)
-------------------------------------*/
inline void AtomicAllocator::free(void* p, size_type n) noexcept
{
    mLock.lock();
    this->memory_source().free(p, n);
    mLock.unlock();
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_ALLOCATOR_IMPL_HPP */
