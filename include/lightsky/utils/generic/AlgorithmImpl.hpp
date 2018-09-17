
namespace ls
{
namespace utils
{



/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
constexpr bool IsGreater<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a > b;
}



/*-------------------------------------
 * Less Than
-------------------------------------*/
template <typename data_type>
constexpr bool IsLess<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a < b;
}



/*-------------------------------------
 * Greater Than or Equal
-------------------------------------*/
template <typename data_type>
constexpr bool IsGreaterOrEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a >= b;
}



/*-------------------------------------
 * Less Than or Equal
-------------------------------------*/
template <typename data_type>
constexpr bool IsLessOrEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a <= b;
}



/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
constexpr bool IsEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a == b;
}



/*-------------------------------------
 * Greater Than
-------------------------------------*/
template <typename data_type>
constexpr bool IsNotEqual<data_type>::operator() (const data_type& a, const data_type& b) const noexcept
{
    return a != b;
}



} // end utils namespace
} // end ls namespace
