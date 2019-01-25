
/**
 * @file Testing implementations of different sorting methods.
 */

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread>

#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Sort.hpp"



#if 0
enum {
    MAX_RAND_NUMS = 65535
};
#else
enum
{
    MAX_RAND_NUMS = 50000001
};
#endif /* DEBUG */

static const unsigned int MAX_THREADS = 8;



/*-----------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
-----------------------------------------------------------------------------*/
// Verify that a list has been fully sorted
long is_sorted(int* const nums, long count);

// Function to run a set of tests
int bench_sorts(
    int numTests,
    void (* pTests[])(int* const nums, long count),
    const char* const* testNames
);

// Print a list of numbers to a file
void print_nums(
    int* nums,
    long count,
    const char* const testName,
    FILE* pFile
);

// Quick Sort (iterative)
void quick_sort_2(int* const nums, long count);

// Quick Sort - Reference Implementation
void quick_sort_ref(int* const nums, long count);

// Quick Sort - Reference Implementation
template <typename data_type, class LessComparator = ls::utils::IsLess<data_type>, class GreaterComparator = ls::utils::IsGreater<data_type>>
void shear_sort_parallel(data_type* const nums, long count);

// Function to populate a list of random numbers
void gen_rand_nums(int* const nums, long count);



/*-----------------------------------------------------------------------------
 * MAIN()
-----------------------------------------------------------------------------*/
int main(void)
{
    void (* pTests[])(int* const, long) = {
        //&ls::utils::sort_bubble<int>,
        //&ls::utils::sort_selection<int>,
        //&ls::utils::sort_insertion<int>,
        //&ls::utils::sort_shell<int>,
        &ls::utils::sort_merge<int>,
        &ls::utils::sort_merge_iterative<int>,
        &ls::utils::sort_quick<int>,
        &quick_sort_2,
        &ls::utils::sort_quick_iterative<int>,
        &quick_sort_ref
        //&shear_sort_parallel<int>
    };

    const char* testNames[] = {
        //"Bubble Sort",
        //"Selection Sort",
        //"Insertion Sort",
        //"Shell Sort",
        "Merge Sort (recursive)",
        "Merge Sort (iterative)",
        "Quick Sort (recursive)",
        "Quick Sort (iterative)",
        "Quick Sort (with insertion sort)",
        "Quick Sort-Reference"
        //"Shear Sort (Parallel)"
    };

    int numTests = sizeof(testNames) / sizeof(char*);

    srand(time(nullptr));

    if (!bench_sorts(numTests, pTests, testNames))
    {
        fprintf(stderr, "An error occurred while running sorting tests.\n");
        return -1;
    }

    return 0;
}



/*-----------------------------------------------------------------------------
 * FUNCTION IMPLEMENTATIONS
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Function to populate a list of random numbers.
-----------------------------------------------------------------------------*/
void gen_rand_nums(int* const nums, long count)
{
    long i = count;
    while (i--)
    {
        nums[i] = rand();
    }
}



/*-----------------------------------------------------------------------------
 * Function to run a set of tests.
-----------------------------------------------------------------------------*/
int bench_sorts(
    int numTests,
    void (* pTests[])(int* const, long),
    const char* const* testNames
)
{
    int* const nums = (int*)malloc(sizeof(int) * MAX_RAND_NUMS);

    if (!nums)
    {
        fprintf(
            stderr,
            "ERROR: Couldn't make an array of %d integers.\n",
            MAX_RAND_NUMS
        );
        return 0;
    }

    ls::utils::Clock<float> ticks;

    for (int i = 0; i < numTests; ++i)
    {

        printf("Initializing a %s test...", testNames[i]);
        gen_rand_nums(nums, MAX_RAND_NUMS);
        printf("Done!\n");

        printf("Testing a %s...", testNames[i]);

        // Time the current sort method
        ticks.start(); // start time

        (*pTests[i])(nums, MAX_RAND_NUMS); // pointer to the sort function

        ticks.tick(); // stop time

        const float timeToSort = ticks.tick_time().count();

        printf("Done!\n");
        printf("The %s test took %f seconds.\n", testNames[i], timeToSort);

        printf("Verifying the %s...", testNames[i]);
        if (is_sorted(nums, MAX_RAND_NUMS))
        {
            printf("Passed!\n");
        }
        else
        {
            printf("Failed!\n");
        }

#ifdef DEBUG
        print_nums(MAX_RAND_NUMS, nums, testNames[i], stdout);
#endif

        printf("\n\n");
    }

    free(nums);

    return 1;
}



/*-----------------------------------------------------------------------------
 * Print a list of numbers to a file
-----------------------------------------------------------------------------*/
void print_nums(
    int* nums,
    const long count,
    const char* const testName,
    FILE* pFile
)
{
    fprintf(pFile, "%s:\n", testName);

    for (long i = 0; i < count; ++i)
    {
        fprintf(pFile, "%d\n", nums[i]);
    }

    fprintf(pFile, "\n\n");
}



/*-----------------------------------------------------------------------------
 * Verify that a list has been sorted
-----------------------------------------------------------------------------*/
long is_sorted(int* const nums, long count)
{
    long i;

    for (i = 0; (i < count - 1) && (nums[i] <= nums[i + 1]); ++i)
    {
    }

    return i + 1 == count;
}



/*-----------------------------------------------------------------------------
 * Quick Sort (iterative)
-----------------------------------------------------------------------------*/
// Function to partition an array during a quicksort
long partitionqsort2(int* nums, long l, long r)
{
    int temp;
    int pivot;
    long i;
    long mid = (l + r) >> 1;

    temp = nums[mid];
    nums[mid] = nums[l];
    nums[l] = temp;

    pivot = nums[l];
    mid = l;
    i = l + 1;

    while (i <= r)
    {
        if (nums[i] < pivot)
        {
            ++mid;

            temp = nums[i];
            nums[i] = nums[mid];
            nums[mid] = temp;
        }

        ++i;
    }

    temp = nums[l];
    nums[l] = nums[mid];
    nums[mid] = temp;

    return mid;
}

// Iterative Quicksort
void quick_sort_2(int* const nums, long count)
{
    long stack[64];
    long mid;
    long space = 0;
    long l = 0;
    long r = count - 1;

    while (true)
    {
        if (l < r)
        {
            mid = partitionqsort2(nums, l, r);

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

/*-----------------------------------------------------------------------------
 * Quick Sort Reference Implementation
-----------------------------------------------------------------------------*/
// Quick Sort Comparison function
inline int qsortcompare(const void* x, const void* y)
{
    const int a = *(const int*)x;
    const int b = *(const int*)y;
    return a - b;
}

// Quick Sort Implementation
void quick_sort_ref(int* const nums, long count)
{
    qsort(nums, (size_t)count, sizeof(int), &qsortcompare);
}



/*-----------------------------------------------------------------------------
 * Threaded Shear Sort Implementation
-----------------------------------------------------------------------------*/
long int_sqrt(long x)
{
    long i, j;

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

float fast_log2(float n)
{
    long* exp;
    long x;
    long log2;
    float ret;

    exp = (long*)&n;
    x = *exp;

    log2 = ((x >> 23) & 255) - 128;

    x &= ~(255 << 23);
    x += 127 << 23;

    *exp = x;
    ret = ((-1.f / 3.f) * n + 2.f) * n - 2.f / 3.f;

    return ret + log2;
}

float fast_log(float n)
{
    return fast_log2(n) * 0.693147181f; /* ln( 2 ) */
}

template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
long shear_partition_lt(data_type* nums, long l, long r, long offset, long stride)
{
    constexpr Comparator cmp;
    data_type temp;
    long pivot;
    long i;
    long i0;
    long mid = (l + r) / 2;

    long mid0 = offset+(mid*stride);
    long l0 = offset+(l*stride);

    temp = nums[mid0];
    nums[mid0] = nums[l0];
    nums[l0] = temp;

    pivot = nums[l0];
    mid = l;
    i = l + 1;

    i0 = offset+(i*stride);
    mid0 = offset+(mid*stride);

    while (i <= r)
    {
        if (cmp(nums[i0], pivot))
        {
            ++mid;
            mid0 = offset+(mid*stride);

            temp = nums[i0];
            nums[i0] = nums[mid0];
            nums[mid0] = temp;
        }

        ++i;
        i0 = offset+(i*stride);
    }

    temp = nums[l0];
    nums[l0] = nums[mid0];
    nums[mid0] = temp;

    return mid;
}

/* Insertion sort that's meant to be used specifically with a shear sort */
template <typename data_type, class Comparator = ls::utils::IsLess<data_type>>
void shear_sort_lt(data_type* const nums, long count, long offset, long stride)
{
    long stack[64];
    long mid;
    long space = 0;
    long l = 0;
    long r = count - 1;

    while (1)
    {
        if (l < r)
        {
            mid = shear_partition_lt<data_type, Comparator>(nums, l, r, offset, stride);

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

template <typename data_type, class Comparator = ls::utils::IsGreater<data_type>>
long shear_partition_gt(data_type* nums, long l, long r, long offset, long stride)
{
    constexpr Comparator cmp;
    data_type temp;
    long pivot;
    long i;
    long i0;
    long mid = (l + r) / 2;

    long mid0 = offset+(mid*stride);
    long l0 = offset+(l*stride);

    temp = nums[mid0];
    nums[mid0] = nums[l0];
    nums[l0] = temp;

    pivot = nums[l0];
    mid = l;
    i = l + 1;

    i0 = offset+(i*stride);
    mid0 = offset+(mid*stride);

    while (i <= r)
    {
        if (cmp(nums[i0], pivot))
        {
            ++mid;
            mid0 = offset+(mid*stride);

            temp = nums[i0];
            nums[i0] = nums[mid0];
            nums[mid0] = temp;
        }

        ++i;
        i0 = offset+(i*stride);
    }

    temp = nums[l0];
    nums[l0] = nums[mid0];
    nums[mid0] = temp;

    return mid;
}

/* Insertion sort that's meant to be used specifically with a shear sort */
template <typename data_type, class Comparator = ls::utils::IsGreater<data_type>>
void shear_sort_gt(data_type* const nums, long count, long offset, long stride)
{
    long stack[64];
    long mid;
    long space = 0;
    long l = 0;
    long r = count - 1;

    while (1)
    {
        if (l < r)
        {
            mid = shear_partition_gt<data_type, Comparator>(nums, l, r, offset, stride);

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



template <typename data_type>
struct ShearArgs
{
    long threadId;
    long count;
    data_type* nums;
    std::atomic_uint* numPhases;
    std::atomic_uint* numFinished;
    long numThreads;
};



template <typename data_type, class LessComparator = ls::utils::IsLess<data_type>, class GreaterComparator = ls::utils::IsGreater<data_type>>
void _shear_sort_impl(const ShearArgs<data_type> args)
{
    /*
     * Attempt to sort the numbers as if they were an MxN matrix.
     */
    long count = args.count;
    data_type* nums = args.nums;
    long i;
    long phase;
    long numSortable;

    /*
     * Calculate the total number of times needed to iterate over each row &
     * column
     */
    long totalPhases = 2 * (long)fast_log(count) + 1;

    /*
     * Shear Sort works on MxN matrices. Here we calculate the dimensions of a
     * NxN matrix then sort the numFullCols values later.
     */
    long numTotalCols = int_sqrt(count);

    /*
     * How many overall rows exist in both the largest and smallest columns.
     */
    long numTotalRows = (count/numTotalCols) + ((count % numTotalCols) != 0);

    /*
     * Retrieve the number of elements in the final row.
     */
    long numFullCols = count % numTotalCols;

    /*
     * How many rows exist in only the smallest columns
     */
    long numFullRows = count / numTotalCols;

    #if 0
    if (!args.threadId)
    {
        printf("\nThread Count:   %d", MAX_THREADS);
        printf("\nSortable Nums:  %ld", count);
        printf("\nTotal Phases:   %ld", totalPhases);
        printf("\nN X N Dimens:   %ld", numTotalCols);
        printf("\nRemaining Nums: %ld", numFullCols);
        printf("\nTotal Rows:     %ld", numTotalRows);
        printf("\nFull Rows:      %ld", numFullRows);
        printf("\nFinal Row:      %ld\n", numFullCols);
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
            for (i = args.threadId; i < numTotalCols; i += MAX_THREADS)
            {
                numSortable = (i < numFullCols) ? numTotalRows : numFullRows;
                shear_sort_lt<data_type, LessComparator>(nums, numSortable, i, numTotalCols);
            }
        }
        else
        {
            /*
             * When the phase is odd, sort all rows of the matrix.
             */
            for (i = args.threadId; i < numFullRows; i += MAX_THREADS)
            {
                numSortable = (i < numFullRows) ? numTotalCols : (numTotalCols+numFullCols);

                /*
                 * The original shear sort algorithm sorts alternating rows
                 * from smallest to largest, then largest to smallest.
                 */
                if (i & 1)
                {
                    shear_sort_gt<data_type, GreaterComparator>(nums, numSortable, i*numTotalCols, 1);
                }
                else
                {
                    shear_sort_lt<data_type, LessComparator>(nums, numSortable, i*numTotalCols, 1);
                }
            }
        }

        /*
         * Sync all threads before moving onto a new phase (i.e., switch from
         * sorting rows to sorting columns).
         */
        args.numPhases->fetch_add(1, std::memory_order_relaxed);
        while (args.numPhases->load(std::memory_order_relaxed) < (unsigned)(phase+1)*MAX_THREADS);

        if ((phase+1) == totalPhases)
        {
            break;
        }
    }

    /*
     * The traditional shear-sort algorithm contains alternating rows of
     * increasing and decreasing values. Use one final check to see if we need
     * to re-sort any rows.
     * Don't sort the last row just yet (see comment below).
     */
    for (i = (args.threadId*2)+1; i < numFullRows; i += 2*MAX_THREADS)
    {
        const long offset = i * numTotalCols;

        //if (nums[offset] > nums[offset+(numTotalCols-1)])
        {
            //printf("%ld\n", i);
            shear_sort_lt<data_type, LessComparator>(nums, numTotalCols, offset, 1);
        }
    }

    /*
     * The last thread that finishes needs to sort the last two rows due to
     * the fact we're sorting an MxN+R matrix, where R represents the extra
     * values. The first thread to reach this point can do the sorting.
     */
    if (args.numFinished->fetch_add(1, std::memory_order_relaxed) == args.numThreads-1)
    {
        shear_sort_lt<data_type, LessComparator>(nums, numTotalCols+numFullCols, numTotalCols*(numFullRows-1), 1);

        /*
         * Let the other threads know they can exit.
         */
        args.numFinished->fetch_add(1, std::memory_order_relaxed);
    }
}



template <typename data_type, class LessComparator, class GreaterComparator>
void shear_sort_parallel(data_type* const nums, long count)
{
    struct ShearArgs<data_type> args[MAX_THREADS];
    std::thread threads[MAX_THREADS];
    std::atomic_uint numPhases{0};
    std::atomic_uint numFinished{0};

    for (long i = 0; i < (long)MAX_THREADS; ++i)
    {
        args[i].threadId = i;
        args[i].count = count;
        args[i].nums = nums;
        args[i].numPhases = &numPhases;
        args[i].numFinished = &numFinished;
        args[i].numThreads = MAX_THREADS;

        if (i > 0)
        {
            threads[i] = std::thread{&_shear_sort_impl<data_type, LessComparator, GreaterComparator>, args[i]};
            threads[i].detach();
        }
    }

    _shear_sort_impl<data_type, LessComparator, GreaterComparator>(args[0]);

    /*
     * Sync
     */
    while (numFinished.load(std::memory_order_relaxed) <= MAX_THREADS);

    #if 0
    for (long i = 0, j = 0; i < count; ++i)
    {
        if (!(i % int_sqrt(count)))
        {
            printf("\n%ld: ", j++);
        }

        printf("%d ", nums[i]);
    }
    #endif
}

