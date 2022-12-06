/*
 * File:   lsutils_test_malloc.cpp
 * Author: hammy
 *
 * Created on Nov 05, 2022 at 1:30 AM
 */

#include "lightsky/setup/Api.h"

#include "lightsky/utils/GeneralAllocator.hpp"

namespace utils = ls::utils;



#ifndef _LSMALLOC_CACHE_MEM_SRC
    #define _LSMALLOC_CACHE_MEM_SRC 1
#endif



namespace
{

//constexpr unsigned main_block_size = 32u;
constexpr unsigned main_cache_size = 4096u;

//constexpr unsigned tls_block_size = 32u;
constexpr unsigned tls_cache_size = 2u*1024u*1024u;

typedef utils::GeneralAllocator<main_cache_size, true> CachedAllocatorType;
typedef utils::GeneralAllocator<tls_cache_size, false> TLSAllocatorType;

#if _LSMALLOC_CACHE_MEM_SRC
    typedef utils::ThreadLocalAllocator<TLSAllocatorType> ExternalAllocatorType;
#else
    typedef utils::AtomicAllocator ExternalAllocatorType;
#endif



inline ExternalAllocatorType& _get_allocator() noexcept
{
    static utils::SystemMemorySource mallocSrc{};
    static CachedAllocatorType internalAllocator{mallocSrc};

    #if _LSMALLOC_CACHE_MEM_SRC
        static utils::AtomicAllocator atomicAllocator{internalAllocator};
        static ExternalAllocatorType allocator{atomicAllocator};
    #else
        static ExternalAllocatorType allocator{internalAllocator};
    #endif

    return allocator;
}

}



LS_API void* malloc(size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.allocate(size);
}

LS_API void* calloc(size_t num, size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.allocate_contiguous(num, size);
}

LS_API void* realloc(void* ptr, size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.reallocate(ptr, size);
}

LS_API void free(void* ptr)
{
    ExternalAllocatorType& a = _get_allocator();
    a.free(ptr);
}
