
#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Compiler.h"
#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Copy.h"

#ifdef LS_COMPILER_MSC
    #include <intrin.h>
#endif

#ifdef LS_ARCH_X86
    #include <immintrin.h>
#elif defined(LS_ARM_NEON)
    #include <arm_neon.h>
#endif



namespace ls {



/*-------------------------------------
 * fast_memcpy
-------------------------------------*/
void* utils::fast_memcpy(void* const LS_RESTRICT_PTR dst, const void* const LS_RESTRICT_PTR src, const uint_fast64_t count)
{
    #if defined(LS_ARCH_X86) && defined(LS_COMPILER_GNU)
        const uint64_t* simdSrc    = reinterpret_cast<const uint64_t*>(src);
        long long*      simdDst    = reinterpret_cast<long long*>(dst);
        uint_fast64_t   simdCount  = count % sizeof(uint64_t);
        uint_fast64_t   iterations = count / sizeof(uint64_t);

        __asm__ volatile(
            "MOV %[count], %%rcx;"
            "MOV %[in], %%rsi;"
            "MOV %[out], %%rdi;"
            "REP movsq;"
            "MOV %%rsi, %[in];"
            "MOV %%rdi, %[out];"
            : [out]"+r"(simdDst), [in]"+r"(simdSrc)
            : [count]"r"(iterations)
            : "rcx", "rdi", "rsi", "memory"
        );

    #elif defined(LS_X86_SSE2)
        const uint64_t* simdSrc    = reinterpret_cast<const uint64_t*>(src);
        long long*      simdDst    = reinterpret_cast<long long*>(dst);
        uint_fast64_t   simdCount  = count;

        while (simdCount >= sizeof(uint64_t)*4)
        {
            LS_PREFETCH(simdSrc+32, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_NONTEMPORAL);

            _mm_stream_si64(simdDst+0, simdSrc[0]);
            _mm_stream_si64(simdDst+1, simdSrc[1]);
            _mm_stream_si64(simdDst+2, simdSrc[2]);
            _mm_stream_si64(simdDst+3, simdSrc[3]);

            simdSrc += 4;
            simdDst += 4;
            simdCount -= sizeof(uint64_t)*4;
        }

    #elif defined(LS_ARCH_AARCH64)
        const uint64_t* simdSrc   = reinterpret_cast<const uint64_t*>(src);
        uint64_t*       simdDst   = reinterpret_cast<uint64_t*>(dst);
        uint_fast64_t   simdCount = count;

        while (simdCount >= sizeof(uint64_t)*4)
        {
            LS_PREFETCH(simdSrc+32, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_NONTEMPORAL);

            __asm__ volatile(
                "LDNP x0, x1, [%[in]]\n"
                "STNP x0, x1, [%[out]]\n"
                "LDNP x0, x1, [%[in], #16]\n"
                "STNP x0, x1, [%[out], #16]\n"
                "ADD %[in], %[in], #32\n"
                "ADD %[out], %[out], #32\n"
                "SUB %[count], %[count], #32"
                : [out]"+r"(simdDst), [in]"+r"(simdSrc), [count]"+r"(simdCount)
                :
                : "x0", "x1", "memory"
            );
        }

    #elif defined(LS_ARM_NEON)
        const uint32x4_t* simdSrc    = reinterpret_cast<const uint32x4_t*>(src);
        uint32x4_t*       simdDst    = reinterpret_cast<uint32x4_t*>(dst);
        uint_fast64_t     simdCount  = count;

        while (simdCount >= sizeof(uint32x4_t))
        {
            LS_PREFETCH(simdSrc+16, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_NONTEMPORAL);
            vst1q_u32(reinterpret_cast<uint32_t*>(simdDst++), vld1q_u32(reinterpret_cast<const uint32_t*>(simdSrc++)));
            simdCount -= sizeof(uint32x4_t);
        }

    #else
        const uint_fast64_t* simdSrc    = reinterpret_cast<const uint_fast64_t*>(src);
        uint_fast64_t*       simdDst    = reinterpret_cast<uint_fast64_t*>(dst);
        uint_fast64_t        iterations = count / (sizeof(uint64_t)*4);
        uint_fast64_t        simdCount  = count % (sizeof(uint64_t)*4);

        // Using stream intrinsics here is NOT OK because we need the cache for
        // reading "simdSrc."
        while (iterations--)
        {
            simdDst[0] = simdSrc[0];
            simdDst[1] = simdSrc[1];
            simdDst[2] = simdSrc[2];
            simdDst[3] = simdSrc[3];
            simdSrc += 4;
            simdDst += 4;
        }

    #endif

    const uint8_t* s = reinterpret_cast<const uint8_t*>(simdSrc);
    uint8_t*       d = reinterpret_cast<uint8_t*>(simdDst);

    #if defined(LS_ARCH_X86) && defined(LS_COMPILER_GNU)
        __asm__ volatile(
            "MOV %[count], %%rcx;"
            "MOV %[in], %%rsi;"
            "MOV %[out], %%rdi;"
            "REP movsb;"
            :
            : [out]"r"(d), [in]"r"(s), [count]"r"(simdCount)
            : "rcx", "rdi", "rsi", "memory"
        );

    #elif defined(LS_ARCH_AARCH64)
        __asm__ volatile(
            ".MemcpyStart:\n"
            "CMP %[count], #0\n"
            "B.EQ .MemcpyEnd\n"
            "LDRB w0, [%[in]]\n"
            "STRB w0, [%[out]]\n"
            "ADD %[in], %[in], #1\n"
            "ADD %[out], %[out], #1\n"
            "SUB %[count], %[count], #1\n"
            "B .MemcpyStart\n"
            ".MemcpyEnd:"
            : [out]"+r"(d), [in]"+r"(s), [count]"+r"(simdCount)
            :
            : "w0", "memory"
        );

    #elif defined(LS_ARCH_ARM)
        __asm__ volatile(
            ".MemcpyStart:\n"
            "CMP %[count], #0\n"
            "BEQ .MemcpyEnd\n"
            "LDRB r0, [%[in]]\n"
            "STRB r0, [%[out]]\n"
            "ADD %[in], %[in], #1\n"
            "ADD %[out], %[out], #1\n"
            "SUB %[count], %[count], #1\n"
            "B .MemcpyStart\n"
            ".MemcpyEnd:"
            : [out]"+r"(d), [in]"+r"(s), [count]"+r"(simdCount)
            :
            : "r0", "memory"
        );

    #else
        while (simdCount--)
        {
            *d++ = *s++;
        }
    #endif

    return dst;
}



/*-------------------------------------
 * fast_memset of 8-bytes at a time
-------------------------------------*/
void* utils::fast_memset_8(void* dst, const uint64_t fillBytes, uint_fast64_t count)
{
    #if defined(LS_ARCH_X86) && defined(LS_COMPILER_GNU)
        uint_fast64_t stragglers = count % sizeof(uint64_t);
        uint_fast64_t simdCount  = count / sizeof(uint64_t);
        long long*    simdTo     = reinterpret_cast<long long*>(dst);

        __asm__ volatile(
            "MOV %[count], %%rcx;"
            "MOV %[in], %%rax;"
            "MOV %[out], %%rdi;"
            "REP stosq;"
            "MOV %%rdi, %[out];"
            : [out]"+r"(simdTo)
            : [in]"r"(fillBytes), [count]"r"(simdCount)
            : "rax", "rcx", "rdi", "memory"
        );

    #elif defined(LS_X86_SSE2)
        const uint_fast64_t simdCount    = count >> 5;
        const uint_fast64_t stragglers   = count - (count & ~31);
        const int64_t       simdFillByte = (int64_t)(fillBytes);
        long long*          simdTo       = reinterpret_cast<long long*>(dst);
        const long long*    simdEnd      = reinterpret_cast<const long long*>(dst) + simdCount * 4;

        while (LS_LIKELY(simdTo != simdEnd))
        {
            _mm_stream_si64(simdTo+0, simdFillByte);
            _mm_stream_si64(simdTo+1, simdFillByte);
            _mm_stream_si64(simdTo+2, simdFillByte);
            _mm_stream_si64(simdTo+3, simdFillByte);
            simdTo += 4;
        }

        // fencing input data is the job of the caller. Fencing output data
        // due to NT stores here is our job.
        _mm_sfence();

    #elif defined(LS_ARCH_AARCH64)
        const uint_fast64_t simdCount    = count >> 5;
        const uint_fast64_t stragglers   = count - (count & ~31);
        const uint64_t      fillByteSimd = fillBytes;
        uint64_t*           simdTo       = reinterpret_cast<uint64_t*>(dst);
        const uint64_t*     simdEnd      = reinterpret_cast<const uint64_t*>(dst) + simdCount * 4;

        __asm__ volatile(
            ".LoopStart:\n"
            "CMP %[out], %[end]\n"
            "B.EQ .LoopEnd\n"
            "STNP %[in], %[in], [%[out]]\n"
            "STNP %[in], %[in], [%[out], #16]\n"
            "ADD %[out], %[out], #32\n"
            "B .LoopStart\n"
            ".LoopEnd:"
            : [out]"+r"(simdTo)
            : [in]"r"(fillByteSimd), [end]"r"(simdEnd)
            : "memory"
        );

    #elif defined(LS_ARCH_ARM)
        const uint_fast64_t simdCount    = count >> 5;
        const uint_fast64_t stragglers   = count - (count & ~31);
        const uint64_t      fillByteSimd = fillBytes;
        uint64_t*           simdTo       = reinterpret_cast<uint64_t*>(dst);
        const uint64_t*     simdEnd      = reinterpret_cast<const uint64_t*>(dst) + simdCount * 4;

        __asm__ volatile(
            ".LoopStart:\n"
            "CMP %[out], %[end]\n"
            "BEQ .LoopEnd\n"
            "STR %[in], [%[out]]\n"
            "STR %[in], [%[out], #8]\n"
            "STR %[in], [%[out], #16]\n"
            "STR %[in], [%[out], #24]\n"
            "ADD %[out], %[out], #32\n"
            "B .LoopStart\n"
            ".LoopEnd:"
            : [out]"+r"(simdTo)
            : [in]"r"(fillByteSimd), [end]"r"(simdEnd)
            : "memory"
        );

    #else
        const uint_fast64_t simdCount    = count >> 5;
        const uint_fast64_t stragglers   = count - (count & ~31);
        const uint64_t      simdFillByte = (uint64_t)(fillBytes);
        uint64_t*           simdTo       = reinterpret_cast<uint64_t*>(dst);
        const uint64_t*     simdEnd      = reinterpret_cast<uint64_t*>(dst) + simdCount * 4;

        while (LS_LIKELY(simdTo != simdEnd))
        {
            simdTo[0] = simdFillByte;
            simdTo[1] = simdFillByte;
            simdTo[2] = simdFillByte;
            simdTo[3] = simdFillByte;
            simdTo += 4;
        }

    #endif

    const uint64_t fillArray[8] = {fillBytes, fillBytes, fillBytes, fillBytes, fillBytes, fillBytes, fillBytes, fillBytes};
    uint8_t*       from         = reinterpret_cast<uint8_t*>(simdTo);
    const uint8_t* to           = reinterpret_cast<uint8_t*>(simdTo) + stragglers;
    const uint8_t* fillByte     = reinterpret_cast<const uint8_t*>(fillArray);

    #if defined(LS_ARCH_X86) && defined(LS_COMPILER_GNU)
        (void)to;

        __asm__ volatile(
            "MOV %[count], %%rcx;"
            "MOV %[in], %%rsi;"
            "MOV %[out], %%rdi;"
            "REP movsb;"
            :
            : [out]"r"(from), [in]"r"(fillByte), [count]"r"(stragglers)
            : "rcx", "rdi", "rsi", "memory"
        );

    #elif defined(LS_ARCH_AARCH64)
        __asm__ volatile(
            ".LoopStart1:\n"
            "CMP %[out], %[end]\n"
            "B.EQ .LoopEnd1\n"
            "LDR w0, [%[in]]\n"
            "STRB w0, [%[out]]\n"
            "ADD %[in], %[in], #1\n"
            "ADD %[out], %[out], #1\n"
            "B .LoopStart1\n"
            ".LoopEnd1:"
            : [out]"+r"(from), [in]"+r"(fillByte)
            : [end]"r"(to)
            : "w0", "memory"
        );

    #elif defined(LS_ARCH_ARM)
        __asm__ volatile(
            ".LoopStart1:\n"
            "CMP %[out], %[end]\n"
            "BEQ .LoopEnd1\n"
            "LDRB r0, [%[in]]\n"
            "STRB r0, [%[out]]\n"
            "ADD %[in], %[in], #1\n"
            "ADD %[out], %[out], #1\n"
            "B .LoopStart1\n"
            ".LoopEnd1:"
            : [out]"+r"(from), [in]"+r"(fillByte)
            : [end]"r"(to)
            : "r0", "memory"
        );

    #else
        while (LS_LIKELY(from != to))
        {
            *from++ = *fillByte++;
        }
    #endif

    return dst;
}



} // end ls namespace
