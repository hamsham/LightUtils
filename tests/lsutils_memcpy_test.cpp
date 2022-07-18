
#include <cstring>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/RandomNum.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"



void init_data(char* const pSrc, char* const pDst, unsigned long long numBytes) noexcept
{
    std::cout << "Initializing Copy Benchmark..." << std::endl;

    ls::utils::RandomNum randGen;
    randGen.seed();

    for (unsigned i = numBytes; i--;)
    {
        pSrc[i] = (char)randGen();
        pDst[i] = '\0';
    }

    std::cout << "\tDone." << std::endl;
}



void validate_data(const char* const pSrc, const char* const pDst, unsigned long long numBytes) noexcept
{
    for (unsigned i = numBytes; i--;)
    {
        if (pDst[i] != pSrc[i])
        {
            std::cerr << "Mismatch at element " << i << ". " << pDst[i] << " != " << pSrc[i] << '.' << std::endl;
            LS_ASSERT(pDst[i] == pSrc[i]);
        }
    }
}



template <typename MemcpyFunc>
unsigned long long benchmark_memcpy(
    const char* testName,
    char* const pSrc,
    char* const pDst,
    unsigned long long numBytes,
    MemcpyFunc&& pMemcpy
) noexcept
{
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    init_data(pSrc, pDst, numBytes);

    std::cout << "Running " << testName << " benchmark..." << std::endl;

    ticks.start();
    pMemcpy();
    ticks.tick();

    const unsigned long long memsetTime = ticks.tick_time().count();

    validate_data(pSrc, pDst, numBytes);

    std::cout << "\tDone." << std::endl;

    return memsetTime;
}



int main()
{
    constexpr unsigned int numBytes = 256*1024*1024*sizeof(char)-1; // 500 Megabytes
    constexpr long double numGigs = (long double)numBytes * 0.000001l;
    ls::utils::UniqueAlignedArray<char> pSrc = ls::utils::make_unique_aligned_array<char>(numBytes);
    ls::utils::UniqueAlignedArray<char> pDst = ls::utils::make_unique_aligned_array<char>(numBytes);

    const unsigned long long memsetTime = benchmark_memcpy("memcpy()", pSrc.get(), pDst.get(), numBytes, [&]() noexcept->void
    {
        std::memcpy(pDst.get(), pSrc.get(), numBytes);
    });

    const unsigned long long stdCopyTime = benchmark_memcpy("std::copy()", pSrc.get(), pDst.get(), numBytes, [&]() noexcept->void
    {
        std::copy(pSrc.get(), pSrc.get()+numBytes, pDst.get());
    });

    const unsigned long long lsMemcpyTime = benchmark_memcpy("ls::utils::fast_memcpy()", pSrc.get(), pDst.get(), numBytes, [&]() noexcept->void
    {
        ls::utils::fast_memcpy(pDst.get(), pSrc.get(), numBytes);
    });

    const unsigned long long lsCopyTime = benchmark_memcpy("ls::utils::fast_copy()", pSrc.get(), pDst.get(), numBytes, [&]() noexcept->void
    {
        ls::utils::fast_copy(pDst.get(), pSrc.get(), numBytes);
    });

    std::cout << "Copy Time:"
        << "\n\tMemcpy:    " << numGigs/(long double)memsetTime   << " Gb/s @ " << memsetTime   << "ms"
        << "\n\tStd Copy:  " << numGigs/(long double)stdCopyTime  << " Gb/s @ " << stdCopyTime  << "ms"
        << "\n\tLS Memcpy: " << numGigs/(long double)lsMemcpyTime << " Gb/s @ " << lsMemcpyTime << "ms"
        << "\n\tLS Copy:   " << numGigs/(long double)lsCopyTime   << " Gb/s @ " << lsCopyTime   << "ms"
        << std::endl;

    return 0;
}
