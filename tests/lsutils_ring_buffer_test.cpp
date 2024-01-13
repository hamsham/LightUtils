/*
 * File:   lsutils_ring_buffer_test.cpp
 * Author: hammy
 *
 * Created on Dec 21, 2023 at 9:14 PM
 */

#include "lightsky/utils/RingBuffer.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    ls::utils::RingBuffer<unsigned> buffer;

    buffer.reserve(3);
    LS_ASSERT(buffer.capacity() == 3);
    LS_ASSERT(buffer.size() == 0);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(buffer.empty());

    buffer.push_unchecked(0u);
    buffer.push_unchecked(1u);
    buffer.push_unchecked(2u);
    LS_ASSERT(buffer.size() == 3);
    LS_ASSERT(buffer.capacity() == 3);
    LS_ASSERT(buffer.full());
    LS_ASSERT(!buffer.empty());

    unsigned val;

    val = buffer.pop_unchecked();
    LS_ASSERT(val == 0u);
    LS_ASSERT(buffer.front() == 1u);
    LS_ASSERT(buffer.back() == 2u);
    LS_ASSERT(buffer.size() == 2);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(!buffer.empty());

    val = buffer.pop_unchecked();
    LS_ASSERT(val == 1u);
    LS_ASSERT(buffer.front() == 2u);
    LS_ASSERT(buffer.back() == 2u);
    LS_ASSERT(buffer.size() == 1);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(!buffer.empty());

    buffer.emplace(0u);
    LS_ASSERT(buffer.front() == 2u);
    LS_ASSERT(buffer.back() == 0u);
    LS_ASSERT(buffer.size() == 2);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(!buffer.empty());

    buffer.push(1u);
    LS_ASSERT(buffer.front() == 2u);
    LS_ASSERT(buffer.back() == 1u);
    LS_ASSERT(buffer.size() == 3);
    LS_ASSERT(buffer.full());
    LS_ASSERT(!buffer.empty());

    val = buffer.pop_unchecked();
    LS_ASSERT(val == 2u);
    LS_ASSERT(buffer.front() == 0u);
    LS_ASSERT(buffer.back() == 1u);
    LS_ASSERT(buffer.size() == 2);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(!buffer.empty());

    val = buffer.pop_unchecked();
    LS_ASSERT(val == 0u);
    LS_ASSERT(buffer.front() == 1u);
    LS_ASSERT(buffer.back() == 1u);
    LS_ASSERT(buffer.size() == 1);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(!buffer.empty());

    buffer.shrink_to_fit();
    LS_ASSERT(buffer.front() == 1u);
    LS_ASSERT(buffer.back() == 1u);
    LS_ASSERT(buffer.size() == 1);
    LS_ASSERT(buffer.capacity() == 1);
    LS_ASSERT(buffer.full());
    LS_ASSERT(!buffer.empty());

    val = buffer.pop_unchecked();
    LS_ASSERT(val == 1u);
    LS_ASSERT(buffer.size() == 0);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(buffer.empty());

    buffer.shrink_to_fit();
    LS_ASSERT(buffer.size() == 0);
    LS_ASSERT(buffer.capacity() == 0);
    LS_ASSERT(!buffer.full());
    LS_ASSERT(buffer.empty());

    return 0;
}
