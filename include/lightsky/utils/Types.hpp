/*
 * File:   Types.hpp
 * Author: Miles Lacey
 *
 * This file mimics limited functionality of the standard C++ header
 * <type_traits>.
 */

#ifndef LS_UTILS_TYPES_H
#define LS_UTILS_TYPES_H

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Helper types to reduce implementation boilerplace
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * EnableIf True Helper
-------------------------------------*/
template <typename data_t>
struct TrueType
{
    typedef data_t value_type;

    static constexpr bool value = true;

    constexpr explicit operator bool() const noexcept
    {
        return true;
    }

    constexpr bool operator() () const noexcept
    {
        return true;
    }
};



/*-------------------------------------
 * EnableIf False Helper
-------------------------------------*/
template <typename data_t>
struct FalseType
{
    typedef data_t value_type;

    static constexpr bool value = false;

    constexpr explicit operator bool() const noexcept
    {
        return false;
    }

    constexpr bool operator() () const noexcept
    {
        return false;
    }
};



/*-----------------------------------------------------------------------------
 * Implementation of std::enable_if
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Base Case
-------------------------------------*/
template<bool val, class T = void>
struct EnableIf
{
};



/*-------------------------------------
 * True Case
-------------------------------------*/
template<class T>
struct EnableIf<true, T>
{
    typedef T type;
};



/*-----------------------------------------------------------------------------
 * Implementation of RemoveReference
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Base Case
-------------------------------------*/
template<typename T>
struct RemoveReference
{
    typedef T type;
};



/*-------------------------------------
 * Reference Case
-------------------------------------*/
template<typename T>
struct RemoveReference<T&>
{
    typedef T type;
};



/*-------------------------------------
 * R-Value Reference
-------------------------------------*/
template<typename T>
struct RemoveReference<T&&>
{
    typedef T type;
};



/*-----------------------------------------------------------------------------
 * Implementation of std::forward
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Reference Forwarding
-------------------------------------*/
template<typename T>
constexpr T&& forward(typename RemoveReference<T>::type& val) noexcept
{
    return static_cast<T&&>(val);
}



/*-------------------------------------
 * R-Value Reference Forwarding
-------------------------------------*/
template<typename T>
constexpr T&& forward(typename RemoveReference<T>::type&& val) noexcept
{
    return static_cast<T&&>(val);
}



/*-------------------------------------
 * Reference Moving
-------------------------------------*/
template<typename T>
constexpr typename RemoveReference<T>::type&& move(T&& val) noexcept
{
    return static_cast<typename RemoveReference<T>::type&&>(val);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_TYPES_H */
