
#ifndef LS_UTILS_SORT_HPP
#define LS_UTILS_SORT_HPP

#include <atomic>
#include <cstdio> // long long

#include "lightsky/setup/Types.h"
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
 * Merge Sort with pre-allocated storage
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge(data_type* const items, data_type* const temp, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
void sort_merge_iterative(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Merge Sort with pre-allocated storage (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_merge_iterative(data_type* const items, data_type* const temp, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Merge Sort (parallel, iterative, buffered)
-------------------------------------*/
template <typename data_type, class Comparator>
void sort_merge_iterative(
    data_type* const items,
    data_type* const temp,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp) noexcept;



/*-------------------------------------
 * Merge Sort (parallel, iterative, unbuffered)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void sort_merge_iterative(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp) noexcept;



/*-------------------------------------
 * Quick Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
inline void sort_quick(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
 * Quick Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
void sort_quick_iterative(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;



/*-----------------------------------------------------------------------------
 * Radix Sort Reference Implementation
 *
 * Note: The "Indexer" template parameter is responsible for turning an input
 * element from "items" into an unsigned long integer hash. This hash will
 * then be used for sorting "items" in ascending order.
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Radix sort adapter for increasing magnitude
-------------------------------------*/
template <typename data_type>
struct RadixIndexerAscending
{
    constexpr unsigned long long operator()(const typename ls::setup::EnableIf<ls::setup::IsIntegral<data_type>::value, data_type>::type& val) const noexcept
    {
        return (unsigned long long)(ls::setup::IsUnsigned<data_type>::value ? val : (val - ~(data_type)0 + (data_type)1));
    }
};

/*-------------------------------------
 * Radix sort adapter for decreasing magnitude
-------------------------------------*/
template <typename data_type>
struct RadixIndexerDescending
{
    constexpr unsigned long long operator()(const typename ls::setup::EnableIf<ls::setup::IsIntegral<data_type>::value, data_type>::type& val) const noexcept
    {
        return ~RadixIndexerAscending<data_type>{}(val);
    }
};

/*-------------------------------------
 * Radix sort
-------------------------------------*/
template <typename data_type, class Indexer = RadixIndexerAscending<data_type>>
inline void sort_radix(data_type* const items, long long count, Indexer indexer = Indexer{}) noexcept;



/*-------------------------------------
 * Radix sort with pre-allocated storage
-------------------------------------*/
template <typename data_type, class Indexer = RadixIndexerAscending<data_type>>
void sort_radix(data_type* const items, data_type* const temp, long long count, Indexer indexer = Indexer{}) noexcept;



/*-------------------------------------
 * Adapter to emulate the radix sort as a comparative numerical sort
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>, class AscendingIndexer = RadixIndexerAscending<data_type>, class DescendingIndexer = RadixIndexerDescending<data_type>>
inline void sort_radix_comparative(data_type* const items, long long count, Comparator cmp) noexcept;



/*-------------------------------------
 * Pre-allocated radix-sort adapter
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>, class AscendingIndexer = RadixIndexerAscending<data_type>, class DescendingIndexer = RadixIndexerDescending<data_type>>
inline void sort_radix_comparative(data_type* const items, data_type* const temp, long long count, Comparator cmp) noexcept;



/*-------------------------------------
 * Shear Sort (parallel, SLOW)
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
void sort_sheared(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
    Bitonic Sort (parallel, only for arrays that are powers of 2 in size).
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
void sort_bitonic(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp = Comparator{}) noexcept;



/*-------------------------------------
    Bitonic Sort (parallel, only for arrays that are powers of 2 in size).
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
void sort_odd_even(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp = Comparator{}) noexcept;



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SortImpl.hpp"

#endif /* LS_UTILS_SORT_HPP */
