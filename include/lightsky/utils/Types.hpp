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




} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_TYPES_H */
