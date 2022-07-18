
#include <cstring>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"



void init_data(char* const pDst, unsigned long long numBytes) noexcept
{
    std::cout << "Initializing Memset Benchmark..." << std::endl;

    for (unsigned i = numBytes; i--;)
    {
        pDst[i] = '\0';
    }

    std::cout << "\tDone." << std::endl;
}



void validate_data(const char* const pDst, unsigned long long numBytes) noexcept
{
    for (unsigned i = numBytes; i--;)
    {
        if (pDst[i] != (char)'\xFF')
        {
            std::cerr << "Mismatch at element " << i << ". " << pDst[i] << " != 0xFF" << std::endl;
            LS_ASSERT(pDst[i] == (char)'\xFF');
        }
    }
}



template <typename MemsetFunc>
unsigned long long benchmark_memset(
    const char* testName,
    char* const pDst,
    unsigned long long numBytes,
    MemsetFunc&& pMemset
) noexcept
{
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    init_data(pDst, numBytes);

    std::cout << "Running " << testName << " benchmark..." << std::endl;

    ticks.start();
    pMemset();
    ticks.tick();

    const unsigned long long memsetTime = ticks.tick_time().count();

    validate_data(pDst, numBytes);

    std::cout << "\tDone." << std::endl;

    return memsetTime;
}



int main()
{
    constexpr unsigned int numBytes = 1024*1024*1024*sizeof(char)-1; // 1 gigabyte
    constexpr long double numGigs = (long double)numBytes * 0.000001l;
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ls::utils::UniqueAlignedArray<char> pDst  = ls::utils::make_unique_aligned_array<char>(numBytes);

    const unsigned long long memsetTime = benchmark_memset("memset()", pDst.get(), numBytes, [&]() noexcept->void
    {
        std::memset(pDst.get(), '\xFF', numBytes);
    });

    const unsigned long long stdFillTime = benchmark_memset("std::fill()", pDst.get(), numBytes, [&]() noexcept->void
    {
        std::fill(pDst.get(), pDst.get()+numBytes, '\xFF');
    });

    const unsigned long long lsMemsetTime = benchmark_memset("ls::utils::fast_memset()", pDst.get(), numBytes, [&]() noexcept->void
    {
        ls::utils::fast_memset(pDst.get(), '\xFF', numBytes);
    });

    const unsigned long long lsFillTime = benchmark_memset("ls::utils::fast_fill()", pDst.get(), numBytes, [&]() noexcept->void
    {
        ls::utils::fast_fill<char, char>(pDst.get(), '\xFF', numBytes);
    });

    std::cout << "Copy Time:"
        << "\n\tMemset:    " << numGigs/(long double)memsetTime   << " Gb/s @ " << memsetTime   << "ms"
        << "\n\tStd Fill:  " << numGigs/(long double)stdFillTime  << " Gb/s @ " << stdFillTime  << "ms"
        << "\n\tLS Memset: " << numGigs/(long double)lsMemsetTime << " Gb/s @ " << lsMemsetTime << "ms"
        << "\n\tLS Fill:   " << numGigs/(long double)lsFillTime   << " Gb/s @ " << lsFillTime   << "ms"
        << std::endl;

    return 0;
}
