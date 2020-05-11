
#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Compiler.h"
#include "lightsky/setup/Macros.h"

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
void* utils::fast_memcpy(void* const LS_RESTRICT_PTR dst, const void* const LS_RESTRICT_PTR src, const uint_fast64_t count)
{
    #if defined(LS_ARCH_X86)
        const __m256i* simdSrc    = reinterpret_cast<const __m256i*>(src);
        __m256i*       simdDst    = reinterpret_cast<__m256i*>(dst);
        uint_fast64_t  simdCount  = count >> 5;
        uint_fast64_t  stragglers = count - (count & ~31);

        if ((uintptr_t)simdDst % sizeof(__m256i))
        {
            while (simdCount--)
            {
                _mm256_storeu_si256(simdDst++, _mm256_lddqu_si256(simdSrc++));
            }
        }
        else
        {
            while (simdCount--)
            {
                _mm256_stream_si256(simdDst++, _mm256_lddqu_si256(simdSrc++));
            }
        }
    #elif defined(LS_ARCH_ARM)
        const uint32x4_t* simdSrc    = reinterpret_cast<const uint32x4_t*>(src);
        uint32x4_t*       simdDst    = reinterpret_cast<uint32x4_t*>(dst);
        uint_fast64_t     simdCount  = count >> 4;
        uint_fast64_t     stragglers = count - (count & ~15);

        while (simdCount--)
        {
            vst1q_u32(reinterpret_cast<uint32_t*>(simdDst++), vld1q_u32(reinterpret_cast<const uint32_t*>(simdSrc++)));
        }
    #else
        const uint_fast64_t* simdSrc    = reinterpret_cast<const uint_fast64_t*>(src);
        uint_fast64_t*       simdDst    = reinterpret_cast<uint_fast64_t*>(dst);
        uint_fast64_t        simdCount  = count >> 3;
        uint_fast64_t        stragglers = count - (count & ~7);

        // Using stream intrinsics here is NOT OK because we need the cache for
        // reading "simdSrc."
        while (simdCount--)
        {
            *simdDst++ = *simdSrc++;
        }

    #endif

    const uint8_t* s = reinterpret_cast<const uint8_t*>(simdSrc);
    uint8_t*       d = reinterpret_cast<uint8_t*>(simdDst);

    while (stragglers--)
    {
        *d++ = *s++;
    }

    return dst;
}



/*-------------------------------------
 * fast_memset of 4-bytes at a time
-------------------------------------*/
void* utils::fast_memset_8(void* dst, const uint64_t fillBytes, uint_fast64_t count)
{
    #if defined(LS_ARCH_X86)
        const __m256i simdFillByte = _mm256_set1_epi64x(fillBytes);
        __m256i*      simdTo       = reinterpret_cast<__m256i*>(dst);
        uint_fast64_t simdCount    = count >> 5;
        uint_fast64_t stragglers   = count - (count & ~31);

        // Using stream intrinsics here is OK because we're not reading data
        // from memory
        if ((uintptr_t)simdTo % sizeof(__m256i))
        {
            while (simdCount--)
            {
                _mm256_storeu_si256(simdTo++, simdFillByte);
            }
        }
        else
        {
            while (simdCount--)
            {
                _mm256_stream_si256(simdTo++, simdFillByte);
            }
        }
    #elif defined(LS_ARCH_ARM)
        const uint64x2_t fillByteSimd = vdupq_n_u64(fillBytes);
        uint64x2_t*      simdTo       = reinterpret_cast<uint64x2_t*>(dst);
        uint_fast64_t     simdCount  = count >> 4;
        uint_fast64_t     stragglers = count - (count & ~15);

        while (simdCount--)
        {
            vst1q_u64(reinterpret_cast<uint64_t*>(simdTo++), fillByteSimd);
        }
    #else
        const uint_fast64_t fillByteSimd = (uint_fast64_t)fillBytes;
        uint_fast64_t*      simdTo       = reinterpret_cast<uint_fast64_t*>(dst);
        uint_fast64_t       simdCount    = count >> 3;
        uint_fast64_t       stragglers   = count - (count & ~7);

        // Using stream intrinsics here is OK because we're not reading data
        // from memory
        while (simdCount--)
        {
            *simdTo++ = fillByteSimd;
        }
    #endif

    uint8_t*      to = reinterpret_cast<uint8_t*>(simdTo);
    const uint8_t fillByte = (uint8_t)fillBytes;

    while (stragglers--)
    {
        *to++ = fillByte;
    }

    return dst;
}



} // end ls namespace
