
#include <climits> // CHAR_BIT

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
void sort_shell_insert(data_type* const items, const long count, const long increment)
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
 * Recursive Merge Sort
-------------------------------------*/
template <typename data_type, class Comparator>
void sort_merge_impl(data_type* const items, data_type* const temp, long left, long right)
{
    if (right-left < 1)
    {
        return;
    }

    constexpr Comparator cmp;
    long i, j, k;
    const long mid = (left+right) >> 1;
    const long rightMid = right-mid;

    sort_merge_impl<data_type, Comparator>(items, temp, left, mid);
    sort_merge_impl<data_type, Comparator>(items, temp, mid+1, right);

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
inline void sort_quick_impl(data_type* const items, const long l, const long r)
{
    constexpr Comparator cmp;
    data_type temp;

    if (r <= l)
    {
        return;
    }

    if (r-l < (long)(CHAR_BIT*sizeof(long)))
    {
        ls::utils::sort_insertion<data_type, Comparator>(items + l, (r-l) + 1);
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

    sort_quick_impl<data_type, Comparator>(items, l, m - 1);
    sort_quick_impl<data_type, Comparator>(items, m + 1, r);
}



/*-------------------------------------
 * Quick Sort Partitioning
-------------------------------------*/
template <typename data_type, class Comparator>
inline long sort_quick_partition(data_type* const items, const long l, const long r)
{
    constexpr Comparator cmp;
    data_type temp;
    data_type pivot;
    long i;
    long mid = (l + r) >> 1;

    temp = items[mid];
    items[mid] = items[l];
    items[l] = temp;

    pivot = items[l];
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



} // end impl namespace
} // end utils namespace



/*-----------------------------------------------------------------------------
 * Invocations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Bubble Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_bubble(data_type* const items, long count)
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
inline void utils::sort_selection(data_type* const items, long count)
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
inline void utils::sort_insertion(data_type* const items, long count)
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
inline void utils::sort_shell(data_type* const items, long count)
{
    for (long i = count >> 2; i > 4; i >>= 2)
    {
        for (long j = 0; j < i; ++j)
        {
            impl::sort_shell_insert<data_type, Comparator>(items + j, count - j, i);
        }
    }

    impl::sort_shell_insert<data_type, Comparator>(items, count, 1);
}



/*-------------------------------------
 * Merge Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge(data_type* const items, long count)
{
    ls::utils::Pointer<data_type[]> temp{new data_type[count]};
    impl::sort_merge_impl<data_type, Comparator>(items, temp, 0, count-1);
}



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge_iterative(data_type* const items, long count)
{
    constexpr Comparator cmp;
    long left, rght, rend;
    long i,j,k,m;
    ls::utils::Pointer<data_type[]> temp{new data_type[count]};

    if (!temp)
    {
        return;
    }

    for (k=1; k < count; k <<= 1)
    {
        for (left=0; left+k < count; left += (k << 1))
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
                temp[m] = items[i];
                i++;
                m++;
            }

            while (j < rend)
            {
                temp[m] = items[j];
                j++;
                m++;
            }

            for (m=left; m < rend; m++)
            {
                items[m] = temp[m];
            }
        }
    }

}



/*-------------------------------------
 * Quick Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_quick(data_type* const items, long count)
{
    impl::sort_quick_impl<data_type, Comparator>(items, 0, count - 1);
}



/*-------------------------------------
 * Quick Sort (iterative)
 *
 * Based on the method presented by:
 * https://kabas.online/tutor/sorting.html
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_quick_iterative(data_type* const items, long count)
{
    long stack[CHAR_BIT*sizeof(long)];
    long mid;
    long space = 0;
    long l = 0;
    long r = count - 1;

    while (true)
    {
        const long remaining = r - l;

        if (remaining < (long)(CHAR_BIT*sizeof(long))-1l)
        {
            ls::utils::sort_insertion<data_type, Comparator>(items + l, remaining + 1);

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
            mid = impl::sort_quick_partition<data_type, Comparator>(items, l, r);

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




} // end ls namespace
