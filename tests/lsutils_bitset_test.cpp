/*
 * File:   lsutils_bitset_test.cpp
 * Author: miles
 * Created on December 29, 2025, at 5:18 a.m.
 */

#include <iostream>

#include "lightsky/utils/BitSet.hpp"

namespace utils = ls::utils;



template <typename ElementType>
std::ostream& operator <<(std::ostream& os, const utils::BitSet<ElementType>& bitset)
{
    typedef ls::utils::BitSet<ElementType> BitSetType;
    const typename BitSetType::value_type* pBits = bitset.data();
    const typename BitSetType::size_type numBits = bitset.size();
    const typename BitSetType::size_type numElements = numBits / BitSetType::bits_per_bucket;
    typename BitSetType::size_type i = numElements;

    while (i--)
    {
        typename BitSetType::value_type localBits = pBits[i];
        typename BitSetType::value_type j = BitSetType::bits_per_bucket;

        while (j--)
        {
            os << ((localBits >> j) & 1);
        }

        if (i > 0)
        {
            os << "`";
        }
    }

    return os;
}



template <typename ElementType>
void test_bit_set()
{
    constexpr typename utils::BitSet<ElementType>::size_type numBits = 24;
    constexpr unsigned char bits[] = {
        0b00110001, 0b11110000, 0b11110111, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000
    };

    utils::BitSet<ElementType> bitset0{numBits, reinterpret_cast<const ElementType*>(bits)};
    bitset0.resize(57);
    LS_ASSERT(bitset0.size() == 64);

    utils::BitSet<ElementType> bitset1 = bitset0;
    LS_ASSERT(bitset0 == bitset1);

    bitset0.reserve(255);
    LS_ASSERT(!(bitset0 != bitset1));

    bitset0.set_and(bitset1);
    bitset0.set_not();
    bitset0.set_xor(bitset1); // all bits set
    bitset0.set_not(); // no bits set
    bitset0.set_or(bitset1); // original bits

    bitset0.set(63, 1); // set
    bitset0.set(62, 1);
    bitset0.bit_not(63); // clear
    bitset0.bit_not(62);
    bitset0.bit_or(30, 1); // set
    bitset0.bit_or(31, 1);
    bitset0.bit_xor(31, 1); // clear
    bitset0.bit_xor(62, 1); // set

    std::cout << "BitSet Test"
        << "\n\tBits:       " << bitset0
        << "\n\tSize:       " << bitset0.size()
        << "\n\tCapacity:   " << bitset0.capacity()
        << "\n\tBit Width:  " << bitset0.bucket_size()
        << "\n\tBuckets:    " << bitset0.bucket_count()
        << "\n\tAND(30, 1): " << (unsigned)bitset0.cbit_and(30, 1)
        << "\n\tAND(30, 0): " << (unsigned)bitset0.cbit_and(30, 0)
        << "\n\tOR(30, 1):  " << (unsigned)bitset0.cbit_or(30, 1)
        << "\n\tOR(30, 0):  " << (unsigned)bitset0.cbit_or(30, 0)
        << "\n\tXOR(30, 1): " << (unsigned)bitset0.cbit_xor(30, 1)
        << "\n\tXOR(30, 0): " << (unsigned)bitset0.cbit_xor(30, 0)
        << "\n\tNOT(30):    " << (unsigned)bitset0.cbit_not(30)
        << "\n\tNOT(31):    " << (unsigned)bitset0.cbit_not(31)
        << '\n' << std::endl;
}



int main()
{
    test_bit_set<uint8_t>();
    test_bit_set<uint16_t>();
    test_bit_set<uint32_t>();
    test_bit_set<uint64_t>();

    return 0;
}