
#include <climits> // CHAR_BIT

#include "lightsky/utils/Pointer.h" // Pointer<>
#include "lightsky/utils/Types.hpp" // std::forward


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
void sort_shell_insert(data_type* const items, const long long count, const long long increment, Comparator cmp) noexcept
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
    data_type pivot;
    long long i;
    long long mid = (l + r) >> 1;

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
 * fast, approximate log-base 2
-------------------------------------*/
inline float fast_log2(float n) noexcept
{
    static_assert(sizeof(int) == sizeof(float), "Failed test for data-type aliasing");
    int* exp;
    int x;
    int log2;
    float ret;

    exp = (int*)&n;
    x = *exp;

    log2 = ((x >> 23) & 255) - 128;

    x &= ~(255 << 23);
    x += 127 << 23;

    *exp = x;
    ret = ((-1.f / 3.f) * n + 2.f) * n - 2.f / 3.f;

    return ret + log2;
}



/*-------------------------------------
 * fast, approximate ln
-------------------------------------*/
inline float fast_log(float n) noexcept
{
    return fast_log2(n) * 0.693147181f; /* ln( 2 ) */
}



/*-------------------------------------
 * shear-sort partitioning for less-than comparison
-------------------------------------*/
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
long long shear_partition_lt(data_type* items, long long l, long long r, long long offset, long long stride, Comparator cmp) noexcept
{
    data_type temp;
    data_type pivot;
    long long i;
    long long i0;
    long long mid = (l + r) / 2;

    long long mid0 = offset+(mid*stride);
    long long l0 = offset+(l*stride);

    temp = items[mid0];
    items[mid0] = items[l0];
    items[l0] = temp;

    pivot = items[l0];
    mid = l;
    i = l + 1;

    i0 = offset+(i*stride);
    mid0 = offset+(mid*stride);

    while (i <= r)
    {
        if (cmp(items[i0], pivot))
        {
            ++mid;
            mid0 = offset+(mid*stride);

            temp = items[i0];
            items[i0] = items[mid0];
            items[mid0] = temp;
        }

        ++i;
        i0 = offset+(i*stride);
    }

    temp = items[l0];
    items[l0] = items[mid0];
    items[mid0] = temp;

    return mid;
}



/*-------------------------------------
 * shear-sort with less-than comparison
-------------------------------------*/
template <typename data_type, class Comparator>
void shear_sort_lt(data_type* const items, long long count, long long offset, long long stride, Comparator cmp) noexcept
{
    long long stack[64];
    long long mid;
    long long space = 0;
    long long l = 0;
    long long r = count - 1;

    while (1)
    {
        if (l < r)
        {
            mid = shear_partition_lt<data_type, Comparator>(items, l, r, offset, stride, cmp);

            if (mid < (l + r) / 2)
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
        else if (space > 0)
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
}



/*-------------------------------------
 * shear-sort partitioning for greater-than comparison
-------------------------------------*/
template <typename data_type, class Comparator>
long long shear_partition_gt(data_type* items, long long l, long long r, long long offset, long long stride, Comparator cmp) noexcept
{
    data_type temp;
    data_type pivot;
    long long i;
    long long i0;
    long long mid = (l + r) / 2;

    long long mid0 = offset+(mid*stride);
    long long l0 = offset+(l*stride);

    temp = items[mid0];
    items[mid0] = items[l0];
    items[l0] = temp;

    pivot = items[l0];
    mid = l;
    i = l + 1;

    i0 = offset+(i*stride);
    mid0 = offset+(mid*stride);

    while (i <= r)
    {
        if (cmp(items[i0], pivot))
        {
            ++mid;
            mid0 = offset+(mid*stride);

            temp = items[i0];
            items[i0] = items[mid0];
            items[mid0] = temp;
        }

        ++i;
        i0 = offset+(i*stride);
    }

    temp = items[l0];
    items[l0] = items[mid0];
    items[mid0] = temp;

    return mid;
}



/*-------------------------------------
 * shear-sort with greater-than comparison
-------------------------------------*/
template <typename data_type, class Comparator>
void shear_sort_gt(data_type* const items, long long count, long long offset, long long stride, Comparator cmp) noexcept
{
    long long stack[64];
    long long mid;
    long long space = 0;
    long long l = 0;
    long long r = count - 1;

    while (1)
    {
        if (l < r)
        {
            mid = shear_partition_gt<data_type, Comparator>(items, l, r, offset, stride, cmp);

            if (mid < (l + r) / 2)
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
        else if (space > 0)
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
    for (long long i = count >> 2; i > 4; i >>= 2)
    {
        for (long long j = 0; j < i; ++j)
        {
            impl::sort_shell_insert<data_type, Comparator>(items + j, count - j, i, cmp);
        }
    }

    impl::sort_shell_insert<data_type, Comparator>(items, count, 1, cmp);
}



/*-------------------------------------
 * Merge Sort (recursive)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge(data_type* const items, long long count, Comparator cmp) noexcept
{
    ls::utils::Pointer<data_type[]> temp{new data_type[count]};
    impl::sort_merge_impl<data_type, Comparator>(items, temp, 0, count-1, cmp);
}



/*-------------------------------------
 * Merge Sort (iterative)
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::sort_merge_iterative(data_type* const items, long long count, Comparator cmp) noexcept
{
    long long left, rght, rend;
    long long i,j,k,m;
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
inline void utils::sort_quick_iterative(data_type* const items, long long count, Comparator cmp) noexcept
{
    long long stack[CHAR_BIT*sizeof(long long)];
    long long mid;
    long long space = 0;
    long long l = 0;
    long long r = count - 1;

    while (true)
    {
        const long long remaining = r - l;

        if (remaining < (long long)(CHAR_BIT*sizeof(long long))-1l)
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
 * Shear Sort for parallel sorting
-------------------------------------*/
template <typename data_type, class LessComparator, class GreaterComparator>
void utils::sort_sheared(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numThreadsFinished,
    std::atomic_llong* numSortPhases,
    LessComparator cmpL,
    GreaterComparator cmpG) noexcept
{
    /*
     * Attempt to sort the numbers as if they were an MxN matrix.
     */
    data_type* nums = items;
    long long i;
    long long phase;
    long long numSortable;

    /*
     * Calculate the total number of times needed to iterate over each row &
     * column
     */
    long long totalPhases = 2l * (long long)impl::fast_log((float)count) + 1l;

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
    long long offsetIncrement = numTotalCols * numThreads * 2l;

    #if 0
    if (!threadId)
    {
        printf("\nThread Count:   %zd", numThreads);
        printf("\nSortable Nums:  %ld", count);
        printf("\nTotal Phases:   %ld", totalPhases);
        printf("\nN x N Dimens:   %ld", numTotalCols);
        printf("\nN x X Count:    %ld", numSquared);
        printf("\nTotal Rows:     %ld", numTotalRows);
        printf("\nFull Rows:      %ld", numFullRows);
        printf("\nFinal Row:      %ld\n", numFinalCol);
    }
    #endif

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
                impl::shear_sort_lt<data_type, LessComparator>(nums, numSortable, i, numTotalCols, cmpL);
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
                if (i & 1)
                {
                    impl::shear_sort_gt<data_type, GreaterComparator>(nums, numSortable, i*numTotalCols, 1, cmpG);
                }
                else
                {
                    impl::shear_sort_lt<data_type, LessComparator>(nums, numSortable, i*numTotalCols, 1, cmpL);
                }
            }
        }

        /*
         * Sync all threads before moving onto a new phase (i.e., switch from
         * sorting rows to sorting columns).
         */
        numSortPhases->fetch_add(1, std::memory_order_relaxed);
        if ((phase+1) == totalPhases)
        {
            break;
        }

        while (numSortPhases->load(std::memory_order_relaxed) < (unsigned)(phase+1)*numThreads);
    }

    /*
     * Ensure all threads have finished their initial sorting.
     */
    numThreadsFinished->fetch_add(1, std::memory_order_relaxed);
    while (numThreadsFinished->load(std::memory_order_relaxed) < numThreads)
    {
    }

    /*
     * The traditional shear-sort algorithm contains alternating rows of
     * increasing and decreasing values. Use one final sort to re-order the
     * final rows.
     */
    for (i = (threadId*2l+1l)*numTotalCols; i <= numSquared; i += offsetIncrement)
    {
        long long offset = i;

        /* ensure the last row gets sorted with a full row */
        long long numsToSort = numTotalCols;

        if (i+numTotalCols >= numSquared)
        {
            if (count-i < numTotalCols)
            {
                offset -= numTotalCols;
            }

            numsToSort = count-offset;
        }

        impl::shear_sort_lt<data_type, LessComparator>(nums, numsToSort, offset, 1, cmpL);
    }

    /*
     * Indicate that the sort has completed.
     */
    numThreadsFinished->fetch_add(1, std::memory_order_relaxed);

    /*
     * Sync
     */
    while (numThreadsFinished->load(std::memory_order_acquire) < numThreads*2);

    #if 0
    for (long long k = 0, j = 0; k < count; ++k)
    {
        if (!(k % impl::int_sqrt(count)))
        {
            printf("\n%ld: ", j++);
        }

        printf("%d ", nums[k]);
    }
    #endif
}




/*-------------------------------------
    Bitonic Sort for arrays of size 2^n
-------------------------------------*/
template <typename data_type, class LessComparator, class GreaterComparator>
void utils::sort_bitonic(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numThreadsFinished,
    std::atomic_llong* numSortPhases,
    LessComparator cmpL,
    GreaterComparator cmpG) noexcept
{
    long long phase = numThreads;

    for (long long i = 1; i < count; i <<= 1)
    {
        for (long long j = i; j > 0; j >>= 1, phase += numThreads)
        {
            for (long long k = 0; k < count; k += (j << 1))
            {
                const long long kj = k+j;
                const long long cmpLess = k & (i << 1);

                if (cmpLess)
                {
                    for (long long l = k + threadId; l < kj; l += numThreads)
                    {
                        const long long lj = l + j;

                        if (cmpL(items[l], items[lj]))
                        {
                            const data_type temp = items[l];
                            items[l] = items[lj];
                            items[lj] = temp;
                        }
                    }
                }
                else
                {
                    for (long long l = k + threadId; l < kj; l += numThreads)
                    {
                        const long long lj = l + j;

                        if (cmpG(items[l], items[lj]))
                        {
                            const data_type temp = items[l];
                            items[l] = items[lj];
                            items[lj] = temp;
                        }
                    }
                }
            }

            numSortPhases->fetch_add(1, std::memory_order_acq_rel);
            while (numSortPhases->load(std::memory_order_release) < phase)
            {
                // spin
            }
        }
    }

    // sync
    numThreadsFinished->fetch_add(1, std::memory_order_acq_rel);
    while (numThreadsFinished->load(std::memory_order_release) != numThreads)
    {
        // spin
    }
}



} // end ls namespace
