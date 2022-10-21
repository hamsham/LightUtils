/*
 * File:   MemorySource.cpp
 * Author: hammy
 *
 * Created on Oct 19, 2022 at 6:23 PM
 */

#include <cstdlib> // malloc, free
#include <utility> // std::move

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



} // end utils namespace
} // end ls namespace
