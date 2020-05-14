
#include <cstring>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/RandomNum.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"



int main()
{
    constexpr unsigned int numBytes = 256*1024*1024*sizeof(char)-1; // 500 Megabytes
    constexpr long double numGigs = (long double)numBytes * 0.000001l;
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ls::utils::RandomNum randGen;
    ls::utils::Pointer<char[], ls::utils::AlignedDeleter> pSrc{(char*)ls::utils::aligned_malloc(numBytes)};
    ls::utils::Pointer<char[], ls::utils::AlignedDeleter> pDst{(char*)ls::utils::aligned_malloc(numBytes)};

    std::cout << "Initializing Copy Benchmark..." << std::endl;

    randGen.seed();

    for (unsigned i = numBytes; i--;)
    {
        pSrc[i] = randGen();
        pDst[i] = '\0';
    }

    std::cout << "\tDone." << std::endl;

    std::cout << "Running memset() benchmark..." << std::endl;
    ticks.start();
    memcpy(pDst.get(), pSrc.get(), numBytes);
    ticks.tick();
    const unsigned long long memsetTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pSrc.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Running std::copy() benchmark..." << std::endl;
    ticks.start();
    std::copy(pSrc.get(), pSrc.get()+numBytes, pDst.get());
    ticks.tick();
    const unsigned long long stdCopyTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pSrc.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Running ls::utils::fast_memcpy() benchmark..." << std::endl;
    ticks.start();
    ls::utils::fast_memcpy(pDst.get(), pSrc.get(), numBytes);
    ticks.tick();
    const unsigned long long lsMemcpyTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pSrc.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Running ls::utils::fast_copy() benchmark..." << std::endl;
    ticks.start();
    ls::utils::fast_copy(pDst.get(), pSrc.get(), numBytes);
    ticks.tick();
    const unsigned long long lsCopyTime = ticks.tick_time().count();
    LS_ASSERT(std::memcmp(pSrc.get(), pDst.get(), numBytes) == 0);
    std::cout << "\tDone." << std::endl;

    std::cout << "Copy Time:"
        << "\n\tMemcpy:    " << numGigs/(long double)memsetTime   << " Gb/s @ " << memsetTime   << "ms"
        << "\n\tStd Copy:  " << numGigs/(long double)stdCopyTime  << " Gb/s @ " << stdCopyTime  << "ms"
        << "\n\tLS Memcpy: " << numGigs/(long double)lsMemcpyTime << " Gb/s @ " << lsMemcpyTime << "ms"
        << "\n\tLS Copy:   " << numGigs/(long double)lsCopyTime   << " Gb/s @ " << lsCopyTime   << "ms"
        << std::endl;

    return 0;
}
