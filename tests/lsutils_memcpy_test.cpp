
#include <cstring>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/RandomNum.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"



int main()
{
    constexpr unsigned int numBytes = 1024*1024*1024*sizeof(char)-1; // 1 gigabyte
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ls::utils::RandomNum randGen;
    ls::utils::Pointer<char[], ls::utils::AlignedDeleter> pSrc{(char*)ls::utils::aligned_malloc(numBytes)};
    ls::utils::Pointer<char[], ls::utils::AlignedDeleter> pDst{(char*)ls::utils::aligned_malloc(numBytes)};

    std::cout << "Initializing Copy Benchmark..." << std::endl;

    randGen.seed();

    for (unsigned i = numBytes; i--;)
    {
        pSrc[i] = randGen();
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
        << "\n\tMemcpy:    " << memsetTime//   / 1000ull
        << "\n\tStd Copy:  " << stdCopyTime//  / 1000ull
        << "\n\tLS Memcpy: " << lsMemcpyTime// / 1000ull
        << "\n\tLS Copy:   " << lsCopyTime//   / 1000ull
        << std::endl;

    std::cout << "Estimated bandwidth (read+write): " << 2.0l/((long double)lsMemcpyTime/1000.0l) << " Gb/s" << std::endl;

    return 0;
}
