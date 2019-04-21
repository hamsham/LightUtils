
#ifndef LS_UTILS_ALGORITHM_HPP
#define LS_UTILS_ALGORITHM_HPP

#include "lightsky/setup/Api.h" // LS_IMPERATIVE



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Comparators
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
struct IsGreater
{
    constexpr bool LS_IMPERATIVE operator() (const data_type& a, const data_type& b) const noexcept;
};



/*-------------------------------------
 * Less Than
-------------------------------------*/
template <typename data_type>
struct IsLess
{
    constexpr bool LS_IMPERATIVE operator() (const data_type& a, const data_type& b) const noexcept;
};



/*-------------------------------------
 * Greater Than or Equal
-------------------------------------*/
template <typename data_type>
struct IsGreaterOrEqual
{
    constexpr bool LS_IMPERATIVE operator() (const data_type& a, const data_type& b) const noexcept;
};



/*-------------------------------------
 * Less Than or Equal
-------------------------------------*/
template <typename data_type>
struct IsLessOrEqual
{
    constexpr bool LS_IMPERATIVE operator() (const data_type& a, const data_type& b) const noexcept;
};



/*-------------------------------------
 * Equal
-------------------------------------*/
template <typename data_type>
struct IsEqual
{
    constexpr bool LS_IMPERATIVE operator() (const data_type& a, const data_type& b) const noexcept;
};



/*-------------------------------------
 * Not Equal
-------------------------------------*/
template <typename data_type>
struct IsNotEqual
{
    constexpr bool LS_IMPERATIVE operator() (const data_type& a, const data_type& b) const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/AlgorithmImpl.hpp"

#endif /* LS_UTILS_ALGORITHM_HPP */
