
#include <cstring>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"



int main()
{
    constexpr unsigned int numBytes = 512*1024*1024*sizeof(char)-1; // 1 gigabyte
    constexpr long double numGigs = (long double)numBytes * 0.000001l;
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ls::utils::UniqueAlignedArray<char> pTest = ls::utils::make_unique_aligned_array<char>(numBytes);
    ls::utils::UniqueAlignedArray<char> pDst  = ls::utils::make_unique_aligned_array<char>(numBytes);

    std::cout << "Initializing Memset Benchmark..." << std::endl;

    for (unsigned i = numBytes; i--;)
    {
        pTest[i] = (char)0xFF;
        pDst[i] = '\0';
    }

    std::cout << "\tDone." << std::endl;

    std::cout << "Running memset() benchmark..." << std::endl;
    ticks.start();
    memset(pDst.get(), '\xFF', numBytes);
    ticks.tick();
    const unsigned long long memsetTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pTest.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Running std::fill() benchmark..." << std::endl;
    ticks.start();
    std::fill(pDst.get(), pDst.get()+numBytes, 0xFF);
    ticks.tick();
    const unsigned long long stdFillTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pTest.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Running ls::utils::fast_memset() benchmark..." << std::endl;
    ticks.start();
    ls::utils::fast_memset(pDst.get(), 0xFF, numBytes);
    ticks.tick();
    const unsigned long long lsMemsetTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pTest.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Running ls::utils::fast_fill() benchmark..." << std::endl;
    ticks.start();
    ls::utils::fast_fill(pDst.get(), 0xFF, numBytes);
    ticks.tick();
    const unsigned long long lsFillTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pTest.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Copy Time:"
        << "\n\tMemset:    " << numGigs/(long double)memsetTime   << " Gb/s @ " << memsetTime   << "ms"
        << "\n\tStd Fill:  " << numGigs/(long double)stdFillTime  << " Gb/s @ " << stdFillTime  << "ms"
        << "\n\tLS Memset: " << numGigs/(long double)lsMemsetTime << " Gb/s @ " << lsMemsetTime << "ms"
        << "\n\tLS Fill:   " << numGigs/(long double)lsFillTime   << " Gb/s @ " << lsFillTime   << "ms"
        << std::endl;

    return 0;
}
