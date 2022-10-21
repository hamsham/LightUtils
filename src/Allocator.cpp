/*
 * File:   Allocator.cpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:03 PM
 */

#include "lightsky/utils/Allocator.hpp"

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
 * Allocate
-------------------------------------*/
void* IAllocator::allocate() noexcept
{
    return nullptr;
}



/*-------------------------------------
 * Free
-------------------------------------*/
void IAllocator::free(void* pData) noexcept
{
    (void)pData;
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
-------------------------------------*/
Allocator::Allocator(MemorySource& src) noexcept :
    mMemSource{&src}
{}



/*-------------------------------------
-------------------------------------*/
Allocator::Allocator(Allocator&& allocator) noexcept :
    IAllocator{allocator},
    mMemSource{allocator.mMemSource}
{
    allocator.mMemSource = nullptr;
}



/*-------------------------------------
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
-------------------------------------*/
ThreadSafeAllocator::ThreadSafeAllocator(ThreadSafeAllocator&& allocator) noexcept :
    Allocator{std::move(allocator)}
{}



/*-------------------------------------
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
-------------------------------------*/
MallocAllocator::MallocAllocator(ls::utils::MallocMemorySource& src) noexcept :
    ThreadSafeAllocator{src}
{}



/*-------------------------------------
-------------------------------------*/
MallocAllocator::MallocAllocator(MallocAllocator&& allocator) noexcept :
    ThreadSafeAllocator{std::move(allocator)}
{}



/*-------------------------------------
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
-------------------------------------*/
AtomicAllocator::AtomicAllocator(AtomicAllocator&& allocator) noexcept :
    ThreadSafeAllocator{std::move(allocator)},
    mLock{}
{
}



/*-------------------------------------
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



/*-----------------------------------------------------------------------------
 * Threaded Memory Cache
-----------------------------------------------------------------------------*/
template class ThreadedMemoryCache<ls::utils::Allocator>;



/*-----------------------------------------------------------------------------
 * Threaded Allocator
-----------------------------------------------------------------------------*/
template class ThreadedAllocator<ls::utils::Allocator>;




} // end utils namespace
} // end ls namespace
