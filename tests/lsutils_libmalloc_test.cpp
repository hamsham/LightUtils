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

constexpr unsigned main_block_size = 32u;
constexpr unsigned main_cache_size = 32768u;

constexpr unsigned tls_block_size = 32u;
constexpr unsigned tls_cache_size = 65536u;

typedef utils::GeneralAllocator<main_block_size, main_cache_size> CachedAllocatorType;
typedef utils::GeneralAllocator<tls_block_size, tls_cache_size> TLSAllocatorType;
typedef utils::ThreadedAllocator<TLSAllocatorType> ExternalAllocatorType;

inline ExternalAllocatorType& _get_allocator() noexcept
{
    static utils::SystemAllocator mallocSrc{};
    static CachedAllocatorType internalAllocator{mallocSrc};
    static utils::AtomicAllocator atomicAllocator{internalAllocator};
    static ExternalAllocatorType allocator{atomicAllocator};
    return allocator;
}

}


LS_API void* malloc(size_t size)
{
    ExternalAllocatorType& allocator = _get_allocator();
    return allocator.allocate(size);
}

LS_API void* calloc(size_t num, size_t size)
{
    ExternalAllocatorType& allocator = _get_allocator();
    return allocator.allocate_contiguous(num, size);
}

LS_API void* realloc(void* ptr, size_t size)
{
    ExternalAllocatorType& allocator = _get_allocator();
    return allocator.reallocate(ptr, size);
}

LS_API void free(void* ptr)
{
    ExternalAllocatorType& allocator = _get_allocator();
    allocator.free(ptr);
}
