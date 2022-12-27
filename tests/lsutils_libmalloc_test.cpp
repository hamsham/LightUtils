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



namespace
{

//constexpr unsigned main_block_size = 32u;
constexpr unsigned main_cache_size = 3072;

//constexpr unsigned tls_block_size = 32u;
constexpr unsigned tls_cache_size = 3u*1024u*1024u;

typedef utils::GeneralAllocator<main_cache_size, true> CachedAllocatorType;
typedef utils::GeneralAllocator<tls_cache_size, false> ExternalAllocatorType;



#if defined(LS_OS_UNIX)
    struct TLSDestructorData
    {
        pthread_key_t* pKey;
        utils::AtomicAllocator* pAllocator;
        ExternalAllocatorType* pData;
    };

    inline ExternalAllocatorType& _get_allocator() noexcept
    {
        static utils::SystemMemorySource mallocSrc{};
        static CachedAllocatorType internalAllocator{mallocSrc};
        static utils::AtomicAllocator atomicAllocator{internalAllocator};
        //static __thread ExternalAllocatorType allocator{atomicAllocator};
        static __thread ExternalAllocatorType* allocator = nullptr;

        static __thread pthread_key_t key = 0;
        static __thread int result = -1;

        // Setup a destructor for TLS storage
        if (LS_UNLIKELY(!allocator))
        {
            allocator = new(atomicAllocator.allocate(sizeof(ExternalAllocatorType))) ExternalAllocatorType{atomicAllocator};

            if (LS_UNLIKELY(result < 0))
            {
                static __thread char destructBuf[sizeof(TLSDestructorData)];
                TLSDestructorData* pDestructBuf = reinterpret_cast<TLSDestructorData*>(destructBuf);

                pDestructBuf->pKey = &key;
                pDestructBuf->pAllocator = &atomicAllocator;
                pDestructBuf->pData = allocator;

                result = pthread_key_create(&key, [](void* pData)->void {
                    utils::AtomicAllocator* pAllocator = static_cast<TLSDestructorData*>(pData)->pAllocator;
                    ExternalAllocatorType* pTlsAllocator = static_cast<TLSDestructorData*>(pData)->pData;
                    pthread_key_t* pKey = static_cast<TLSDestructorData*>(pData)->pKey;

                    pTlsAllocator->~ExternalAllocatorType();
                    pAllocator->free(pTlsAllocator);
                    pthread_key_delete(*pKey);
                });

                if (result == 0)
                {
                    pthread_setspecific(key, destructBuf);
                }
            }
        }

        return *allocator;
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
