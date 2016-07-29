/*
 * File:   bytes/endian.h
 * Author: Miles Lacey
 *
 * This file contains methods to determine the current system's endian order at
 * compile-time. This is preferred over using the preprocessor as macros may
 * need to be added and updated over time.
 */

#ifndef __LS_UTILS_ENDIAN_H__
#define __LS_UTILS_ENDIAN_H__

#include <cstdint>

namespace ls {
namespace utils {



/**
 * This enumeration can be placed into templated objects in order to generate
 * compile-time code based on a program's target endianness.
 *
 * The values placed in this enum are used just in case the need arises in
 * order to manually compare them against the number order in the
 * endianValues[] array.
 */
enum endian_t : uint32_t {
    LS_LITTLE_ENDIAN = 0x00000001,
    LS_BIG_ENDIAN = 0x01000000,
    LS_PDP_ENDIAN = 0x00010000,
    LS_UNKNOWN_ENDIAN = 0xFFFFFFFF
};

/**
 * A simple function that can be used to help determine a program's endianness
 * at compile-time.
 */
constexpr endian_t get_endian_order() {
    return ((0xFFFFFFFF & 1) == LS_LITTLE_ENDIAN)
        ? LS_LITTLE_ENDIAN
        : ((0xFFFFFFFF & 1) == LS_BIG_ENDIAN)
            ? LS_BIG_ENDIAN
            : ((0xFFFFFFFF & 1) == LS_PDP_ENDIAN)
                ? LS_PDP_ENDIAN
                : LS_UNKNOWN_ENDIAN;
}

/**
 * Swap the bytes of an unsigned 16-bit integral type between big and little
 * endian representation. This function can be used at compile-time.
 *
 * @param n
 * an unsigned integral type.
 *
 * @return uint16_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
constexpr
uint16_t btol(uint16_t n) {
    return (n >> 8) ^ (n << 8);
}

/**
 * Swap the bytes of a signed 16-bit integral type between big and little
 * endian representation. This function can be used at compile-time.
 *
 * @param n
 * a signed integral type.
 *
 * @return int16_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
constexpr
int16_t btol(int16_t n) {
    return (int16_t) btol((uint16_t) n);
}

/**
 * Swap the bytes of an unsigned 32-bit integral type between big and little
 * endian representation. This function can be used at compile-time.
 *
 * @param n
 * an unsigned integral type.
 *
 * @return uint32_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
constexpr
uint32_t btol(uint32_t n) {
    return
    (0x000000FF & (n >> 24)) ^
        (0x0000FF00 & (n >> 8)) ^
        (0x00FF0000 & (n << 8)) ^
        (0xFF000000 & (n << 24));
}

/**
 * Swap the bytes of a signed 32-bit integral type between big and little
 * endian representation. This function can be used at compile-time.
 *
 * @param n
 * a signed integral type.
 *
 * @return int32_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
constexpr
int32_t btol(int32_t n) {
    return (int32_t) btol((uint32_t) n);
}

/**
 * Swap the bytes of an unsigned 64-bit integral type between big and little
 * endian representation. This function can be used at compile-time.
 *
 * @param n
 * an unsigned integral type.
 *
 * @return uint64_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
constexpr
uint64_t btol(uint64_t n) {
    return
    (0x00000000000000FF & (n >> 56)) ^
        (0x000000000000FF00 & (n >> 40)) ^
        (0x0000000000FF0000 & (n >> 24)) ^
        (0x00000000FF000000 & (n >> 8)) ^
        (0x000000FF00000000 & (n << 8)) ^
        (0x0000FF0000000000 & (n << 24)) ^
        (0x00FF000000000000 & (n << 40)) ^
        (0xFF00000000000000 & (n << 56));
}

/**
 * Swap the bytes of a signed 64-bit integral type between big and little
 * endian representation. This function can be used at compile-time.
 *
 * @param n
 * a signed integral type.
 *
 * @return int64_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
constexpr
int64_t btol(int64_t n) {
    return (int64_t) btol((uint64_t) n);
}

/**
 * Swap the bytes of a POD type between big and little endian representation.
 *
 * @param n
 * A plain-old-data type.
 *
 * @return num_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
template <typename num_t>
num_t btol(num_t n) {
    num_t ret;
    unsigned char* const pIn = (unsigned char* const) &n;
    unsigned char* const pOut = (unsigned char* const) &ret;

    for (unsigned i = 0, j = sizeof (num_t) - 1; i < sizeof (num_t); ++i, --j) {
        pOut[i] = pIn[j];
    }

    return ret;
}

} /* end utils namespace */
} /* end ls namespace */

/**
 * Macro that should be used to always get the endianness of the current build
 * target.
 */
#define LS_ENDIANNESS ls::utils::get_endian_order()

#endif /* __LS_UTILS_ENDIAN_H__ */
