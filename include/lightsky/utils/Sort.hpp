
#ifndef LS_UTILS_SORT_HPP
#define LS_UTILS_SORT_HPP

#include <atomic>
#include <cstdio> // long long

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
inline void sort_bubble(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Selection Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_selection(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Insertion Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_insertion(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Shell Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_shell(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Merge Sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge_iterative(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Quick Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Quick Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick_iterative(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Shear Sort (parallel, SLOW)
-------------------------------------*/
template <typename data_type, class LessComparator = ls::utils::IsLess<data_type>, class GreaterComparator = ls::utils::IsGreater<data_type>>
void sort_sheared(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numThreadsFinished,
    std::atomic_llong* numSortPhases,
    LessComparator cmpL = LessComparator{},
    GreaterComparator cmpG = GreaterComparator{}) noexcept;



/*-------------------------------------
    Bitonic Sort (parallel, only for arrays that are powers of 2 in size).
-------------------------------------*/
template <typename data_type, class LessComparator, class GreaterComparator>
void sort_bitonic(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numThreadsFinished,
    std::atomic_llong* numSortPhases,
    LessComparator cmpL = LessComparator{},
    GreaterComparator cmpG = GreaterComparator{}) noexcept;



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SortImpl.hpp"

#endif /* LS_UTILS_SORT_HPP */
