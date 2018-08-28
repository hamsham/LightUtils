
#ifndef LS_UTILS_BYTESIZE_H
#define LS_UTILS_BYTESIZE_H

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Byte Counting for Variadic Templates
 * --------------------------------------------------------------------------*/
/*
template <typename data_t>
constexpr unsigned num_bytes()
{
    return sizeof(data_t);
}
*/



/**
 *  @brief num_bytes
 *  Helper function to sum the number of bytes used by a set of types.
 *
 *  @return
 *  An unsigned integer, representing the size, in bytes, of a set of object
 *  types. These types can be built-in, or user-defined structures.
 */
/*
template <typename arg_t, typename other_arg_t>
constexpr unsigned num_bytes()
{
    return sizeof(arg_t) + sizeof(other_arg_t);
}
*/


template <typename... args_t>
struct NumBytes;



template <>
struct NumBytes <>
{
    static constexpr unsigned value()
    {
        return 0;
    }
};



template <typename arg_t>
struct NumBytes <arg_t>
{
    static constexpr unsigned value()
    {
        return sizeof(arg_t);
    }
};



template <typename arg_t, typename... args_t>
struct NumBytes <arg_t, args_t...>
{
    static constexpr unsigned value()
    {
        return sizeof(arg_t) + NumBytes<args_t...>::value();
    }
};



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_BYTESIZE_H */
