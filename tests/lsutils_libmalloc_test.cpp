/*
 * File:   lsutils_test_malloc.cpp
 * Author: hammy
 *
 * Created on Nov 05, 2022 at 1:30 AM
 */

#include "lightsky/setup/Api.h"

#include "lightsky/utils/GeneralAllocator.hpp"

namespace utils = ls::utils;



namespace
{

//constexpr unsigned main_block_size = 32u;
constexpr unsigned main_cache_size = 4096u;

//constexpr unsigned tls_block_size = 32u;
constexpr unsigned tls_cache_size = 2u*1024u*1024u;

typedef utils::GeneralAllocator<main_cache_size, true> CachedAllocatorType;
typedef utils::GeneralAllocator<tls_cache_size, false> ExternalAllocatorType;



inline ExternalAllocatorType& _get_allocator() noexcept
{
    static utils::SystemMemorySource mallocSrc{};
    static CachedAllocatorType internalAllocator{mallocSrc};
    static utils::AtomicAllocator atomicAllocator{internalAllocator};
    static thread_local ExternalAllocatorType allocator{atomicAllocator};

    return allocator;
}

} // end anonymous namespace



extern "C" void* LS_API malloc(size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.allocate(size);
}

extern "C" void* LS_API calloc(size_t num, size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.allocate_contiguous(num, size);
}

extern "C" void* LS_API realloc(void* ptr, size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.reallocate(ptr, size);
}

extern "C" void LS_API free(void* ptr)
{
    ExternalAllocatorType& a = _get_allocator();
    a.free(ptr);
}
