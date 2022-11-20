/*
 * File:   Allocator.cpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:03 PM
 */

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Allocator.hpp"
#include "lightsky/utils/Copy.h"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * IAllocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
IAllocator::~IAllocator() noexcept
{
}



/*-------------------------------------
 * Calloc (sized)
-------------------------------------*/
void* IAllocator::allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept
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
    void* const pData = this->memory_source().allocate(numBytes);
    if (pData)
    {
        fast_memset(pData, '\0', numBytes);
    }

    return pData;
}



/*-------------------------------------
 * Realloc
-------------------------------------*/
void* IAllocator::reallocate(void* p, size_type numNewBytes) noexcept
{
    if (!numNewBytes)
    {
        if (p)
        {
            this->free(p);
        }

        return nullptr;
    }

    void* pNewData = this->allocate(numNewBytes);
    if (pNewData)
    {
        if (p)
        {
            this->free(p);
        }

        fast_memset(pNewData, '\x00', numNewBytes);
    }

    return pNewData;
}



/*-------------------------------------
 * Realloc (sized)
-------------------------------------*/
void* IAllocator::reallocate(void* p, size_type numNewBytes, size_type numPrevBytes) noexcept
{
    if (!numNewBytes)
    {
        if (p)
        {
            this->free(p);
        }

        return nullptr;
    }

    void* const pNewData = this->allocate(numNewBytes);
    if (pNewData)
    {
        if (p)
        {
            const size_type numMaxBytes = numNewBytes < numPrevBytes ? numNewBytes : numPrevBytes;
            fast_memcpy(pNewData, p, numMaxBytes);
            this->free(p, numPrevBytes);
        }
        else
        {
            fast_memset(pNewData, '\x00', numNewBytes);
        }
    }

    return pNewData;
}



/*-----------------------------------------------------------------------------
 * Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
Allocator::~Allocator() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Allocator::Allocator(MemorySource& src) noexcept :
    mMemSource{&src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
Allocator::Allocator(Allocator&& allocator) noexcept :
    IAllocator{allocator},
    mMemSource{allocator.mMemSource}
{
    allocator.mMemSource = nullptr;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
Allocator& Allocator::operator=(Allocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        IAllocator::operator=(std::move(allocator));

        mMemSource = allocator.mMemSource;
        allocator.mMemSource = nullptr;
    }

    return *this;
}



/*-----------------------------------------------------------------------------
 * Constrained Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
ConstrainedAllocator<0>::~ConstrainedAllocator() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
ConstrainedAllocator<0>::ConstrainedAllocator(MemorySource& src, size_type maxNumBytes) noexcept :
    Allocator{src},
    mBytesAllocated{0},
    mMaxAllocSize{maxNumBytes}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
ConstrainedAllocator<0>::ConstrainedAllocator(ConstrainedAllocator&& allocator) noexcept :
    Allocator{std::move(allocator)},
    mBytesAllocated{allocator.mBytesAllocated},
    mMaxAllocSize{allocator.mMaxAllocSize}
{
    allocator.mBytesAllocated = 0;
    allocator.mMaxAllocSize = 0;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
ConstrainedAllocator<0>& ConstrainedAllocator<0>::operator=(ConstrainedAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        Allocator::operator=(std::move(allocator));

        mBytesAllocated = allocator.mBytesAllocated;
        allocator.mBytesAllocated = 0;

        mMaxAllocSize = allocator.mMaxAllocSize;
        allocator.mMaxAllocSize = 0;
    }

    return *this;
}



/*-------------------------------------
 * Calloc (sized)
-------------------------------------*/
void* ConstrainedAllocator<0>::allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept
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

    if (newByteCount > mMaxAllocSize)
    {
        return nullptr;
    }

    void* const pData = this->memory_source().allocate(numBytes);
    if (pData)
    {
        mBytesAllocated = newByteCount;
        fast_memset(pData, '\0', numBytes);
    }

    return pData;
}



/*-------------------------------------
 * Free
-------------------------------------*/
void ConstrainedAllocator<0>::free(void* pData) noexcept
{
    (void)pData;
    LS_ASSERT(false);
}


/*-----------------------------------------------------------------------------
 * Thread-Safe Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
ThreadSafeAllocator::~ThreadSafeAllocator() noexcept
{
}



/*-------------------------------------
-------------------------------------*/
ThreadSafeAllocator::ThreadSafeAllocator(MemorySource& src) noexcept :
    Allocator{src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
ThreadSafeAllocator::ThreadSafeAllocator(ThreadSafeAllocator&& allocator) noexcept :
    Allocator{std::move(allocator)}
{}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
ThreadSafeAllocator& ThreadSafeAllocator::operator=(ThreadSafeAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        Allocator::operator=(std::move(allocator));
    }

    return *this;
}



/*-----------------------------------------------------------------------------
 * Malloc-based Allocator
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
MallocAllocator::~MallocAllocator() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
MallocAllocator::MallocAllocator(MallocMemorySource& src) noexcept :
    ThreadSafeAllocator{src}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
MallocAllocator::MallocAllocator(MallocAllocator&& allocator) noexcept :
    ThreadSafeAllocator{std::move(allocator)}
{}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
MallocAllocator& MallocAllocator::operator=(MallocAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        ThreadSafeAllocator::operator=(std::move(allocator));
    }

    return *this;
}



/*-----------------------------------------------------------------------------
 * Atomic Allocator wrapper
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
AtomicAllocator::~AtomicAllocator() noexcept
{}



/*-------------------------------------
-------------------------------------*/
AtomicAllocator::AtomicAllocator(MemorySource& src) noexcept :
    ThreadSafeAllocator{src},
    mLock{}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
AtomicAllocator::AtomicAllocator(AtomicAllocator&& allocator) noexcept :
    ThreadSafeAllocator{std::move(allocator)},
    mLock{}
{
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
AtomicAllocator& AtomicAllocator::operator=(AtomicAllocator&& allocator) noexcept
{
    if (&allocator != this)
    {
        mLock.lock();
        ThreadSafeAllocator::operator=(std::move(allocator));
        mLock.unlock();
    }

    return *this;
}



/*-------------------------------------
 * Calloc (sized)
-------------------------------------*/
void* AtomicAllocator::allocate_contiguous(size_type numElements, size_type numBytesPerElement) noexcept
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
    void* pData;

    mLock.lock();
    pData = this->memory_source().allocate(numBytes);
    mLock.unlock();

    if (pData)
    {
        fast_memset(pData, '\0', numBytes);
    }

    return pData;
}



/*-----------------------------------------------------------------------------
 * Threaded Memory Cache
-----------------------------------------------------------------------------*/
template class ThreadedMemoryCache<ls::utils::Allocator>;



/*-----------------------------------------------------------------------------
 * Threaded Allocator
-----------------------------------------------------------------------------*/
template class ThreadLocalAllocator<ls::utils::Allocator>;




} // end utils namespace
} // end ls namespace
