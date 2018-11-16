
#include "lightsky/utils/Pointer.h"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Implementations
-----------------------------------------------------------------------------*/
namespace impl
{

/*-------------------------------------
 * Insertion sort that's meant to be used specifically with a shell sort
-------------------------------------*/
template <typename data_type, class Comparator>
void shell_inssort(data_type* const items, const long count, const long increment)
{
    constexpr Comparator cmp;

    for (long i = increment; i < count; i += increment)
    {
        long j = i;
        long k = j-increment;

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
 * Merge Sort Implementation
-------------------------------------*/
template <typename data_type, class Comparator>
inline void merge_sort_impl(data_type* const nums, data_type* const temp, long left, long right)
{
    constexpr Comparator cmp;

    long i, j, k;
    const long mid = (left + right) >> 1;
    const long rightMid = right - mid;

    if (right - left < 1)
    {
        return;
    }

    merge_sort_impl<data_type, Comparator>(nums, temp, left, mid);
    merge_sort_impl<data_type, Comparator>(nums, temp, mid + 1, right);

    for (i = mid; i >= left; --i)
    {
        temp[i] = nums[i];
    }

    for (j = 1; j <= rightMid; ++j)
    {
        temp[right - j + 1] = nums[j + mid];
    }

    for (i = left, j = right, k = left; k <= right; ++k)
    {
        if (cmp(temp[i], temp[j]))
        {
            nums[k] = temp[i++];
        }
        else
        {
            nums[k] = temp[j--];
        }
    }
}



/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void quick_sort_impl(data_type* const items, const long l, const long r)
{
    constexpr Comparator cmp;
    data_type temp;

    if (r <= l)
    {
        return;
    }

    //const long pivotIndex = (l+r)/2l;
    const long pivotIndex = (l+r) >> 1l;

    temp = items[pivotIndex];
    items[pivotIndex] = items[r];
    items[r] = temp;

    long m = l - 1;
    long n = r;
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

    quick_sort_impl<data_type, Comparator>(items, l, m - 1);
    quick_sort_impl<data_type, Comparator>(items, m + 1, r);
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
inline void utils::bubble_sort(data_type* const items, long count)
{
    constexpr Comparator cmp;

    for (long i = 0; i < count; ++i)
    {
        for (long j = count-1; j > i; --j)
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
inline void utils::selection_sort(data_type* const items, long count)
{
    constexpr Comparator cmp;

    for (long i = 0; i < count; ++i)
    {
        long minIdx = i;

        for (long j = count-1; j > i; --j)
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
inline void utils::insertion_sort(data_type* const items, long count)
{
    constexpr Comparator cmp;

    for (long i = 1; i < count; ++i)
    {
        const data_type x = items[i];
        long j = i - 1;

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
inline void utils::shell_sort(data_type* const items, long count)
{
    for (long i = count >> 2; i > 4; i >>= 2)
    {
        for (long j = 0; j < i; ++j)
        {
            impl::shell_inssort<data_type, Comparator>(items+j, count-j, i);
        }
    }

    impl::shell_inssort<data_type, Comparator>(items, count, 1);
}



/*-------------------------------------
 * Shell Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::merge_sort(data_type* const items, long count)
{
    ls::utils::Pointer<data_type[]> temp{new data_type[count]};

    impl::merge_sort_impl<data_type, Comparator>(items, temp, 0, count);
}



/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::quick_sort(data_type* const items, long count)
{
    impl::quick_sort_impl<data_type, Comparator>(items, 0, count-1);
}




} // end ls namespace
