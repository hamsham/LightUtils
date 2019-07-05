
#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Compiler.h"

#include "lightsky/utils/Bits.h"
#include "lightsky/utils/Copy.h"

#ifdef LS_ARCH_X86
    #include <immintrin.h>
#elif defined(LS_ARCH_ARM)
    #include <arm_neon.h>
#endif



namespace ls {



/*-------------------------------------
 * fast_memcpy
-------------------------------------*/
void* utils::fast_memcpy(void* const dst, const void* const src, const std::size_t count)
{
    #if defined(LS_ARCH_X86)
        std::size_t    stragglers = count % sizeof(__m256i);
        std::size_t    simdCount  = (count-stragglers)/sizeof(__m256i);
        const __m256i* simdSrc    = reinterpret_cast<const __m256i*>(src);
        __m256i*       simdDst    = reinterpret_cast<__m256i*>(dst);

        // Using stream intrinsics here is NOT OK because we need the cache for
        // reading "simdSrc."
        while (simdCount--)
        {
            _mm256_storeu_si256(simdDst++, _mm256_lddqu_si256(simdSrc++));
        }
    #elif defined(LS_ARCH_ARM)
        std::size_t       stragglers = count % sizeof(uint32x4_t);
        std::size_t       simdCount  = (count-stragglers)/sizeof(uint32x4_t);
        const uint32x4_t* simdSrc    = reinterpret_cast<const uint32x4_t*>(src);
        uint32x4_t*       simdDst    = reinterpret_cast<uint32x4_t*>(dst);

        while (simdCount--)
        {
            vst1q_u32(reinterpret_cast<uint32_t*>(simdDst++), vld1q_u32(reinterpret_cast<const uint32_t*>(simdSrc++)));
        }
    #else
        std::size_t     stragglers = count % sizeof(uint64_t);
        std::size_t     simdCount  = (count-stragglers)/sizeof(uint64_t);
        const uint64_t* simdSrc    = reinterpret_cast<const uint64_t*>(src);
        uint64_t*       simdDst    = reinterpret_cast<uint64_t*>(dst);

        // Using stream intrinsics here is NOT OK because we need the cache for
        // reading "simdSrc."
        while (simdCount--)
        {
            *simdDst++ = *simdSrc++;
        }

    #endif

    const char* s = reinterpret_cast<const char*>(simdSrc) - stragglers;
    char*       d = reinterpret_cast<char*>(simdDst) - stragglers;
    while (stragglers--)
    {
        *d++ = *s++;
    }

    return dst;
}



/*-------------------------------------
 * fast_memset of 4-bytes at a time
-------------------------------------*/
void* utils::fast_memset_4(void* dst, const uint32_t fillBytes, std::size_t count)
{
    #if defined(LS_ARCH_X86)
        const __m256i simdFillByte = _mm256_set1_epi32((int32_t)fillBytes);
        __m256i*      simdTo       = reinterpret_cast<__m256i*>(dst);
        std::size_t   stragglers   = count % sizeof(__m256i);
        std::size_t   simdCount    = (count-stragglers)/sizeof(__m256i);

        // Using stream intrinsics here is OK because we're not reading data
        // from memory
        while (simdCount--)
        {
            _mm256_stream_si256(simdTo++, simdFillByte);
        }
    #elif defined(LS_ARCH_ARM)
        std::size_t      stragglers   = count % sizeof(uint32x4_t);
        std::size_t      simdCount    = (count-stragglers)/sizeof(uint32x4_t);
        const uint32x4_t fillByteSimd = vdupq_n_u32(fillBytes);
        uint32x4_t*      simdTo       = reinterpret_cast<uint32x4_t*>(dst);

        while (simdCount--)
        {
            vst1q_u32(reinterpret_cast<uint32_t*>(simdTo++), fillByteSimd);
        }
    #else
        uint32_t*   simdTo     = reinterpret_cast<uint32_t*>(dst);
        std::size_t stragglers = count % sizeof(uint32_t);
        std::size_t simdCount  = (count-stragglers)/sizeof(uint32_t);

        // Using stream intrinsics here is OK because we're not reading data
        // from memory
        while (simdCount--)
        {
            *simdTo++ = fillBytes;
        }
    #endif

    uint8_t*      to = reinterpret_cast<uint8_t*>(dst) + count - stragglers;
    const uint8_t fillByte = (uint8_t)fillBytes;

    while (stragglers--)
    {
        *to++ = fillByte;
    }

    return dst;
}



} // end ls namespace
