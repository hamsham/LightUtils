/*
 * File:   lsutils_libmalloc_test.cpp
 * Author: hammy
 *
 * Created on Nov 05, 2022 at 1:30 AM
 */

#include "lightsky/setup/Api.h"
#include "lightsky/setup/OS.h"

#if defined(LS_OS_UNIX)
    #include <pthread.h>
#endif

#include "lightsky/utils/GeneralAllocator.hpp"

namespace utils = ls::utils;

#ifdef LS_COMPILER_MSC
    #define _LSMALLOC_LINKAGE
#else
    #define _LSMALLOC_LINKAGE extern "C"
#endif



constexpr unsigned long long main_cache_size = 4096-32;
constexpr unsigned long long tls_cache_size = 3u*1024u*1024u;

typedef utils::GeneralAllocator<main_cache_size, true> CachedAllocatorType;
typedef utils::GeneralAllocator<tls_cache_size, false> ExternalAllocatorType;



namespace
{

#if defined(LS_OS_UNIX)
    struct TLSDestructorData
    {
        ExternalAllocatorType tlsAllocator;
        utils::AtomicAllocator* pParentAllocator;
        pthread_key_t key;
    };

    void _destroy_tls_allocator(void* pData) noexcept
    {
        TLSDestructorData* const pDestructorData = static_cast<TLSDestructorData*>(pData);
        ExternalAllocatorType&   tlsAllocator    = pDestructorData->tlsAllocator;
        utils::AtomicAllocator*  pAllocator      = pDestructorData->pParentAllocator;
        const pthread_key_t      key             = pDestructorData->key;

        tlsAllocator.~ExternalAllocatorType();
        pAllocator->free(pDestructorData);
        pthread_key_delete(key);
    }

    ExternalAllocatorType* _create_tls_allocator() noexcept
    {
        static utils::SystemMemorySource mallocSrc{};
        static CachedAllocatorType internalAllocator{mallocSrc};
        static utils::AtomicAllocator atomicAllocator{internalAllocator};

        TLSDestructorData* allocator = new(atomicAllocator.allocate(sizeof(TLSDestructorData))) TLSDestructorData{ExternalAllocatorType{atomicAllocator}, &atomicAllocator, 0};
        if (LS_LIKELY(allocator != nullptr))
        {
            allocator->pParentAllocator = &atomicAllocator;
            allocator->key = 0;

            const int result = pthread_key_create(&allocator->key, &_destroy_tls_allocator);
            if (LS_LIKELY(result == 0))
            {
                pthread_setspecific(allocator->key, allocator);
            }
            else
            {
                allocator->tlsAllocator.~ExternalAllocatorType();
                atomicAllocator.free(allocator);
                allocator = nullptr;
            }
        }

        return allocator ? &allocator->tlsAllocator : nullptr;
    }

    static thread_local ExternalAllocatorType* _allocator = _create_tls_allocator();
    inline LS_INLINE ExternalAllocatorType& _get_allocator() noexcept
    {
        return *_allocator;
    }

#else /* LS_OS_UNIX */
    inline ExternalAllocatorType& _get_allocator() noexcept
    {
        static utils::SystemMemorySource mallocSrc{};
        static CachedAllocatorType internalAllocator{mallocSrc};
        static utils::AtomicAllocator atomicAllocator{internalAllocator};
        static thread_local ExternalAllocatorType allocator{atomicAllocator};

        return allocator;
    }

#endif /* LS_OS_UNIX */

} // end anonymous namespace



_LSMALLOC_LINKAGE void* LS_API malloc(size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.allocate(size);
}

_LSMALLOC_LINKAGE void* LS_API calloc(size_t num, size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.allocate_contiguous(num, size);
}

_LSMALLOC_LINKAGE void* LS_API realloc(void* ptr, size_t size)
{
    ExternalAllocatorType& a = _get_allocator();
    return a.reallocate(ptr, size);
}

_LSMALLOC_LINKAGE void LS_API free(void* ptr)
{
    ExternalAllocatorType& a = _get_allocator();
    a.free(ptr);
}
