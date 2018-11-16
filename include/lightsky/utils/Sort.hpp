
#ifndef LS_UTILS_SORT_HPP
#define LS_UTILS_SORT_HPP

#include "lightsky/utils/Algorithm.hpp" // utils::IsLess

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Sorting Algorithms
 *
 * All of the sorting algorithms require at least an implementation of an
 * assignment operator (for temporary storage) and a less-than operator (<).
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Bubble Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void bubble_sort(data_type* const items, long count);



/*-------------------------------------
 * Selection Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void selection_sort(data_type* const items, long count);



/*-------------------------------------
 * Insertion Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void insertion_sort(data_type* const items, long count);



/*-------------------------------------
 * Shell Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void shell_sort(data_type* const items, long count);



/*-------------------------------------
 * Merge Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void merge_sort(data_type* const items, long count);



/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void quick_sort(data_type* const items, long count);



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SortImpl.hpp"

#endif /* LS_UTILS_SORT_HPP */
