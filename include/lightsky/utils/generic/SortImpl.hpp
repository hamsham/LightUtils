
#ifndef LS_UTILS_SORT_IMPL_HPP
#define LS_UTILS_SORT_IMPL_HPP

#include <cstdio>
#include <climits> // CHAR_BIT

#include "lightsky/setup/CPU.h"

#include "lightsky/utils/Pointer.h" // Pointer<>

#ifdef LS_ARCH_X86
    #include <immintrin.h> // _tzcnt_u64

    #ifdef LS_COMPILER_GNU
        #include <x86intrin.h> // __builtin_ctzll
    #elif defined(LS_COMPILER_MSC)
        #include <intrin.h>
    #endif
#endif



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Implementations
-----------------------------------------------------------------------------*/
namespace impl
{

inline long long log2 (long long val)
{
    if (val <= 1) return 0;

    unsigned ret = 0;
    while (val > 1)
    {
        val >>= 1;
        ret++;
    }

    return (long long)ret;
}



template <typename IntegralType, class Comparator>
inline LS_INLINE void sort_swap_minmax(IntegralType& a0, IntegralType& b0, Comparator cmp)  noexcept
{
    const IntegralType mask = -((IntegralType)cmp(a0, b0));
    const long long a  = (long long)a0;
    const long long b  = (long long)b0;

    const long long al = ~mask & a;
    const long long bg = ~mask & b;
    const long long bl = mask & b;
    const long long ag = mask & a;

    a0 = (IntegralType)(al | bl);
    b0 = (IntegralType)(ag | bg);
}



template <typename IntegralType, class LessComparator, class GreaterComparator>
inline LS_INLINE void bitonic_swap_minmax(IntegralType& a0, IntegralType& b0, long long cmpLess, LessComparator cmpL, GreaterComparator cmpG)  noexcept
{
    const IntegralType mask = -((IntegralType)(cmpLess ? cmpL(a0, b0) : cmpG(a0, b0)));
    const long long a  = (long long)a0;
    const long long b  = (long long)b0;

    const long long al = ~mask & a;
    const long long bg = ~mask & b;
    const long long ag = mask & a;
    const long long bl = mask & b;

    a0 = (IntegralType)(al | bl);
    b0 = (IntegralType)(ag | bg);
}



/*-------------------------------------
 * Insertion sort that's meant to be used specifically with a shell sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void sort_shell_insert(data_type* const items, const long long count, const long long increment, Comparator cmp) noexcept
{
    for (long long i = increment; i < count; i += increment)
    {
        long long j = i;
        long long k = j-increment;

        while ((j >= increment) && cmp(items[j], items[k]))
        {
            const data_type temp = items[j];
            items[j] = items[k];
            items[k] = temp;

            j -= increment;
            k = j-increment;
        }
    }
}



/*-------------------------------------
 * Insertion sort that's meant to be used specifically with a shell sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void sort_shell_insert(data_type* const items, const long long count, Comparator cmp) noexcept
{
    for (long long i = 1; i < count; i += 1)
    {
        long long j = i;
        long long k = j-1;

        while ((j >= 1) && cmp(items[j], items[k]))
        {
            const data_type temp = items[j];
            items[j] = items[k];
            items[k] = temp;

            j -= 1;
            k = j-1;
        }
    }
}



/*-------------------------------------
 * Recursive Merge Sort
-------------------------------------*/
template <typename data_type, class Comparator>
void sort_merge_impl(data_type* const items, data_type* const temp, long long left, long long right, Comparator cmp) noexcept
{
    if (right-left < 1)
    {
        return;
    }

    long long i, j, k;
    const long long mid = (left+right) >> 1;
    const long long rightMid = right-mid;

    sort_merge_impl<data_type, Comparator>(items, temp, left, mid, cmp);
    sort_merge_impl<data_type, Comparator>(items, temp, mid+1, right, cmp);

    for (i = mid; i >= left; --i)
    {
        temp[i] = items[i];
    }

    for (j = 1; j <= rightMid; ++j)
    {
        temp[right-j+1] = items[j+mid];
    }

    for (i = left, j = right, k = left; k <= right; ++k)
    {
        if (cmp(temp[i], temp[j]))
        {
            items[k] = temp[i++];
        }
        else
        {
            items[k] = temp[j--];
        }
    }
}



/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void sort_quick_impl(data_type* const items, const long long l, const long long r, Comparator cmp) noexcept
{
    data_type temp;

    if (r <= l)
    {
        return;
    }

    if (r-l < (long long)(CHAR_BIT*sizeof(long long)))
    {
        ls::utils::sort_insertion<data_type, Comparator>(items + l, (r-l) + 1, cmp);
        return;
    }

    //const long long pivotIndex = (l+r)/2l;
    const long long pivotIndex = (l+r) >> 1l;

    temp = items[pivotIndex];
    items[pivotIndex] = items[r];
    items[r] = temp;

    long long m = l - 1;
    long long n = r;
    const data_type pivot = items[r];

    do
    {
        while (cmp(items[++m], pivot));
        while ((m < n) && cmp(pivot, items[--n]));

        temp     = items[m];
        items[m] = items[n];
        items[n] = temp;
    } while (m < n);

    temp    = items[m];
    items[m] = items[r];
    items[r] = temp;

    sort_quick_impl<data_type, Comparator>(items, l, m - 1, cmp);
    sort_quick_impl<data_type, Comparator>(items, m + 1, r, cmp);
}



/*-------------------------------------
 * Quick Sort Partitioning
-------------------------------------*/
template <typename data_type, class Comparator>
inline long long sort_quick_partition(data_type* const items, const long long l, const long long r, Comparator cmp) noexcept
{
    data_type temp;
    long long i;
    long long mid = (l + r) >> 1;

    temp = items[mid];
    items[mid] = items[l];
    items[l] = temp;

    const data_type& pivot = items[l];
    mid = l;
    i = l + 1;

    while (i <= r)
    {
        if (cmp(items[i], pivot))
        {
            ++mid;

            temp = items[i];
            items[i] = items[mid];
            items[mid] = temp;
        }

        ++i;
    }

    temp = items[l];
    items[l] = items[mid];
    items[mid] = temp;

    return mid;
}



/*-----------------------------------------------------------------------------
 * Threaded Shear Sort Implementation
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Integral nearest-square-root
-------------------------------------*/
inline long long int_sqrt(long long x) noexcept
{
    long long i, j;

    if (x < 2)
    {
        x = -(x > 0) & x;
    }
    else
    {
        i = int_sqrt(x >> 2) << 1;
        j = i + 1;
        x = ((j * j) > x) ? i : j;
    }

    return x;
}



/*-------------------------------------
 * fast, approximate log-base2 (round to the next power of 2 and count the
 * trailing zeroes.
-------------------------------------*/
inline long long fast_log2(long long n) noexcept
{
    if (n == 0)
    {
        return 0;
    }

    // next power of 2
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    ++n;

    // count trailing zeroes
    #if defined(LS_COMPILER_MSC) && (defined(LS_ARCH_X86) || defined(LS_ARCH_ARM))
        #if (defined(LS_ARCH_X86) && LS_ARCH_X86 == 64) || defined(LS_ARCH_AARCH64)
            unsigned long ret;
            if (_BitScanForward64(&ret, (unsigned long)n))
            {
                return (long long)ret;
            }
            return 64ll;

        #else
            unsigned long ret;
            if (_BitScanForward(&ret, (unsigned long)n))
            {
                return (long long)ret;
            }
            return 32ll;

        #endif

    #elif defined(LS_ARCH_X86)
        return (long long)_tzcnt_u64((unsigned long long)n);

    #elif defined(LS_COMPILER_GNU)
        return __builtin_ctzll(n);

    #else
        long long ret = 0ll;
        while (!(n & 1ll))
        {
            n >>= 1ll;
            ++ret;
        }
        return ret;
    #endif
}



/*-------------------------------------
 * shear-sort implementation
-------------------------------------*/
template <typename data_type, class Indexer>
inline void shear_sort_internal(data_type* const items, data_type* const indices, long long count, long long stride, Indexer indexer) noexcept
{
    if (count <= 1ll)
    {
        return;
    }

    constexpr unsigned long long base = 256ull;
    constexpr unsigned long long mask = base - 1ull;
    static_assert(base && !(base & (base-1ull)), "Input template parameter 'base' must be a power of two.");

    for (unsigned long long divisor = 0ll, m = 0ull; m < 4ull; divisor += 8, ++m)
    {
        unsigned long long radices[base] = {0ull};

        // Store count of occurrences in radices[]
        for (long long i = 0, j = 0; i < count; ++i, j = i * stride)
        {
            const unsigned long long inIndex = indexer(items[j]);
            const unsigned long long radix   = (inIndex >> divisor) & mask;
            radices[radix]++;
        }

        // Change radices[i] so that radices[i] now contains actual
        //  position of this digit in output[]
        for (unsigned long long i = 1ull; i < base; i++)
        {
            radices[i] += radices[i - 1ull];
        }

        // Build the output array
        for (long long i = count - 1ll, j = i*stride; i >= 0ll; i--, j = i*stride)
        {
            const data_type&   elem     = items[j];
            unsigned long long radix    = indexer(elem);
            unsigned long long inIndex  = (radix >> divisor) & mask;
            unsigned long long outIndex = --radices[inIndex];

            indices[outIndex] = elem;
        }

        // Copy the output array to arr[], so that arr[] now
        // contains sorted numbers according to current digit
        for (long long i = 0ll, j = 0ll; i < count; i++, j = i*stride)
        {
            items[j] = indices[i];
        }
    }
}



/*-------------------------------------
 * shear-sort partitioning
-------------------------------------*/
template <typename data_type, class AscendingIndexer = ls::utils::RadixIndexerAscending<data_type>, class DescendingIndexer = ls::utils::RadixIndexerDescending<data_type>>
inline void shear_sort_dispatch(data_type* items, data_type* indices, long long count, long long offset, long long stride, bool ascending) noexcept
{
    if (ascending)
    {
        ls::utils::impl::shear_sort_internal<data_type, AscendingIndexer>(items+offset, indices, count, stride, AscendingIndexer{});
    }
    else
    {
        ls::utils::impl::shear_sort_internal<data_type, DescendingIndexer>(items+offset, indices, count, stride, DescendingIndexer{});
    }
}



} // end impl namespace
} // end utils namespace



/*-----------------------------------------------------------------------------
 * Invocations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Bubble Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_bubble(data_type* const items, long long count, Comparator cmp) noexcept
{
    for (long long i = 0; i < count; ++i)
    {
        for (long long j = count-1; j > i; --j)
        {
            if (cmp(items[j], items[i]))
            {
                const data_type temp = items[i];
                items[i] = items[j];
                items[j] = temp;
            }
        }
    }
}



/*-------------------------------------
 * Selection Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_selection(data_type* const items, long long count, Comparator cmp) noexcept
{
    for (long long i = 0; i < count; ++i)
    {
        long long minIdx = i;

        for (long long j = count-1; j > i; --j)
        {
            if (cmp(items[j], items[minIdx]))
            {
                minIdx = j;
            }
        }

        {
            const data_type temp = items[i];
            items[i] = items[minIdx];
            items[minIdx] = temp;
        }
    }
}



/*-------------------------------------
 * Insertion Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_insertion(data_type* const items, long long count, Comparator cmp) noexcept
{
    for (long long i = 1; i < count; ++i)
    {
        const data_type x = items[i];
        long long j = i - 1;

        while (j >= 0 && cmp(x, items[j]))
        {
            items[j+1] = items[j];
            --j;
        }

        items[j+1] = x;
    }
}



/*-------------------------------------
 * Shell Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_shell(data_type* const items, long long count, Comparator cmp) noexcept
{
    for (long long i = count / 4; i > 4; i /= 4)
    {
        for (long long j = 0; j < i; ++j)
        {
            impl::sort_shell_insert<data_type, Comparator>(items + j, count - j, i, cmp);
        }
    }

    impl::sort_shell_insert<data_type, Comparator>(items, count, 1, cmp);
}



/*-------------------------------------
 * Merge Sort (recursive, unbuffered)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge(data_type* const items, long long count, Comparator cmp) noexcept
{
    ls::utils::Pointer<data_type[], ls::utils::AlignedDeleter>&& temp = ls::utils::make_unique_aligned_array<data_type>(count);
    if (temp)
    {
        ls::utils::sort_merge<data_type, Comparator>(items, temp, count, cmp);
    }

}



/*-------------------------------------
 * Merge Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge(data_type* const items, data_type* const temp, long long count, Comparator cmp) noexcept
{
    impl::sort_merge_impl<data_type, Comparator>(items, temp, 0, count-1, cmp);
}



/*-------------------------------------
 * Merge Sort (iterative, unbuffered)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge_iterative(data_type* const items, long long count, Comparator cmp) noexcept
{
    ls::utils::Pointer<data_type[], ls::utils::AlignedDeleter>&& temp = ls::utils::make_unique_aligned_array<data_type>(count);
    if (temp)
    {
        ls::utils::sort_merge_iterative<data_type, Comparator>(items, temp, count, cmp);
    }
}



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator>
void utils::sort_merge_iterative(data_type* const items, data_type* const temp, long long count, Comparator cmp) noexcept
{
    long long left, rght, rend;
    long long i,j,k,m;

    for (k = 1; k < count; k *= 2)
    {
        long long k2 = k*2;
        
        for (left = 0; left+k < count; left += k2)
        {
            rght = left + k;
            rend = rght + k;

            if (rend > count)
            {
                rend = count;
            }

            m = left;
            i = left;
            j = rght;

            while (i < rght && j < rend)
            {
                if (cmp(items[j], items[i]))
                {
                    temp[m] = items[j];
                    j++;
                }
                else
                {
                    temp[m] = items[i];
                    i++;
                }

                m++;
            }

            while (i < rght)
            {
                temp[m++] = items[i++];
            }

            while (j < rend)
            {
                temp[m++] = items[j++];
            }

            for (m = left; m < rend; m++)
            {
                items[m] = temp[m];
            }
        }
    }
}



/*-------------------------------------
 * Merge Sort (parallel, iterative, buffered)
-------------------------------------*/
template <typename data_type, class Comparator>
void utils::sort_merge_iterative(
    data_type* const items,
    data_type* const temp,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp) noexcept
{
    long long left, rght, rend;
    long long i,j,k,m;
    long long phase = numThreads;
    constexpr unsigned maxIters = 8;
    unsigned currentIters;

    for (k = 1; k < count; k *= 2, phase += numThreads)
    {
        const long long k2 = k * 2;
        const long long ki = k2 * threadId;
        const long long kt = k2 * numThreads;

        LS_PREFETCH(items+ki, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
        LS_PREFETCH(temp+ki, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);

        for (left = ki; left+k < count; left += kt)
        {
            rght = left + k;
            rend = rght + k;

            if (rend > count)
            {
                rend = count;
            }

            m = left;
            i = left;
            j = rght;

            while (i < rght && j < rend)
            {
                if (cmp(items[j], items[i]))
                {
                    temp[m] = items[j++];
                }
                else
                {
                    temp[m] = items[i++];
                }

                m++;
            }

            while (i < rght)
            {
                temp[m++] = items[i++];
            }

            while (j < rend)
            {
                temp[m++] = items[j++];
            }

            for (m = left; m < rend; ++m)
            {
                items[m] = temp[m];
            }
        }

        // sync
        numSortPhases->fetch_add(1, std::memory_order_acq_rel);
        currentIters = 1;

        while (numSortPhases->load(std::memory_order_consume) < phase)
        {
            // spin
            // spin
            switch (currentIters)
            {
                case 8:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 4:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 2:
                    ls::setup::cpu_yield();
                default:
                    ls::setup::cpu_yield();
                    currentIters = currentIters < maxIters ? (currentIters+currentIters) : maxIters;
            }
        }
    }
}



/*-------------------------------------
 * Merge Sort (parallel, iterative, unbuffered)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge_iterative(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp) noexcept
{
    ls::utils::Pointer<data_type[], ls::utils::AlignedDeleter>&& temp = ls::utils::make_unique_aligned_array<data_type>(count);
    if (temp)
    {
        ls::utils::sort_merge_iterative<data_type, Comparator>(items, temp, count, numThreads, threadId, numSortPhases, cmp);
    }
}



/*-------------------------------------
 * Quick Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_quick(data_type* const items, long long count, Comparator cmp) noexcept
{
    impl::sort_quick_impl<data_type, Comparator>(items, 0, count - 1, cmp);
}



/*-------------------------------------
 * Quick Sort (iterative)
 *
 * Based on the method presented by:
 * https://kabas.online/tutor/sorting.html
-------------------------------------*/
template <typename data_type, class Comparator>
void utils::sort_quick_iterative(data_type* const items, long long count, Comparator cmp) noexcept
{
    constexpr long long stackSpace = CHAR_BIT*sizeof(int);
    long long stack[stackSpace];
    long long mid;
    long long space = 0;
    long long l = 0;
    long long r = count - 1;

    while (true)
    {
        const long long remaining = r - l;

        if (remaining < stackSpace)
        {
            ls::utils::sort_insertion<data_type, Comparator>(items + l, remaining + 1, cmp);

            if (space > 0)
            {
                space -= 2;
                l = stack[space];
                r = stack[space + 1];
            }
            else
            {
                break;
            }
        }
        else
        {
            mid = impl::sort_quick_partition<data_type, Comparator>(items, l, r, cmp);

            if (mid < ((l + r) >> 1))
            {
                stack[space] = mid + 1;
                stack[space + 1] = r;
                r = mid - 1;
            }
            else
            {
                stack[space] = l;
                stack[space + 1] = mid - 1;
                l = mid + 1;
            }

            space += 2;
        }
    }
}



/*-------------------------------------
 * Radix Sort
-------------------------------------*/
template <typename data_type, class Indexer>
inline void utils::sort_radix(data_type* const items, long long count, Indexer indexer) noexcept
{
    // temp storage array
    ls::utils::Pointer<data_type[], ls::utils::AlignedDeleter>&& indices = ls::utils::make_unique_aligned_array<data_type>(count);
    if (indices)
    {
        ls::utils::sort_radix<data_type, Indexer>(items, indices, count, indexer);
    }
}



/*-------------------------------------
 * Radix Sort (buffered)
-------------------------------------*/
template <typename data_type, class Indexer>
void utils::sort_radix(data_type* const items, data_type* const indices, long long count, Indexer indexer) noexcept
{
    if (count <= 1ll)
    {
        return;
    }

    constexpr unsigned long long base = 256ull;
    constexpr unsigned long long mask = base - 1ull;
    static_assert(base && !(base & (base-1ull)), "Input template parameter 'base' must be a power of two.");

    /*
    // Find the maximum value to know number of digits needing processing
    unsigned long long m = indexer(*items);

    for (long long i = 1ll; i < count; ++i)
    {
        const unsigned long long val = indexer(items[i]);
        m = (val > m) ? val : m;
    }

    const auto&& ctz = [](unsigned long long e)->unsigned long long
    {
        #ifdef LS_X86_BMI
            return (unsigned long long)_tzcnt_u64((unsigned long long)e);
        #elif defined(LS_COMPILER_GNU) && !defined(LS_COMPILER_MSC)
            return (unsigned long long)__builtin_ctzll(e);
        #elif defined(LS_COMPILER_MSC)
            unsigned long ret;
            #if defined(LS_ARCH_AARCH64) || (defined(LS_ARCH_X86) && LS_ARCH_X86 == 64)
                return _BitScanForward64(&ret, (unsigned long)e) ? (unsigned long long)ret : 64ull;
            #else
                return _BitScanForward(&ret, (unsigned long)e) ? (unsigned long long)ret : 64ull;
            #endif
        #else
            unsigned long long ret = 0ull;
            while (!(e & 1ull))
            {
                e >>= 1ull;
                ++ret;
            }
            return ret;
        #endif
    };

    // Do counting sort for every digit. Note that instead
    // of passing digit number, exp is passed. exp is 10^i
    // where i is current digit number
    for (unsigned long long exponent = 1ll, divisor = 0ll; -(long long)(divisor < 64ull) & (long long)(m >> divisor); exponent *= base, divisor = ctz(exponent))
    */

    for (unsigned long long divisor = 0ll, m = 0ull; m < 4ull; divisor += 8, ++m)
    {
        unsigned long long radices[base] = {0ull};

        // Store count of occurrences in radices[]
        for (long long i = 0; i < count; ++i)
        {
            const unsigned long long inIndex = indexer(items[i]);
            const unsigned long long radix   = (inIndex >> divisor) & mask;
            radices[radix]++;

            LS_PREFETCH(items+i+8, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
        }

        // Change radices[i] so that radices[i] now contains actual
        //  position of this digit in output[]
        for (unsigned long long i = 1ull; i < base; ++i)
        {
            radices[i] += radices[i - 1ull];
        }

        // Build the output array
        for (long long i = count - 1ll; i >= 0ll; --i)
        {
            const data_type&   elem     = items[i];
            unsigned long long radix    = indexer(elem);
            unsigned long long inIndex  = (radix >> divisor) & mask;
            unsigned long long outIndex = --radices[inIndex];

            indices[outIndex] = elem;

            LS_PREFETCH(items+i-8, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
        }

        // Copy the output array to arr[], so that arr[] now
        // contains sorted numbers according to current digit
        for (long long i = 0ll; i < count; ++i)
        {
            items[i] = indices[i];
        }
    }
}



/*-------------------------------------
 * Radix Sort comparative adapter
-------------------------------------*/
template <typename data_type, class Comparator, class AscendingIndexer, class DescendingIndexer>
inline void utils::sort_radix_comparative(data_type* const items, long long count, Comparator cmp) noexcept
{
    // temp storage array
    ls::utils::Pointer<data_type[], ls::utils::AlignedDeleter>&& indices = ls::utils::make_unique_aligned_array<data_type>(count);
    if (indices)
    {
        ls::utils::sort_radix_comparative<data_type, Comparator, AscendingIndexer, DescendingIndexer>(items, indices, count, cmp);
    }
}



/*-------------------------------------
 * Radix Sort comparative adapter (preallocated)
-------------------------------------*/
template <typename data_type, class Comparator, class AscendingIndexer, class DescendingIndexer>
inline void utils::sort_radix_comparative(data_type* const items, data_type* const indices, long long count, Comparator cmp) noexcept
{
    const bool cmpLt = cmp((data_type)0, (data_type)1);
    const bool cmpGt = cmp((data_type)1, (data_type)0);

    if (cmpLt)
    {
        ls::utils::sort_radix<data_type, AscendingIndexer>(items, indices, count, AscendingIndexer{});
    }
    else if (cmpGt)
    {
        ls::utils::sort_radix<data_type, DescendingIndexer>(items, indices, count, DescendingIndexer{});
    }
}



/*-------------------------------------
 * Shear Sort for parallel sorting
-------------------------------------*/
template <typename data_type, class Comparator>
void utils::sort_sheared(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator) noexcept
{
    /*
     * Attempt to sort the numbers as if they were an MxN matrix.
     */
    data_type* nums = items;
    long long i;
    long long phase;
    long long numSortable;

    /*
     * Shear Sort works on MxN matrices. Here we calculate the dimensions of a
     * NxN matrix then sort the numFinalCol values later.
     */
    long long numTotalCols = impl::int_sqrt(count);

    /*
     * How many overall rows exist in both the largest and smallest columns.
     */
    long long numTotalRows = (count/numTotalCols) + ((count % numTotalCols) != 0);

    /*
     * Retrieve the number of elements in the final row.
     */
    long long numFinalCol = count % numTotalCols;

    /*
     * How many rows exist in only the smallest columns
     */
    long long numFullRows = count / numTotalCols;

    /*
     * Count of the elements in a square matrix
     */
    long long numSquared = numFullRows*numTotalCols;

    /*
     * Increment of which rows should be modified in the last sorting phase.
     */
    long long offsetIncrement = numTotalCols * numThreads * 2ll;

    /*
     * Calculate the total number of times needed to iterate over each row &
     * column
     */
    long long totalPhases = 2ll * impl::fast_log2(numTotalCols);

    #if 0
    if (threadId == numThreads-1)
    {
        printf("\nThread Count:   %lld", numThreads);
        printf("\nSortable Nums:  %lld", count);
        printf("\nTotal Phases:   %lld", totalPhases);
        printf("\nN x N Dimens:   %lld", numTotalCols);
        printf("\nN x X Count:    %lld", numSquared);
        printf("\nTotal Rows:     %lld", numTotalRows);
        printf("\nFull Rows:      %lld", numFullRows);
        printf("\nFinal Row:      %lld\n", numFinalCol);
    }
    #endif

    ls::utils::Pointer<data_type[], AlignedDeleter>&& indices = ls::utils::make_unique_aligned_array<data_type>(numTotalRows+numFullRows+numFinalCol);

    /*
     * A phase counts as a single pass over the rows or  columns of the MxM
     * matrix. The upper-bound on the number of phases is equal to 2*ln(N)+1,
     * where M is equal to the number of total input values.
     */
    for (phase = 0; phase < totalPhases; ++phase)
    {
        if (phase & 1)
        {
            /*
             * When the phase is even, sort all columns of the matrix.
             */
            for (i = threadId; i < numTotalCols; i += numThreads)
            {
                numSortable = (i < numFinalCol) ? numTotalRows : numFullRows;
                impl::shear_sort_dispatch<data_type>(nums, indices, numSortable, i, numTotalCols, true);
            }
        }
        else
        {
            /*
             * When the phase is odd, sort all rows of the matrix.
             */
            for (i = threadId; i < numFullRows; i += numThreads)
            {
                numSortable = (i < numFullRows) ? numTotalCols : (numTotalCols+numFinalCol);

                /*
                 * The original shear sort algorithm sorts alternating rows
                 * from smallest to largest, then largest to smallest.
                 */
                impl::shear_sort_dispatch<data_type>(nums, indices, numSortable, i*numTotalCols, 1, (i & 1) != 0);
            }
        }

        /*
         * Sync all threads before moving onto a new phase (i.e., switch from
         * sorting rows to sorting columns).
         */
        numSortPhases->fetch_add(1, std::memory_order_acq_rel);
        while (numSortPhases->load(std::memory_order_consume) < (phase+1)*numThreads)
        {
            ls::setup::cpu_yield();
            ls::setup::cpu_yield();
        }
    }

    /*
     * The traditional shear-sort algorithm contains alternating rows of
     * increasing and decreasing values. Use one final sort to re-order the
     * final rows.
     */
    for (i = (threadId*2ll+1ll)*numTotalCols; i <= numSquared; i += offsetIncrement)
    {
        long long offset = i-numTotalCols;

        /* ensure the last row gets sorted with a full row */
        long long numsToSort = numTotalCols;

        if (i == numSquared)
        {
            numsToSort += numFinalCol;
        }
        else if (i+numTotalCols == numSquared)
        {
            numsToSort += numTotalCols + numFinalCol;
        }
        else
        {
            numsToSort += numTotalCols;
        }

        impl::shear_sort_dispatch<data_type>(nums, indices, numsToSort, offset, 1, true);
    }

    /*
     * Sync all threads before moving onto a new phase (i.e., switch from
     * sorting rows to sorting columns).
     */
    numSortPhases->fetch_add(1, std::memory_order_acq_rel);
    ++phase;
    while (numSortPhases->load(std::memory_order_consume) < phase*numThreads);

    #if 0
    if (threadId == numThreads-1)
    {
        for (long long k = 0, j = 0; k < count; ++k)
        {
            if (!(k % numTotalCols))
            {
                printf("\n%-4lld: ", j++);
            }

            printf("%-4d ", nums[k]);
        }

        printf("\n");
    }
    #endif
}



/*-------------------------------------
    Bitonic Sort for arrays of size 2^n
-------------------------------------*/
template <typename data_type, class Comparator>
void utils::sort_bitonic(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp) noexcept
{
    // Can only sort powers of 2
    if ((count == 0) || (count & (count - 1ll)))
    {
        return;
    }

    long long phase = numThreads;
    constexpr unsigned maxIters = 8;
    unsigned currentIters;

    // Improved algorithm to reduce false sharing
    #if 1
        const long long chunks = (count / numThreads);
        const long long start  = chunks * threadId;
        const long long c      = start + chunks;
        const long long end    = c > count ? count : c;

        for (long long k = 1; k < count; k <<= 1)
        {
            long long k2 = k << 1;

            for (long long j = k; j > 0; j >>= 1, phase += numThreads)
            {
                for (long long i = start; i < end; ++i)
                {
                    const long long ik = i & k2;
                    const long long l = i ^ j;
                    const long long a = ik ? i : l;
                    const long long b = ik ? l : i;

                    if (l > i && cmp(items[a], items[b]))
                    {
                        const data_type temp = items[a];
                        items[a] = items[b];
                        items[b] = temp;
                    }
                }

                numSortPhases->fetch_add(1, std::memory_order_acq_rel);
                LS_PREFETCH(items+start, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
                currentIters = 1;

                while (numSortPhases->load(std::memory_order_consume) < phase)
                {
                    // spin
                    switch (currentIters)
                    {
                        case 8:
                            ls::setup::cpu_yield();
                            ls::setup::cpu_yield();
                            ls::setup::cpu_yield();
                            ls::setup::cpu_yield();
                        case 4:
                            ls::setup::cpu_yield();
                            ls::setup::cpu_yield();
                        case 2:
                            ls::setup::cpu_yield();
                        default:
                            ls::setup::cpu_yield();
                            currentIters = currentIters < maxIters ? (currentIters+currentIters) : maxIters;
                    }
                }
            }
        }

    #else
        for (long long i = 1; i < count; i *= 2)
        {
            for (long long j = i; j > 0; j /= 2, phase += numThreads)
            {
                const long long jt0 = 2 * j * threadId;
                const long long jt1 = 2 * j * numThreads;

                for (long long k = jt0; k < count; k += jt1)
                {
                    const long long kj = k + j;
                    const long long cmpLess = k & (i * 2);

                    for (long long l = k; l < kj; ++l)
                    {
                        const long long lj = l + j;
                        const long long c0 = cmpLess ? l : lj;
                        const long long c1 = cmpLess ? lj : l;

                        if (cmp(items[c0], items[c1]))
                        {
                            const data_type temp = items[l];
                            items[l] = items[lj];
                            items[lj] = temp;
                        }
                    }
                }

                numSortPhases->fetch_add(1, std::memory_order_acq_rel);
                while (numSortPhases->load(std::memory_order_consume) < phase)
                {
                    // spin
                }
            }
        }

    #endif
}



/*-------------------------------------
    Odd-Even Merge Sort for arrays of size 2^n
-------------------------------------*/
template <typename data_type, class Comparator>
void utils::sort_odd_even(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numSortPhases,
    Comparator cmp) noexcept
{
    // Can only sort powers of 2
    if ((count == 0) || (count & (count - 1ll)))
    {
        return;
    }

    long long phase = numThreads;
    constexpr unsigned maxIters = 8;
    unsigned currentIters;

    for (long long p = 1, p2 = 1; p < count; p *= 2, p2 += 1)
    {
        for (long long k = p; k > 0; k /= 2, phase += numThreads)
        {
            const long long kpmod = k & (p-1); // k % p
            const long long k2 = 2 * k;
            const long long kt0 = k2 * threadId;
            const long long kt1 = k2 * numThreads;

            for (long long j = kpmod+kt0; j+k < count; j += kt1)
            {
                for (long long i = 0; i < k; ++i)
                {
                    const long long ij = j + i;
                    const long long ijk = ij + k;

                    if ((ij >> p2) == (ijk >> p2) && cmp(items[ijk], items[ij]))
                    {
                        const data_type temp = items[ij];
                        items[ij] = items[ijk];
                        items[ijk] = temp;
                    }
                }
            }

            numSortPhases->fetch_add(1, std::memory_order_acq_rel);
            currentIters = 1;

            while (numSortPhases->load(std::memory_order_consume) < phase)
            {
                // spin
                switch (currentIters)
                {
                    case 8:
                        ls::setup::cpu_yield();
                        ls::setup::cpu_yield();
                        ls::setup::cpu_yield();
                        ls::setup::cpu_yield();
                    case 4:
                        ls::setup::cpu_yield();
                        ls::setup::cpu_yield();
                    case 2:
                        ls::setup::cpu_yield();
                    default:
                        ls::setup::cpu_yield();
                        currentIters = currentIters < maxIters ? (currentIters+currentIters) : maxIters;
                }
            }
        }
    }
}


} // end ls namespace

#endif /* LS_UTILS_SORT_IMPL_HPP */
