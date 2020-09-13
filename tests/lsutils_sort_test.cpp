
/**
 * @file Testing implementations of different sorting methods.
 */

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Types.h"

#ifdef LS_ARCH_X86
    #include <immintrin.h>
#endif

#ifdef LS_COMPILER_GNU
    #include <x86intrin.h>
#elif defined(LS_COMPILER_MSC)
    #include <intrin.h>
#endif

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Sort.hpp"



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
 * FUNCTION PROTOTYPES
-----------------------------------------------------------------------------*/
// Verify that a list has been fully sorted
long long is_sorted(int* const nums, long long count);

// Function to run a set of tests
int bench_sorts(
    int numTests,
    void (* pTests[])(int* const nums, long long count, ls::utils::IsLess<int>),
    void (* pThreadedTest[])(int* const items, long long count, long long numThreads, long long threadId, std::atomic_llong* numThreadsFinished, std::atomic_llong* numSortPhases, ls::utils::IsLess<int>, ls::utils::IsGreater<int>),
    const char* const* testNames
);

// Print a list of numbers to a file
void print_nums(
    int* nums,
    int mismatchPos,
    long long count,
    const char* const testName,
    FILE* pFile
);

// Quick Sort (iterative)
void quick_sort_2(int* const nums, long long count, ls::utils::IsLess<int>);

// Quick Sort - Reference Implementation
void quick_sort_ref(int* const nums, long long count, ls::utils::IsLess<int>);

template <typename data_type>
struct RadixIndexer
{
    constexpr unsigned long long operator()(const typename ls::setup::EnableIf<ls::setup::IsIntegral<data_type>::value, data_type>::type& val) const noexcept
    {
        return (unsigned long long)val;
    }
};

template <typename data_type, class Comparator = ls::utils::IsLess<data_type>, class Indexer = RadixIndexer<data_type>, unsigned long long base = 256, unsigned long long mask = base-1ull>
void sort_radix(data_type* const items, long long count, Comparator cmp = Comparator{}) noexcept;

// Function to populate a list of random numbers
void gen_rand_nums(int* const nums, long long count);



/*-----------------------------------------------------------------------------
 * MAIN()
-----------------------------------------------------------------------------*/
int main(void)
{
    void (* pTests[])(int* const, long long, ls::utils::IsLess<int>) = {
        &ls::utils::sort_bubble<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_selection<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_insertion<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_shell<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_merge<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_merge_iterative<int, ls::utils::IsLess<int>>,
        &ls::utils::sort_quick<int, ls::utils::IsLess<int>>,
        &quick_sort_2,
        &ls::utils::sort_quick_iterative<int, ls::utils::IsLess<int>>,
        &quick_sort_ref,
        &sort_radix<int, ls::utils::IsLess<int>>,
        nullptr,
        nullptr
    };

    void (* pThreadedTests[])(int* const items, long long count, long long numThreads, long long threadId, std::atomic_llong* numThreadsFinished, std::atomic_llong* numSortPhases, ls::utils::IsLess<int>, ls::utils::IsGreater<int>) = {
        &ls::utils::sort_sheared<int>,
        &ls::utils::sort_bitonic<int>
    };

    const char* testNames[] = {
        "Bubble Sort",
        "Selection Sort",
        "Insertion Sort",
        "Shell Sort",
        "Merge Sort (recursive)",
        "Merge Sort (iterative)",
        "Quick Sort (recursive)",
        "Quick Sort (iterative)",
        "Quick Sort (with insertion sort)",
        "Quick Sort-Reference",
        "Radix Sort",
        "Shear Sort (Parallel)",
        "Bitonic Sort (Parallel)"
    };

    int numTests = sizeof(testNames) / sizeof(char*);

    srand(time(nullptr));

    if (!bench_sorts(numTests, pTests, pThreadedTests, testNames))
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
void gen_rand_nums(int* const nums, long long count)
{
    long long i = count;
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
    void (*pTests[])(int* const, long long, ls::utils::IsLess<int>),
    void (* pThreadedTests[])(int* const, long long, long long, long long, std::atomic_llong*, std::atomic_llong*, ls::utils::IsLess<int>, ls::utils::IsGreater<int>),
    const char* const* testNames
)
{
    int* const nums = (int*)malloc(sizeof(int) * MAX_RAND_NUMS);
    int* const validation = (int*)malloc(sizeof(int) * MAX_RAND_NUMS);

    if (!nums || !validation)
    {
        fprintf(
            stderr,
            "ERROR: Couldn't make an array of %d integers.\n",
            MAX_RAND_NUMS
        );
        return 0;
    }

    ls::utils::Clock<double> ticks;

    for (int i = 0, j = 0; i < numTests; ++i)
    {

        printf("Initializing a %s test...", testNames[i]);
        gen_rand_nums(nums, MAX_RAND_NUMS);
        memcpy(validation, nums, MAX_RAND_NUMS*sizeof(int));
        quick_sort_ref(validation, MAX_RAND_NUMS, ls::utils::IsLess<int>{});
        printf("Done!\n");

        printf("Testing a %s...", testNames[i]);

        if (pTests[i])
        {
            // Time the current sort method
            ticks.start(); // start time
            (*pTests[i])(nums, MAX_RAND_NUMS, ls::utils::IsLess<int>{}); // pointer to the sort function
            ticks.tick(); // stop time
        }
        else
        {
            std::atomic_llong numThreadsFinished{0};
            std::atomic_llong numSortPhases{0};
            for (unsigned t = 1; t < MAX_THREADS; ++t)
            {
                std::thread{pThreadedTests[j], nums, MAX_RAND_NUMS, MAX_THREADS, MAX_THREADS-1-t, &numThreadsFinished, &numSortPhases, ls::utils::IsLess<int>{}, ls::utils::IsGreater<int>{}}.detach();
            }

            // Time the current sort method
            ticks.start(); // start time

            pThreadedTests[j](nums, MAX_RAND_NUMS, MAX_THREADS, MAX_THREADS-1, &numThreadsFinished, &numSortPhases, ls::utils::IsLess<int>{}, ls::utils::IsGreater<int>{});

            ticks.tick(); // stop time

            j += 1;
        }

        const double timeToSort = ticks.tick_time().count();

        printf("Done!\n");
        printf("The %s test took %f seconds.\n", testNames[i], timeToSort);

        printf("Verifying the %s...", testNames[i]);
        const long long matchPos = (long long)(std::is_sorted_until(nums, nums+MAX_RAND_NUMS)-nums);

        if (matchPos != MAX_RAND_NUMS)
        {
            #if defined(DEBUG)
                print_nums(nums, matchPos, MAX_RAND_NUMS, testNames[i], stdout);
            #endif

            printf("Failed! Mismatch at position %lld\n", matchPos);
        }
        else
        {
            printf("Passed!\n");
        }

        printf("\n\n");
    }

    free(validation);
    free(nums);

    return 1;
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
 * Verify that a list has been sorted
-----------------------------------------------------------------------------*/
long long is_sorted(int* const nums, long long count)
{
    long long i;

    for (i = 0; (i < count - 1); ++i)
    {
        if (nums[i] > nums[i + 1])
        {
            return i+1;
        }
    }

    return 0;
}



/*-----------------------------------------------------------------------------
 * Quick Sort (iterative)
-----------------------------------------------------------------------------*/
// Function to partition an array during a quicksort
long long partitionqsort2(int* nums, long long l, long long r)
{
    int temp;
    int pivot;
    long long i;
    long long mid = (l + r) >> 1;

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
void quick_sort_2(int* const nums, long long count, ls::utils::IsLess<int>)
{
    long long stack[64];
    long long mid;
    long long space = 0;
    long long l = 0;
    long long r = count - 1;

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
void quick_sort_ref(int* const nums, long long count, ls::utils::IsLess<int>)
{
    qsort(nums, (long long)count, sizeof(int), &qsortcompare);
}



/*-----------------------------------------------------------------------------
 * Quick Sort Reference Implementation
-----------------------------------------------------------------------------*/
// The main function to that sorts arr[] of size n using
// Radix Sort
template <typename data_type, class Comparator, class Indexer, unsigned long long base, unsigned long long mask>
void sort_radix(data_type* const items, long long count, Comparator) noexcept
{
    if (count <= 0ll)
    {
        return;
    }

    // Find the maximum value to know number of digits
    constexpr Indexer indexer;
    unsigned long long m = indexer(*items);

    for (long long i = 1ll; i < count; ++i)
    {
        unsigned long long val = indexer(items[i]);
        if (val > m)
        {
            m = val;
        }
    }

    // output array
    ls::utils::Pointer<data_type[]>&& indices = ls::utils::make_unique_array<data_type>(count);

    const auto&& ctz = [](long long e)->long long
    {
        #if defined(LS_COMPILER_MSC) && defined(LS_ARCH_X86) && LS_ARCH_X86 == 64
            unsigned long ret;
            if (_BitScanForward64(&ret, (unsigned long long)e))
            {
                return (long long)ret;
            }
            return 64ll;
        #elif defined(LS_ARCH_X86)
            return (long long)_tzcnt_u64((unsigned long long)e);
        #elif defined(LS_COMPILER_GNU)
            return __builtin_ctzll(e);
        #else
            long long ret = 0ll;
            while (!(n & 0x01ll))
            {
                n >>= 1ll;
                ++ret;
            }
            return ret;
        #endif
    };

    // Do counting sort for every digit. Note that instead
    // of passing digit number, exp is passed. exp is 10^i
    // where i is current digit number
    for (long long exponent = 1ll, divisor = 0ll; (m >> divisor) > 0ll; exponent *= base, divisor = ctz(exponent))
    {
        unsigned long long radices[base] = {0ull};

        // Store count of occurrences in radices[]
        for (long long i = 0; i < count; ++i)
        {
            const unsigned long long inIndex = indexer(items[i]);
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
        for (long long i = count - 1ll; i >= 0ll; i--)
        {
            const data_type&   elem     = items[i];
            unsigned long long radix    = indexer(elem);
            unsigned long long inIndex  = (radix >> divisor) & mask;
            unsigned long long outIndex = radices[inIndex] - 1ull;

            indices[outIndex] = elem;
            radices[inIndex]--;
        }

        // Copy the output array to arr[], so that arr[] now
        // contains sorted numbers according to current digit
        for (long long i = 0ll; i < count; i++)
        {
            items[i] = indices[i];
        }
    }
}



/*
https://pdfs.semanticscholar.org/3536/3f8ccda03736320f3ebff92e744dbd245257.pdf

template <typename data_type, class LessComparator, class GreaterComparator>
void sort_quick_parallel(
    data_type* const items,
    long long count,
    long long numThreads,
    long long threadId,
    std::atomic_llong* numThreadsFinished,
    std::atomic_llong* numSortPhases,
    LessComparator cmpL,
    GreaterComparator cmpG) noexcept
{

Algorithm ParallelQuicksort(data_set, n, p)
{
    threshold : = (p > 1)? (1 + n / (p << 3)): n
    submitToThreadPool (PQuicksort (data_set, 0, n - 1))
    // wait for all threads to complete execution
}

Algorithm PQuicksort (data_set, low, high)
{
    if ((high - low) < threshold)
    {
        sortDirectly (data_set, low, high)
    }
    else
    {
        i := low
        j := high
        pivot:= data_set[(low + (high-low)/2)

        while i <= j
        {
            while data_set[i] < pivot
            {
                increment i
            }

            while data_set[j] > pivot
            {
                decrement j
            }

            if i <= j
            {
                swap data_set[i] with data_set[j]
                increment i
                decrement j
            }
        }
        if low < j
        {
             submitToThreadPool(PQuicksort(data_set, low, j))
         }
        if i < high
        {
             submitToThreadPool (PQuicksort(data_set, i, high)
         }
    }
}
}
*/
