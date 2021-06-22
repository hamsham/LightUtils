
/**
 * @file Testing implementations of different sorting methods.
 */

#include <atomic>
#include <cstdio>
#include <thread>

#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Sort.hpp"
#include "lightsky/utils/Time.hpp"



#if 1
enum {
    MAX_RAND_NUMS = 131072
};
#else
enum
{
    MAX_RAND_NUMS = 50000001
};
#endif /* DEBUG */

static const unsigned int MAX_THREADS = (unsigned int)std::thread::hardware_concurrency();



/*-----------------------------------------------------------------------------
 * Quick Sort Reference Implementation
-----------------------------------------------------------------------------*/
void quick_sort_ref(int* const nums, long long count, ls::utils::IsLess<int>)
{
    qsort(nums, (long long)count, sizeof(int), [](const void* x, const void* y)->int
    {
        const int a = *(const int*)x;
        const int b = *(const int*)y;
        return a - b;
    });
}



/*-----------------------------------------------------------------------------
 * Function to populate a list of random numbers.
-----------------------------------------------------------------------------*/
void gen_rand_nums(int* const nums, long long count)
{
    long long i = count;
    while (i--)
    {
        nums[i] = rand();
    }
}



/*-----------------------------------------------------------------------------
 * Print a list of numbers to a file
-----------------------------------------------------------------------------*/
void print_nums(
    int* nums,
    int mismatchPos,
    const long long count,
    const char* const testName,
    FILE* pFile
)
{
    mismatchPos = std::abs(mismatchPos);
    fprintf(pFile, "%s:\n", testName);

    for (long long i = 0; i < count; ++i)
    {
        if (mismatchPos && i == mismatchPos)
        {
            fprintf(pFile, "%10lld: %d    <--- mismatch\n", i, nums[i]);
        }
        else
        {
            fprintf(pFile, "%10lld: %d\n", i, nums[i]);
        }
    }

    fprintf(pFile, "\n\n");
}



/*-----------------------------------------------------------------------------
 * MAIN()
-----------------------------------------------------------------------------*/
ls::utils::UniqueAlignedArray<int>&& MERGE_SORT_TEMP_BUFFER = ls::utils::make_unique_aligned_array<int>(MAX_RAND_NUMS);

int main(void)
{
    srand(time(nullptr));

    ls::utils::UniqueAlignedArray<int>&& nums = ls::utils::make_unique_aligned_array<int>(MAX_RAND_NUMS);
    ls::utils::UniqueAlignedArray<int>&& temp = ls::utils::make_unique_aligned_array<int>(MAX_RAND_NUMS);
    ls::utils::UniqueAlignedArray<int>&& validation = ls::utils::make_unique_aligned_array<int>(MAX_RAND_NUMS);

    constexpr void (*pSorts[])(int* const, long long, ls::utils::IsLess<int>) = {
        &ls::utils::sort_bubble<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_selection<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_insertion<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_shell<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_merge<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_merge_iterative<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_quick<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_quick_iterative<int, ls::utils::IsLess<int>>,
        &quick_sort_ref,
        &ls::utils::sort_radix_comparative<int, ls::utils::IsLess<int>>
    };

    constexpr void (*pBufferedSorts[])(int* const, int* const, long long, ls::utils::IsLess<int>) = {
        &ls::utils::sort_merge<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_merge_iterative<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_radix_comparative<int, ls::utils::IsLess<int>>,
    };

    void (*merge_sort_parallel)(int* const, long long, long long, long long, std::atomic_llong*, ls::utils::IsLess<int>) =
    [](int* const items, long long count, long long numThreads, long long threadId, std::atomic_llong* sortPhases, ls::utils::IsLess<int> cmp)
    {
        ls::utils::sort_merge_iterative<int, decltype(cmp)>(items, MERGE_SORT_TEMP_BUFFER.get(), count, numThreads, threadId,  sortPhases, cmp);
    };

    void (*pThreadedSorts[])(int* const, long long, long long, long long, std::atomic_llong*, ls::utils::IsLess<int>) = {
        &ls::utils::sort_sheared<int>,
        &ls::utils::sort_bitonic<int>,
        &ls::utils::sort_odd_even<int>,
        merge_sort_parallel
    };

    const char* sortNames[] = {
        "Bubble Sort",
        "Selection Sort",
        "Insertion Sort",
        "Shell Sort",
        "Merge Sort (recursive)",
        "Merge Sort (iterative)",
        "Quick Sort (recursive)",
        "Quick Sort (with insertion sort)",
        "Quick Sort-Reference",
        "Radix Sort",

        "Merge Sort (prebuffered, recursive)",
        "Merge Sort (prebuffered, iterative)",
        "Radix Sort (prebuffered)",

        "Shear Sort (Parallel)",
        "Bitonic Sort (Parallel)",
        "Odd-Even Merge Sort (Parallel)",
        "Merge Sort (Parallel, prebuffered, iterative)"
    };

    ls::utils::Clock<double> ticks;    
    constexpr unsigned numTests = LS_ARRAY_SIZE(sortNames);
    constexpr unsigned sortOffset = 0;
    constexpr unsigned bufferedSortOffset = LS_ARRAY_SIZE(pSorts);
    constexpr unsigned threadedSortOffset = bufferedSortOffset + LS_ARRAY_SIZE(pBufferedSorts);

    if (!nums || !temp || !validation)
    {
        fprintf(stderr, "ERROR: Couldn't make an array of %d integers.\n", MAX_RAND_NUMS);
        return -1;
    }

    for (unsigned i = 0, sortIndex = 0; i < numTests; ++i)
    {
        fprintf(stdout, "Initializing a %s test...", sortNames[i]);
        gen_rand_nums(nums, MAX_RAND_NUMS);
        ls::utils::fast_memcpy(validation, nums.get(), MAX_RAND_NUMS*sizeof(int));
        ls::utils::fast_memset(MERGE_SORT_TEMP_BUFFER.get(), 0, MAX_RAND_NUMS*sizeof(int));
        quick_sort_ref(validation, MAX_RAND_NUMS, ls::utils::IsLess<int>{});
        fprintf(stdout, "Done!\n");

        fprintf(stdout, "Testing a %s...", sortNames[i]);

        if (i < LS_ARRAY_SIZE(pSorts))
        {
            sortIndex = i - sortOffset;

            // Time the current sort method
            ticks.start(); // start time
            (*pSorts[sortIndex])(nums.get(), MAX_RAND_NUMS, ls::utils::IsLess<int>{}); // pointer to the sort function
            ticks.tick(); // stop time
        }
        else if (i < (LS_ARRAY_SIZE(pSorts)+LS_ARRAY_SIZE(pBufferedSorts)))
        {
            sortIndex = i - bufferedSortOffset;

            ticks.start(); // start time
            (*pBufferedSorts[sortIndex])(nums.get(), temp.get(), MAX_RAND_NUMS, ls::utils::IsLess<int>{});
            ticks.tick(); // stop time
        }
        else
        {
            sortIndex = i - threadedSortOffset;

            std::atomic_llong numSortPhases{0};
            for (unsigned t = 1; t < MAX_THREADS; ++t)
            {
                std::thread{pThreadedSorts[sortIndex], nums.get(), MAX_RAND_NUMS, MAX_THREADS, t, &numSortPhases, ls::utils::IsLess<int>{}}.detach();
            }

            ticks.start(); // start time
            pThreadedSorts[sortIndex](nums.get(), MAX_RAND_NUMS, MAX_THREADS, 0, &numSortPhases, ls::utils::IsLess<int>{});
            ticks.tick(); // stop time
        }

        const double timeToSort = ticks.tick_time().count();
        ticks.stop();

        fprintf(stdout, "Done!\n");
        fprintf(stdout, "The %s test took %.10f seconds.\n", sortNames[i], timeToSort);

        fprintf(stdout, "Verifying the %s...", sortNames[i]);
        long long matchPos = 0;
        while (matchPos < MAX_RAND_NUMS)
        {
            if (nums[matchPos] != validation[matchPos])
            {
                fprintf(stdout, "%d != %d\n", nums[matchPos], validation[matchPos]);
                break;
            }

            ++matchPos;
        }

        if (matchPos != MAX_RAND_NUMS)
        {
            fprintf(stdout, "Failed! Mismatch at position %lld\n", matchPos);

            #if 0
                print_nums(nums, matchPos, MAX_RAND_NUMS, sortNames[i], stdout);
            #endif
        }
        else
        {
            fprintf(stdout, "Passed!\n");
        }

        fprintf(stdout, "\n\n");
    }

    return 0;
}
