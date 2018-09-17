
#ifndef LS_UTILS_SORT_HPP
#define LS_UTILS_SORT_HPP

#include "lightsky/utils/Algorithm.hpp" // utils::IsLess

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Sorting Algorithms
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void quick_sort(data_type* const items, long count);



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SortImpl.hpp"

#endif /* LS_UTILS_SORT_HPP */
