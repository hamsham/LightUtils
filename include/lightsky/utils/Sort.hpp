
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
inline void sort_bubble(data_type* const items, long count);



/*-------------------------------------
 * Selection Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_selection(data_type* const items, long count);



/*-------------------------------------
 * Insertion Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_insertion(data_type* const items, long count);



/*-------------------------------------
 * Shell Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_shell(data_type* const items, long count);



/*-------------------------------------
 * Merge Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge(data_type* const items, long count);



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge_iterative(data_type* const items, long count);



/*-------------------------------------
 * Quick Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick(data_type* const items, long count);



/*-------------------------------------
 * Quick Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick_iterative(data_type* const items, long count);



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SortImpl.hpp"

#endif /* LS_UTILS_SORT_HPP */
