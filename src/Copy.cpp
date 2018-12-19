
#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Compiler.h"

#include "lightsky/utils/Bits.h"
#include "lightsky/utils/Copy.h"

#ifdef LS_ARCH_X86
    #include <emmintrin.h>
#elif defined(LS_ARCH_ARM)
    #include <arm_neon.h>
#endif



namespace ls {



/*-------------------------------------
 * fast_memcpy
-------------------------------------*/
void* utils::fast_memcpy(void* const dst, const void* const src, const std::size_t count)
{
    #ifdef LS_ARCH_X86
        std::size_t       stragglers = count % sizeof(__m128i);
        const std::size_t simdCount  = (count-stragglers)/sizeof(__m128i);
        const __m128i*    simdSrc    = reinterpret_cast<const __m128i*>(src);
        __m128i*          simdDst    = reinterpret_cast<__m128i*>(dst);

        // Using stream intrinsics here is NOT OK because we need the cache for
        // reading "simdSrc."
        if (simdCount)
        {
            LS_UTILS_LOOP_UNROLL_32(simdCount, (*simdDst++ = *simdSrc++))
        }

        const char* s = reinterpret_cast<const char*>(simdSrc);
        char*       d = reinterpret_cast<char*>(simdDst);
        while (stragglers --> 0)
        {
            *d++ = *s++;
        }
    #elif defined(LS_ARCH_ARM)
        std::size_t       stragglers = count % sizeof(uint32x4_t);
        const std::size_t simdCount  = (count-stragglers)/sizeof(uint32x4_t);
        const uint32x4_t* simdSrc    = reinterpret_cast<const uint32x4_t*>(src);
        uint32x4_t*       simdDst    = reinterpret_cast<uint32x4_t*>(dst);

        if (simdCount)
        {
            LS_UTILS_LOOP_UNROLL_32(simdCount, vst1q_u32(reinterpret_cast<uint32_t*>(simdDst++), *simdSrc++))
        }

        const char* s = reinterpret_cast<const char*>(simdSrc);
        char*       d = reinterpret_cast<char*>(simdDst);
        while (stragglers --> 0)
        {
            *d++ = *s++;
        }
    #else
        char* to = reinterpret_cast<char*>(dst);
        const char* from = reinterpret_cast<const char*>(src);

        if (simdCount)
        {
            LS_UTILS_LOOP_UNROLL_32(count, (*to++ = *from++))
        }
    #endif

    return dst;
}



/*-------------------------------------
 * fast_memset
-------------------------------------*/
void* utils::fast_memset(void* dst, const unsigned char fillByte, std::size_t count)
{
    #ifdef LS_ARCH_X86
        std::size_t       stragglers   = count % sizeof(__m128i);
        const std::size_t simdCount    = (count-stragglers)/sizeof(__m128i);
        const __m128i     simdFillByte = _mm_set1_epi8(fillByte);
        __m128i*          simdTo       = reinterpret_cast<__m128i*>(dst);

        // Using stream intrinsics here is OK because we're not reading data
        // from memory
        if (simdCount)
        {
            #ifdef LS_COMPILER_MSC
                LS_UTILS_LOOP_UNROLL_8(simdCount, (*simdTo++ = simdFillByte))
            #else
                LS_UTILS_LOOP_UNROLL_8(simdCount, _mm_stream_si128(simdTo++, simdFillByte))
            #endif
        }

        char* to = reinterpret_cast<char*>(simdTo);
        while (stragglers --> 0)
        {
            *to++ = fillByte;
        }
    #elif defined(LS_ARCH_ARM)
        std::size_t       stragglers   = count % sizeof(uint32x4_t);
        const std::size_t simdCount    = (count-stragglers)/sizeof(uint32x4_t);
        const uint32_t    fillByteU32  = ((uint32_t)fillByte) | (fillByte << 8) | (fillByte << 16) | (fillByte << 24);
        const uint32x4_t  simdFillByte = vdupq_n_u32(fillByteU32);
        uint32x4_t*       simdTo       = reinterpret_cast<uint32x4_t*>(dst);

        LS_UTILS_LOOP_UNROLL_8(simdCount, vst1q_u32(reinterpret_cast<uint32_t*>(simdTo++), simdFillByte))

        char* to = reinterpret_cast<char*>(simdTo);
        while (stragglers --> 0)
        {
            *to++ = fillByte;
        }
    #else
        char* to = reinterpret_cast<char*>(dst);
        LS_UTILS_LOOP_UNROLL_32(count, (*to++ = fillByte))
    #endif

    return dst;
}



} // end ls namespace
