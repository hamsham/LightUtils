
#ifndef LS_UTILS_ALGORITHM_IMPL_HPP
#define LS_UTILS_ALGORITHM_IMPL_HPP

namespace ls
{
namespace utils
{



/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
constexpr bool LS_IMPERATIVE IsGreater<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a > b;
}



/*-------------------------------------
 * Less Than
-------------------------------------*/
template <typename data_type>
constexpr bool LS_IMPERATIVE IsLess<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a < b;
}



/*-------------------------------------
 * Greater Than or Equal
-------------------------------------*/
template <typename data_type>
constexpr bool LS_IMPERATIVE IsGreaterOrEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a >= b;
}



/*-------------------------------------
 * Less Than or Equal
-------------------------------------*/
template <typename data_type>
constexpr bool LS_IMPERATIVE IsLessOrEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a <= b;
}



/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
constexpr bool LS_IMPERATIVE IsEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a == b;
}



/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
constexpr bool LS_IMPERATIVE IsNotEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a != b;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_ALGORITHM_IMPL_HPP */
