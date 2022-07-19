
#ifndef LS_UTILS_COPY_H
#define LS_UTILS_COPY_H

#include <cstdint> // fixed-width data types
#include <utility> // std::move

#include "lightsky/setup/Api.h" // LS_INLINE, LS_RESTRICT_PTR
#include "lightsky/setup/Macros.h" // LS_LIKELY

#include "lightsky/utils/Loops.h"



namespace ls {
namespace utils {



/**
 * Implementation of std::memcpy using loop unrolling.
 * 
 * This implementation copies only bytes of data stored sequentially in memory.
 * It makes no effort to copy data using copy operators or constructors.
 * 
 * @param dest
 * 	A pointer to the memory location to copy to.
 * 
 * @param src
 * 	A pointer to the memory location to copy from.
 * 
 * @param count
 * Specifies the number of bytes which will be copied.
 * 
 * @return The 'dest' parameter.
 */
void* fast_memcpy(void* const dest, const void* const src, const uint_fast64_t count);



/**
 * Implementation of std::copy using loop unrolling.
 * 
 * This implementation uses the function signature of std::memcpy.
 * 
 * @param dest
 * A pointer to the object to fill.
 * 
 * @param src
 * 	A pointer to the memory location to copy data from. This pointer must
 * reference a list of objects with at least the same number of elements as
 * 'dest.'
 * 
 * @param count
 * Specifies the number of items which will be copied.
 */
template <typename dest_t, typename src_t>
inline void fast_copy(dest_t* dest, const src_t* src, uint_fast64_t count)
{
    #if 0
        if (count)
        {
            LS_UTILS_LOOP_UNROLL_32(count, (*dest++ = *src++))
        }
    #else
        while (count--)
        {
            *dest++ = *src++;
        }
    #endif
}



/**
 * Implementation of std::move using loop unrolling.
 * 
 * This implementation uses the function signature of std::memcpy.
 * 
 * @param dest
 * A pointer to the object to fill.
 * 
 * @param src
 * 	A pointer to the memory location to move data from. This pointer must
 * reference a list of objects with at least the same number of elements as
 * 'dest.'
 * 
 * @param count
 * Specifies the number of items which will be moved.
 */
template <typename dest_t, typename src_t>
inline void fast_move(dest_t* dest, src_t* src, uint_fast64_t count)
{
    #if 0
        if (count)
        {
            LS_UTILS_LOOP_UNROLL_32(count, (*dest++ = std::move(*src++)))
        }
    #else
        while (count--)
        {
            *dest++ = std::move(*src++);
        }
    #endif
}



/**
 * Implementation of std::memset using loop unrolling.
 *
 * This version is for setting data which is padded to 8-bytes.
 *
 * @param dest
 * A pointer to the object to fill.
 *
 * @param fillBytes
 * A set of four bytes which will be used to fill the memory between 'dest' and
 * 'dest+count'.
 *
 * @param count
 * Specifies the number of bytes which will be filled.
 *
 * @return The 'dest' parameter.
 */
void* fast_memset_8(void* const dest, const uint64_t fillBytes, const uint_fast64_t count);



/**
 * Implementation of std::memset using loop unrolling.
 *
 * This version is for setting data which is padded to 4-bytes.
 *
 * @param dest
 * A pointer to the object to fill.
 *
 * @param fillBytes
 * A set of four bytes which will be used to fill the memory between 'dest' and
 * 'dest+count'.
 *
 * @param count
 * Specifies the number of bytes which will be filled.
 *
 * @return The 'dest' parameter.
 */
inline LS_INLINE void* fast_memset_4(void* const dest, const uint32_t fillByte, const uint_fast64_t count)
{
    const uint64_t fillBytes = 0ull
        | ((uint64_t)fillByte)
        | ((uint64_t)fillByte << 32ull);
    return fast_memset_8(dest, fillBytes, count);
}



/**
 * Implementation of std::memset using loop unrolling.
 *
 * This version is for setting data which is padded to 4-bytes.
 *
 * @param dest
 * A pointer to the object to fill.
 *
 * @param fillBytes
 * A set of four bytes which will be used to fill the memory between 'dest' and
 * 'dest+count'.
 *
 * @param count
 * Specifies the number of bytes which will be filled.
 *
 * @return The 'dest' parameter.
 */
inline LS_INLINE void* fast_memset_2(void* const dest, const uint16_t fillByte, const uint_fast64_t count)
{
    const uint64_t fillBytes = 0ull
        | ((uint64_t)fillByte)
        | ((uint64_t)fillByte << 16ull)
        | ((uint64_t)fillByte << 32ull)
        | ((uint64_t)fillByte << 48ull);
    return fast_memset_8(dest, fillBytes, count);
}



/**
 * Implementation of std::memset using loop unrolling.
 *
 * @param dest
 * A pointer to the object to fill.
 *
 * @param fillByte
 * A single byte which will be used to fill the memory between 'dest' and
 * 'dest+count'.
 *
 * @param count
 * Specifies the number of bytes which will be filled.
 *
 * @return The 'dest' parameter.
 */
inline LS_INLINE void* fast_memset(void* const dest, const uint8_t fillByte, const uint_fast64_t count)
{
    const uint64_t fillBytes = 0u
        | ((uint64_t)fillByte)
        | ((uint64_t)fillByte << 8ull)
        | ((uint64_t)fillByte << 16ull)
        | ((uint64_t)fillByte << 24ull)
        | ((uint64_t)fillByte << 32ull)
        | ((uint64_t)fillByte << 40ull)
        | ((uint64_t)fillByte << 48ull)
        | ((uint64_t)fillByte << 56ull);
    return fast_memset_8(dest, fillBytes, count);
}



/**
 * Implementation of std::fill using loop unrolling.
 * 
 * This implementation uses the function signature of std::memset.
 * 
 * @param dest
 * A pointer to the object to fill.
 * 
 * @param fillType
 * A data type which can be copied to the elements of 'dest' using a copy
 * operator.
 * 
 * @param count
 * Specifies the number of items which will be filled.
 */
template <typename dest_t, typename fill_t>
inline LS_INLINE void fast_fill(dest_t* LS_RESTRICT_PTR dest, const fill_t& fillType, uint_fast64_t count)
{
    const dest_t* LS_RESTRICT_PTR const end = dest + count;

    #if 0
        while (dest != end)
        {
            LS_UTILS_LOOP_UNROLL_32(count, (*dest++ = fillType))
        }

    #else
        while (LS_LIKELY(dest != end))
        {
            *dest++ = fillType;
        }
    #endif
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_COPY_H */
