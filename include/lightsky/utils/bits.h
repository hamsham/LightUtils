/* 
 * File:   bytes/bits.h
 * Author: Miles Lacey
 *
 * Created on June 21, 2013, 1:31 AM
 */

#ifndef __LS_UTILS_BITS_H__
#define	__LS_UTILS_BITS_H__

#include <climits>

#include "lightsky/utils/assertions.h"

#ifndef LS_BITS_PER_BYTE
    #define LS_BITS_PER_BYTE CHAR_BIT
#endif

namespace ls {
namespace utils {

/**
 *  @brief bitMask
 *  Convenience structure to facilitate bit acquisition of bytes.
 */
struct alignas(1) bitMask {
    /**
     *  @brief byte
     *  A single (hopefully cross-platform) representation of a byte.
     */
    char byte = 0;
    
    /**
     *  @brief get
     *  Get the Ith bit of 'byte.'
     *  
     *  @param i
     *  An iterative value, representing what bit in 'byte' should be returned.
     *  
     *  @return A single binary value representing the Ith bit of *this.
     */
    constexpr int get(int i) const {
        return (byte >> i) & 1;
    }
    
    /**
     *  @brief set
     *  Set the Ith bit in *this.
     *  
     *  @param i
     *  An iterative value, representing what bit in 'byte' should be toggled.
     *  
     *  @param val
     *  A binary value, representing whether 'byte' should be toggled to 1 or
     *  0.
     */
    inline void set(int i, int val) {
        byte = (byte & ~(1 << i)) | (val << i);
    }
};

/*-----------------------------------------------------------------------------
    Functions allowing access to individual bytes
-----------------------------------------------------------------------------*/
/**
 *  @brief getByte
 *  Retrieve the Nth byte of a basic data type.
 *  
 *  @param key
 *  
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *  
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <typename key_t> constexpr
const bitMask* getByte(const key_t* key, unsigned iter);

/**
 *  @brief getByte (char string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *  
 *  @param key
 *  
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *  
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <> constexpr
const bitMask* getByte(const char* key, unsigned iter);

/**
 *  @brief getByte (wchar_t string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *  
 *  @param key
 *  
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *  
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <> constexpr
const bitMask* getByte(const wchar_t* key, unsigned iter);

/**
 *  @brief getByte (char16_t string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *  
 *  @param key
 *  
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *  
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <> constexpr
const bitMask* getByte(const char16_t* key, unsigned iter);

/**
 *  @brief getByte (char32_t string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *  
 *  @param key
 *  
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *  
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <> constexpr
const bitMask* getByte(const char32_t* key, unsigned iter);

} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/bits_impl.h"

#endif	/* __LS_UTILS_BITS_H__ */
