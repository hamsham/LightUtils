
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
        while (simdCount --> 0)
        {
            _mm256_store_si256(simdDst++, _mm256_lddqu_si256(simdSrc++));
        }

        const char* s = reinterpret_cast<const char*>(simdSrc) - stragglers;
        char*       d = reinterpret_cast<char*>(simdDst) - stragglers;
        while (stragglers --> 0)
        {
            *d++ = *s++;
        }
    #elif defined(LS_ARCH_ARM)
        std::size_t       stragglers = count % sizeof(uint32x4_t);
        std::size_t       simdCount  = (count-stragglers)/sizeof(uint32x4_t);
        const uint32x4_t* simdSrc    = reinterpret_cast<const uint32x4_t*>(src);
        uint32x4_t*       simdDst    = reinterpret_cast<uint32x4_t*>(dst);

        while (simdCount --> 0)
        {
            vst1q_u32(reinterpret_cast<uint32_t*>(simdDst++), vld1q_u32(reinterpret_cast<const uint32_t*>(simdSrc++)));
        }

        const char* s = reinterpret_cast<const char*>(simdSrc) - stragglers;
        char*       d = reinterpret_cast<char*>(simdDst) - stragglers;
        while (stragglers --> 0)
        {
            *d++ = *s++;
        }
    #else
        char* to = reinterpret_cast<char*>(dst);
        const char* from = reinterpret_cast<const char*>(src);

        if (count)
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
    #if defined(LS_ARCH_X86)
        const __m256i     simdFillByte = _mm256_set1_epi8(fillByte);
        __m256i*          simdTo       = reinterpret_cast<__m256i*>(dst);
        std::size_t       stragglers   = count % sizeof(__m256i);
        std::size_t       simdCount    = (count-stragglers)/sizeof(__m256i);

        // Using stream intrinsics here is OK because we're not reading data
        // from memory
        while (simdCount --> 0)
        {
            _mm256_stream_si256(simdTo++, simdFillByte);
        }

        char* to = reinterpret_cast<char*>(dst) + count - stragglers;
        while (stragglers --> 0)
        {
            *to++ = fillByte;
        }
    #elif defined(LS_ARCH_ARM)
        std::size_t                                stragglers       = count % sizeof(uint32x4_t);
        std::size_t                                simdCount        = (count-stragglers)/sizeof(uint32x4_t);
        const uint32_t                             fillByteU32      = ((uint32_t)fillByte) | (fillByte << 8) | (fillByte << 16) | (fillByte << 24);
        alignas(sizeof(uint32x4_t)) const uint32_t fillByteU32x4[4] = {fillByteU32, fillByteU32, fillByteU32, fillByteU32};
        uint32x4_t*                                simdTo           = reinterpret_cast<uint32x4_t*>(dst);

        while (simdCount --> 0)
        {
            vst1q_u32(reinterpret_cast<uint32_t*>(simdTo++), vld1q_u32(fillByteU32x4));
        }

        char* to = reinterpret_cast<char*>(dst) + count - stragglers;
        while (stragglers --> 0)
        {
            *to++ = fillByte;
        }
    #else
        char* to = reinterpret_cast<char*>(dst);
        if (count)
        {
            LS_UTILS_LOOP_UNROLL_32(count, (*to++ = fillByte))
        }
    #endif

    return dst;
}



} // end ls namespace
