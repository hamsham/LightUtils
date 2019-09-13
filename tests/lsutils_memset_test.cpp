
#include <cstring>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"







int main()
{
    constexpr unsigned int numBytes = 1024*1024*1024*sizeof(char)-1; // 1 gigabyte
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ls::utils::Pointer<char[], ls::utils::AlignedDeleter> pTest{(char*)ls::utils::aligned_malloc(numBytes)};
    ls::utils::Pointer<char[], ls::utils::AlignedDeleter> pDst{(char*)ls::utils::aligned_malloc(numBytes)};

    std::cout << "Initializing Memset Benchmark..." << std::endl;

    for (unsigned i = numBytes; i--;)
    {
        pTest[i] = (char)0xFF;
    }

    std::cout << "\tDone." << std::endl;

    std::cout << "Running memset() benchmark..." << std::endl;
    ticks.start();
    memset(pDst.get(), 0xFF, numBytes);
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
        << "\n\tMemset:    " << memsetTime//   / 1000ull
        << "\n\tStd Fill:  " << stdFillTime//  / 1000ull
        << "\n\tLS Memset: " << lsMemsetTime// / 1000ull
        << "\n\tLS Fill:   " << lsFillTime//   / 1000ull
        << std::endl;

    std::cout << "Estimated bandwidth (write-only): " << 2.0l/((long double)lsMemsetTime/1000.0l) << " Gb/s" << std::endl;

    return 0;
}
