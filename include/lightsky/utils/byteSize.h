
#ifndef __LS_UTILS_BYTESIZE_H__
#define __LS_UTILS_BYTESIZE_H__

namespace ls {
namespace utils {

/*-----------------------------------------------------------------------------
 * Byte Counting for Variadic Templates
 * --------------------------------------------------------------------------*/
/**
 *  @brief getArgByteSize
 *  Helper function to sum the number of bytes used by a set of types.
 *
 *  @param size
 *  An unsigned integer representing the size, in bytes, of an object.
 *
 *  @return
 *  An unsigned integer, representing the size, in bytes, of an object. This
 *  can be the size of a built-in type, or user-defined structures.
 */
constexpr
unsigned get_arg_byte_size(unsigned size)
{
    return size;
}

/**
 *  @brief getArgByteSize
 *  Helper function to sum the number of bytes used by a set of types.
 *
 *  @param size
 *  An unsigned integer representing the size, in bytes, of an object.
 *
 *  @param sizeN
 *  A set of unsigned integers that contain the byte size of a set of objects.
 *
 *  @return
 *  An unsigned integer, representing the size, in bytes, of a set of object
 *  types. These types can be built-in, or user-defined structures.
 */
template <typename... integral_t>
constexpr
unsigned get_arg_byte_size(unsigned size, integral_t... sizeN)
{
    return size + get_arg_byte_size(sizeN...);
}

/**
 *  @brief getByteSize
 *  Sum the number of bytes used by a set of types.
 *
 *  @return
 *  An unsigned integer, representing the size, in bytes, of a set of object
 *  types. These types can be built-in, or user-defined structures.
 */
 template <typename... integral_t>
 constexpr
 unsigned get_byte_size()
 {
     return get_arg_byte_size(sizeof(integral_t)...);
 }

} // end utils namespace
} // end ls namespace

#endif /* __LS_UTILS_BYTESIZE_H__ */
