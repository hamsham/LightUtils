
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



#if 1
enum {
    MAX_RAND_NUMS = 1234567
};
#else
enum
{
    MAX_RAND_NUMS = 50000001
};
#endif /* DEBUG */

static const unsigned int MAX_THREADS = 4;



/*-----------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
-----------------------------------------------------------------------------*/
// Verify that a list has been fully sorted
long is_sorted(int* const nums, long count);

// Function to run a set of tests
int bench_sorts(
    int numTests,
    void (* pTests[])(int* const nums, long count),
    void (* pThreadedTest)(int* const items, long count, std::size_t numThreads, std::size_t threadId, std::atomic_size_t* numThreadsFinished, std::atomic_size_t* numSortPhases),
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
        &quick_sort_ref,
        nullptr
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
        "Quick Sort-Reference",
        "Shear Sort (Parallel)"
    };

    int numTests = sizeof(testNames) / sizeof(char*);

    srand(time(nullptr));

    if (!bench_sorts(numTests, pTests, &ls::utils::sort_sheared<int>, testNames))
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
    void (*pTests[])(int* const, long),
    void (* pThreadedTest)(int* const, long, std::size_t, std::size_t, std::atomic_size_t*, std::atomic_size_t*),
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

        if (pTests[i])
        {
            (*pTests[i])(nums, MAX_RAND_NUMS); // pointer to the sort function
        }
        else
        {
            std::atomic_size_t numThreadsFinished{0};
            std::atomic_size_t numSortPhases{0};
            for (unsigned t = 1; t < MAX_THREADS; ++t)
            {
                std::thread{pThreadedTest, nums, MAX_RAND_NUMS, MAX_THREADS, t, &numThreadsFinished, &numSortPhases}.detach();
            }
            pThreadedTest(nums, MAX_RAND_NUMS, MAX_THREADS, 0, &numThreadsFinished, &numSortPhases);
        }

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
