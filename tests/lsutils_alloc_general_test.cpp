
#include <cassert>
#include <cstdio>
#include <iostream>

#include "lightsky/utils/GeneralAllocator.hpp"



int test_single_allocations()
{
    constexpr unsigned alloc_table_size = 1024*1024;
    constexpr unsigned block_size = 512;
    constexpr unsigned max_allocations = alloc_table_size / block_size;

    // test allocator of 64 bytes in a 256-byte container
    ls::utils::GeneralAllocator<block_size, alloc_table_size> testAllocator;
    void** allocations = new void*[max_allocations];
    void* p = nullptr;
    void* last = nullptr;

    for (unsigned i = 0; i < max_allocations; ++i)
    {
        allocations[i] = nullptr;
    }

    for (unsigned testRuns = 0; testRuns < 3; ++testRuns)
    {
        for (unsigned i = 0; i <= max_allocations+1; ++i)
        {
            p = testAllocator.allocate();
            if (!p && i < max_allocations)
            {
                std::cerr << "Error: ran out of memory at block " << i << std::endl;
                return -1;
            }

            assert(p == nullptr || last < p);
            last = p;

            std::cout << "Allocated chunk " << i << ": " << p << std::endl;

            if (i >= max_allocations)
            {
                if (p != nullptr)
                {
                    std::cerr << "Error: test pointer 4 provided unknown memory!" << std::endl;
                    return -2;
                }
            }
            else
            {
                allocations[i] = p;
            }
        }

        // we should be able to retrieve and reallocate
        testAllocator.free(allocations[2]);
        allocations[2] = nullptr;

        p = testAllocator.allocate();
        if (!p)
        {
            std::cerr << "Error: Unable to reallocate block 2!" << std::endl;
            return -3;
        }
        else
        {
            allocations[2] = p;
        }

        p = testAllocator.allocate();
        if (p)
        {
            std::cerr << "Error: Allocated too many chunks!" << std::endl;
            return -4;
        }

        // free all chunks and try again
        for (unsigned i = 0; i < max_allocations; ++i)
        {
            testAllocator.free(allocations[i]);
            allocations[i] = nullptr;
        }
    }

    delete [] allocations;
    return 0;
}



int test_array_allocations()
{
    constexpr unsigned alloc_table_size = 1024*1024;
    constexpr unsigned block_size = 512;
    constexpr unsigned max_allocations = alloc_table_size / block_size;

    // test allocator of 64 bytes in a 256-byte container
    ls::utils::GeneralAllocator<block_size, alloc_table_size> testAllocator;
    void** allocations = new void*[max_allocations];
    void* p = nullptr;

    for (unsigned i = 0; i < max_allocations; ++i)
    {
        allocations[i] = nullptr;
    }

    for (unsigned testRuns = 0; testRuns < 3; ++testRuns)
    {
        for (unsigned i = 0; i < max_allocations/3; ++i)
        {
            p = testAllocator.allocate(block_size * 2);
            if (!p && i < max_allocations)
            {
                std::cerr << "Error: ran out of memory at block " << i << std::endl;
                return -1;
            }

            std::cout << "Allocated chunk " << i << ": " << p << std::endl;

            if (i >= max_allocations)
            {
                if (p != nullptr)
                {
                    std::cerr << "Error: test pointer 4 provided unknown memory!" << std::endl;
                    return -2;
                }
            }
            else
            {
                allocations[i] = p;
            }
        }

        // we should be able to retrieve and reallocate
        testAllocator.free(allocations[2], block_size * 2);
        allocations[2] = nullptr;

        p = testAllocator.allocate(block_size * 2);
        if (!p)
        {
            std::cerr << "Error: Unable to reallocate block 2!" << std::endl;
            return -3;
        }
        else
        {
            allocations[2] = p;
        }

        p = testAllocator.allocate(block_size * 2);
        if (p)
        {
            std::cerr << "Error: Allocated too many chunks!" << std::endl;
            return -4;
        }

        // free all chunks and try again
        for (unsigned i = 0; i < max_allocations; ++i)
        {
            testAllocator.free(allocations[i], block_size * 2);
            allocations[i] = nullptr;
        }
    }

    delete [] allocations;
    return 0;
}



int main()
{
    test_single_allocations();
    test_array_allocations();

    return 0;
}