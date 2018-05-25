
#ifndef LS_UTILS_COPY_H
#define LS_UTILS_COPY_H

#include <cstdlib> // std::size_t
#include <utility> // std::move

#include "lightsky/setup/Api.h"

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
void* LS_API fast_memcpy(void* const dest, const void* const src, const std::size_t count);



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
void LS_API fast_copy(dest_t* dest, const src_t* src, const std::size_t count) {
    LS_UTILS_LOOP_UNROLL_16(count, (*dest++ = *src++))
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
void LS_API fast_move(dest_t* dest, src_t* src, const std::size_t count) {
    LS_UTILS_LOOP_UNROLL_16(count, (*dest++ = std::move(*src++)))
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
void* LS_API fast_memset(void* const dest, const unsigned char fillByte, const std::size_t count);



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
void LS_API fast_fill(dest_t* dest, const fill_t& fillType, const std::size_t count) {
    LS_UTILS_LOOP_UNROLL_16(count, (*dest++ = fillType))
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_COPY_H */
