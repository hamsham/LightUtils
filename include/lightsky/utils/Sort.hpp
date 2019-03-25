
#ifndef LS_UTILS_SORT_HPP
#define LS_UTILS_SORT_HPP

#include <atomic>

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
inline void sort_bubble(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Selection Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_selection(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Insertion Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_insertion(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Shell Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_shell(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Merge Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge_iterative(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Quick Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Quick Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick_iterative(data_type* const items, long count) noexcept;



/*-------------------------------------
 * Quick Sort (iterative)
-------------------------------------*/
template <typename data_type, class LessComparator = ls::utils::IsLess<data_type>, class GreaterComparator = ls::utils::IsGreater<data_type>>
void sort_sheared(
    data_type* const items,
    long count,
    std::size_t numThreads,
    std::size_t threadId,
    std::atomic_size_t* numThreadsFinished,
    std::atomic_size_t* numSortPhases) noexcept;



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SortImpl.hpp"

#endif /* LS_UTILS_SORT_HPP */
