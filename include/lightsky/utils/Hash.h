/*
 *  File:   hash.h
 *  Author: Miles Lacey
 *
 *  Created on February 23, 2013, 12:01 PM
 *
 *  These hashing algorithms are designed to be used at compile-time. Keep in
 *  mind that they ARE NOT meant to be used for cryptographic or security
 *  purposes.
 *
 *  The following are some useful compile-time string hashing functions.
 *  Each function has an interface and an implementation.
 *  Please ignore the magic numbers, they were found alongside each algorithm.
 */

#ifndef __LS_UTILS_HASH_H__
#define __LS_UTILS_HASH_H__

#include <cstdint> // uint32_t

namespace ls {
namespace utils {

/**
 *  @brief hash_t
 *  An integral type that's long enough to hold a simple hash value.
 */
typedef uint32_t hash_t;

/**
 *  @brief DJB2 Hashing Function
 *  This hash algorithm was found on here:
 *  http://nguillemot.blogspot.com/2012/06/side-story-compile-time-string-hashing.html
 *
 *  @param str
 *  A pointer to a null-terminated c-style string.
 *
 *  @return an integer-type, representing the hash value using the DJB2
 *  algorithm.
 */
constexpr
hash_t hash_djb2(const char* str);

/**
 *  @brief SDBM Hashing Function
 *  This hash algorithm was found here:
 *  http://www.cse.yorku.ca/~oz/hash.html
 *
 *  @param str
 *  A pointer to a null-terminated c-style string.
 *
 *  @return an integer-type, representing the hash value using the SDBM
 *  algorithm.
 */
constexpr
hash_t hash_sdbm(const char* str);

/**
 *  @brief FNV-1a Hashing Function
 *  This hash algorithm was found here:
 *  http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 *
 *  @param str
 *  A pointer to a null-terminated c-style string.
 *
 *  @return an integer-type, representing the hash value using the FNV-1a
 *  algorithm.
 */
constexpr
hash_t hash_fnv1(const char* str);

/**
 *  @brief CRC32 Hashing function
 *  A simple compile-time hashing function using the CRC32 algorithm.
 *
 *  This method was adapted from a previous implementation on StackOverflow:
 *      http://stackoverflow.com/a/23683218/1217127
 *
 *  @param str
 *  A c-style string that will be hashed.
 *
 *  @param prevCrc
 *  A previous hash value that will be modified by the current string's hash.
 *
 *  @return a 32-bit integer, representing a hashed value of the input string.
 */
constexpr
hash_t hash_crc32(const char* str, hash_t prevCrc = 0xFFFFFFFF);



/*-----------------------------------------------------------------------------
 * Hash Standardization
-----------------------------------------------------------------------------*/
constexpr hash_t string_hash(const char* str) noexcept {
    return hash_fnv1(str);
}

} // end utils namespace
} // end ls namespace

#include "ls/utils/generic/HashImpl.h"

#endif  /* __LS_UTILS_HASH_H__ */
