
/**
 * @file Testing implementations of different sorting methods.
 */

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "lightsky/utils/Sort.hpp"



#if 1
enum {
    MAX_RAND_NUMS = 123456
};
#else
enum
{
    MAX_RAND_NUMS = 50000001
};
#endif /* DEBUG */



/*-----------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
-----------------------------------------------------------------------------*/
// Verify that a list has been fully sorted
long issorted(int* const nums, long count);

// Function to run a set of tests
int benchmarksorts(
    int num_tests,
    void (* pTests[])(int* const nums, long count),
    const char* const* test_names
);

// Print a list of numbers to a file
void printnums(
    int* nums,
    long count,
    const char* const test_name,
    FILE* pFile
);

// Quick Sort (iterative)
void quicksort2(int* const nums, long count);

// Quick Sort - Reference Implementation
void qsortreference(int* const nums, long count);

// Function to populate a list of random numbers
int* genRandomNumList(int* const nums, long count);



/*-----------------------------------------------------------------------------
 * MAIN()
-----------------------------------------------------------------------------*/
int main(void)
{
    void (* pTests[])(int* const, long) = {
        &ls::utils::sort_bubble<int>,
        &ls::utils::sort_selection<int>,
        &ls::utils::sort_insertion<int>,
        &ls::utils::sort_shell<int>,
        &ls::utils::sort_merge<int>,
        &ls::utils::sort_merge_iterative<int>,
        &ls::utils::sort_quick<int>,
        &quicksort2,
        &ls::utils::sort_quick_iterative<int>,
        &qsortreference
    };

    const char* test_names[] = {
        "Bubble Sort",
        "Selection Sort",
        "Insertion Sort",
        "Shell Sort",
        "Merge Sort (recursive)",
        "Merge Sort (iterative)",
        "Quick Sort (recursive)",
        "Quick Sort (iterative)",
        "Quick Sort (with insertion sort)",
        "Quick Sort-Reference"
    };

    int num_tests = sizeof(test_names) / sizeof(char*);

    srand(time(nullptr));

    if (!benchmarksorts(num_tests, pTests, test_names))
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
void populaterandnums(int* const nums, long count)
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
int benchmarksorts(
    int num_tests,
    void (* pTests[])(int* const, long),
    const char* const* test_names
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

    for (int i = 0; i < num_tests; ++i)
    {
        clock_t curr_ticks, prev_ticks, total_ticks;
        double ticks_per_sec;

        printf("Initializing a %s test...", test_names[i]);
        populaterandnums(nums, MAX_RAND_NUMS);
        printf("Done!\n");

        printf("Testing a %s...", test_names[i]);

        // Time the current sort method
        prev_ticks = clock(); // start time
        (*pTests[i])(nums, MAX_RAND_NUMS); // pointer to the sort function
        curr_ticks = clock(); // stop time
        total_ticks = curr_ticks - prev_ticks;
        ticks_per_sec = (double)total_ticks / (double)CLOCKS_PER_SEC;
        printf("Done!\n");
        printf("The %s test took %f seconds.\n", test_names[i], ticks_per_sec);

        printf("Verifying the %s...", test_names[i]);
        if (issorted(nums, MAX_RAND_NUMS))
        {
            printf("Passed!\n");
        }
        else
        {
            printf("Failed!\n");
        }

#ifdef DEBUG
        printnums(MAX_RAND_NUMS, nums, test_names[i], stdout);
#endif

        printf("\n\n");
    }

    free(nums);

    return 1;
}



/*-----------------------------------------------------------------------------
 * Print a list of numbers to a file
-----------------------------------------------------------------------------*/
void printnums(
    int* nums,
    const long count,
    const char* const test_name,
    FILE* pFile
)
{
    fprintf(pFile, "%s:\n", test_name);

    for (long i = 0; i < count; ++i)
    {
        fprintf(pFile, "%d\n", nums[i]);
    }

    fprintf(pFile, "\n\n");
}



/*-----------------------------------------------------------------------------
 * Verify that a list has been sorted
-----------------------------------------------------------------------------*/
long issorted(int* const nums, long count)
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
void quicksort2(int* const nums, long count)
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
int qsortcompare(const void* x, const void* y)
{
    const int a = *(const int*)x;
    const int b = *(const int*)y;
    return a - b;
}

// Quick Sort Implementation
void qsortreference(int* const nums, long count)
{
    qsort(nums, (size_t)count, sizeof(int), &qsortcompare);
}
