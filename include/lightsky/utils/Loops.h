

#ifndef __LS_UTILS_LOOPS_H__
#define __LS_UTILS_LOOPS_H__

#include <cstdlib> // std::size_t



/**
 * Duff's Device for unrolling 8 operations out of a loop.
 */
#ifndef LS_UTILS_LOOP_UNROLL_8
    #define LS_UTILS_LOOP_UNROLL_8(count, op) { \
        const std::size_t numUnrolls = count % 8; \
        std::size_t n = (count + 7) / 8; \
        switch (numUnrolls) { \
            case 0: do {    op; \
            case 7:         op; \
            case 6:         op; \
            case 5:         op; \
            case 4:         op; \
            case 3:         op; \
            case 2:         op; \
            case 1:         op; \
                    } while (--n > 0); \
        } \
    }
# endif /* LS_UTILS_LOOP_UNROLL_8 */



/**
 * Duff's Device for unrolling 16 operations out of a loop.
 */
#ifndef LS_UTILS_LOOP_UNROLL_16
    #define LS_UTILS_LOOP_UNROLL_16(count, op) { \
        const std::size_t numUnrolls = count % 16; \
        std::size_t n = (count + 15) / 16; \
        switch (numUnrolls) { \
            case 0: do {    op; \
            case 15:        op; \
            case 14:        op; \
            case 13:        op; \
            case 12:        op; \
            case 11:        op; \
            case 10:        op; \
            case 9:         op; \
            case 8:         op; \
            case 7:         op; \
            case 6:         op; \
            case 5:         op; \
            case 4:         op; \
            case 3:         op; \
            case 2:         op; \
            case 1:         op; \
                    } while (--n > 0); \
        } \
    }
# endif /* LS_UTILS_LOOP_UNROLL_16 */



/**
 * Duff's Device for unrolling 32 operations out of a loop.
 */
#ifndef LS_UTILS_LOOP_UNROLL_32
    #define LS_UTILS_LOOP_UNROLL_32(count, op) { \
        const std::size_t numUnrolls = count % 32; \
        std::size_t n = (count + 31) / 32; \
        switch (numUnrolls) { \
            case 0: do {    op; \
            case 31:        op; \
            case 30:        op; \
            case 29:        op; \
            case 28:        op; \
            case 27:        op; \
            case 26:        op; \
            case 25:        op; \
            case 24:        op; \
            case 23:        op; \
            case 22:        op; \
            case 21:        op; \
            case 20:        op; \
            case 19:        op; \
            case 18:        op; \
            case 17:        op; \
            case 16:        op; \
            case 15:        op; \
            case 14:        op; \
            case 13:        op; \
            case 12:        op; \
            case 11:        op; \
            case 10:        op; \
            case 9:         op; \
            case 8:         op; \
            case 7:         op; \
            case 6:         op; \
            case 5:         op; \
            case 4:         op; \
            case 3:         op; \
            case 2:         op; \
            case 1:         op; \
                    } while (--n > 0); \
        } \
    }
# endif /* LS_UTILS_LOOP_UNROLL_32 */



#endif /* __LS_UTILS_LOOPS_H__ */
