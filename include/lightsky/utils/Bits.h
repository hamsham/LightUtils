/*
 * File:   bytes/bits.h
 * Author: Miles Lacey
 *
 * Created on June 21, 2013, 1:31 AM
 */

#ifndef __LS_UTILS_BITS_H__
#define	__LS_UTILS_BITS_H__

#include <climits>

#ifndef LS_BITS_PER_BYTE
    #define LS_BITS_PER_BYTE CHAR_BIT
#endif

namespace ls {
namespace utils {

/**
 *  @brief BitMask
 *  Convenience structure to facilitate bit acquisition of bytes.
 */
struct alignas(1) BitMask
{
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
    constexpr
    int get(int i) const
    {
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
    inline
    void set(int i, int val)
    {
        byte = (byte & ~(1 << i)) | (val << i);
    }
};

/*-----------------------------------------------------------------------------
 * Functions allowing access to individual bytes
 * --------------------------------------------------------------------------*/
/**
 *  @brief get_byte
 *  Retrieve the Nth byte of a basic data type.
 *
 *  @param key
 *
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
 template <typename key_t>
 constexpr
 const utils::BitMask* utils::get_byte(const key_t* k, unsigned iter)
 {
     return (iter < sizeof(k))
     ?
        reinterpret_cast<const utils::BitMask*>(k) + iter
     :
        nullptr;
 }

/**
 *  @brief get_byte (char string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *
 *  @param key
 *
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <>
constexpr
const utils::BitMask* utils::get_byte(const char* str, unsigned iter)
{
    return (str[iter / sizeof(char)] != '\0')
    ?
        reinterpret_cast<const utils::BitMask*>(str) + iter
    :
        nullptr;
}

/**
 *  @brief get_byte (wchar_t string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *
 *  @param key
 *
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <>
constexpr
const utils::BitMask* utils::get_byte(const wchar_t* str, unsigned iter)
{
    return (str[iter / sizeof(wchar_t)] != '\0')
    ?
        reinterpret_cast<const utils::BitMask*>(str) + iter
    :
        nullptr;
}

/**
 *  @brief get_byte (char16_t string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *
 *  @param key
 *
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <>
constexpr
const utils::BitMask* utils::get_byte(const char16_t* str, unsigned iter)
{
    return (str[iter / sizeof(char16_t)] != '\0')
    ?
        reinterpret_cast<const utils::BitMask*>(str) + iter
    :
        nullptr;
}

/**
 *  @brief get_byte (char32_t string specialization)
 *  Retrieve the Nth byte of a basic data type.
 *
 *  @param key
 *
 *  @param iter
 *  The iterator which specifies the Nth byte in 'key'.
 *
 *  @return The Nth byte in 'key,' specified by 'iter.'
 */
template <>
constexpr
const utils::BitMask* utils::get_byte(const char32_t* str, unsigned iter)
{
    return (str[iter / sizeof(char32_t)] != '\0')
    ?
        reinterpret_cast<const utils::BitMask*>(str) + iter
    :
        nullptr;
}

} // end utils namespace
} // end ls namespace

#endif	/* __LS_UTILS_BITS_H__ */
