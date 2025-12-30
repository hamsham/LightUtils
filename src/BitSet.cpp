/*
 * File:   BitSet.cpp
 * Author: miles
 * Created on December 28, 2025, at 10:11 p.m.
 */

#include "lightsky/utils/BitSet.hpp"



/*-----------------------------------------------------------------------------
 * BitSet Implementation
-----------------------------------------------------------------------------*/
namespace ls::utils
{

/*--------------------------------------
 * Constructor
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>::BitSet(size_type numBits, const element_type* bits) noexcept :
    BitSet{}
{
    resize(numBits, bits);
}



/*--------------------------------------
 * Copy constructor
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>::BitSet(const BitSet& bitSet) noexcept :
    BitSet{}
{
    const size_type numElements = bitSet.size() / bits_per_element;
    const size_type numReserved = bitSet.capacity() / bits_per_element;

    if (numElements > 0)
    {
        mBits = ls::utils::make_unique_aligned_array<element_type>(numReserved);
    }

    if (mBits != nullptr)
    {
        for (size_type i = 0; i < numElements; ++i)
        {
            mBits[i] = bitSet.mBits[i];
        }

        mNumBitsActive = bitSet.size();
        mNumBitsReserved = bitSet.capacity();
    }
}



/*--------------------------------------
 * Move constructor
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>::BitSet(BitSet&& bitSet) noexcept :
    mBits{ls::setup::move(bitSet.mBits)},
    mNumBitsActive{bitSet.mNumBitsActive},
    mNumBitsReserved{bitSet.mNumBitsReserved}
{
    bitSet.mNumBitsActive = 0;
    bitSet.mNumBitsReserved = 0;
}



/*--------------------------------------
 * Copy operator
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>& BitSet<ElementType>::operator=(const BitSet& bitSet) noexcept
{
    if (this == &bitSet)
    {
        return *this;
    }

    const size_type numElements = bitSet.size() / bits_per_element;
    const size_type numReserved = bitSet.capacity() / bits_per_element;
    ls::utils::UniqueAlignedArray<element_type> bits;

    if (numElements > 0)
    {
        bits = ls::utils::make_unique_aligned_array<element_type>(numReserved);
    }

    if (bits != nullptr)
    {
        for (size_type i = 0; i < numElements; ++i)
        {
            bits[i] = bitSet.mBits[i];
        }

        mBits = ls::setup::move(bits);
        mNumBitsActive = bitSet.size();
        mNumBitsReserved = bitSet.capacity();
    }

    return *this;
}



/*--------------------------------------
 * Move operator
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>& BitSet<ElementType>::operator=(BitSet&& bitSet) noexcept
{
    if (this != &bitSet)
    {
        mBits = ls::setup::move(bitSet.mBits);

        mNumBitsActive = bitSet.mNumBitsActive;
        bitSet.mNumBitsActive = 0;

        mNumBitsReserved = bitSet.mNumBitsReserved;
        bitSet.mNumBitsReserved = 0;
    }

    return *this;
}



/*--------------------------------------
 * Equality operator
--------------------------------------*/
template <typename ElementType>
bool BitSet<ElementType>::operator==(const BitSet& bitSet) const noexcept
{
    bool result = true;

    if (this != &bitSet)
    {
        if (size() != bitSet.size())
        {
            result = false;
        }
        else
        {
            const size_type numElements = size() / bits_per_element;
            for (size_type i = 0; i < numElements; ++i)
            {
                if (mBits[i] != bitSet.mBits[i])
                {
                    result = false;
                    break;
                }
            }
        }
    }

    return result;
}



/*--------------------------------------
 * Equality operator
--------------------------------------*/
template <typename ElementType>
bool BitSet<ElementType>::operator!=(const BitSet& bitSet) const noexcept
{
    bool result = false;

    if (this != &bitSet)
    {
        if (size() != bitSet.size())
        {
            result = true;
        }
        else
        {
            const size_type numElements = size() / bits_per_element;
            for (size_type i = 0; i < numElements; ++i)
            {
                if (mBits[i] != bitSet.mBits[i])
                {
                    result = true;
                    break;
                }
            }
        }
    }

    return result;
}



/*--------------------------------------
 * Clear all data
--------------------------------------*/
template <typename ElementType>
void BitSet<ElementType>::clear() noexcept
{
    mBits.reset();
    mNumBitsActive = 0;
    mNumBitsReserved = 0;
}

/*--------------------------------------
 * Resize internal bit-array
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>::size_type BitSet<ElementType>::resize(size_type numBits, const element_type* bits) noexcept
{
    if (!numBits)
    {
        clear();
        return 0;
    }

    if (numBits == mNumBitsActive)
    {
        return numBits;
    }

    const size_type multiple = ((numBits + bits_per_element - 1) / bits_per_element);
    size_type reserved = 0;
    ls::utils::UniqueAlignedArray<element_type> tmp;

    if (multiple < (mNumBitsReserved/bits_per_element))
    {
        tmp = ls::setup::move(mBits);
        reserved = (mNumBitsReserved/bits_per_element);
    }
    else
    {
        constexpr size_type slack = sizeof(element_type);// * 2;
        reserved = ((multiple + slack - 1) / slack) * slack;

        tmp = ls::utils::make_unique_aligned_array<element_type>(reserved);
        if (tmp == nullptr)
        {
            return 0;
        }
    }

    size_type i = 0;
    if (bits != nullptr)
    {
        for (; i < multiple; ++i)
        {
            tmp[i] = bits[i];
        }
    }
    else if (tmp && mBits) // new allocation
    {
        const size_type bytesToCopy = mNumBitsActive / bits_per_element;
        for (; i < bytesToCopy; ++i)
        {
            tmp[i] = mBits[i];
        }
    }
    else
    {
        // have existing data
        i = multiple;
    }

    for (; i < reserved; ++i)
    {
        tmp[i] = 0;
    }

    mBits = ls::setup::move(tmp);
    mNumBitsActive = multiple * bits_per_element;
    mNumBitsReserved = reserved * bits_per_element;

    return mNumBitsActive;
}

/*--------------------------------------
 * Reserve capacity
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>::size_type BitSet<ElementType>::reserve(size_type numBits) noexcept
{
    const size_type multiple = ((numBits + bits_per_element - 1) / bits_per_element);
    if (!multiple)
    {
        clear();
        return 0;
    }

    if (numBits < mNumBitsActive)
    {
        const size_type activeElements = mNumBitsActive / bits_per_element;
        const size_type elementIndex = multiple-1;
        const size_type bitIndex = numBits % bits_per_element;
        constexpr element_type mask = ~(element_type)0;

        mBits[elementIndex] = mBits[elementIndex] & ~(mask << bitIndex);
        for (size_type i = multiple; i < activeElements; ++i)
        {
            mBits[i] = 0;
        }

        mNumBitsActive = multiple;
        return mNumBitsReserved;
    }

    const size_type reservedElements = mNumBitsReserved / bits_per_element;
    if (multiple > reservedElements)
    {
        ls::utils::UniqueAlignedArray<element_type> tmp = ls::utils::make_unique_aligned_array<element_type>(multiple);
        if (!tmp)
        {
            return 0;
        }

        size_type i = 0;
        for (; i < reservedElements; ++i)
        {
            tmp[i] = mBits[i];
        }

        for (; i < multiple; ++i)
        {
            tmp[i] = 0;
        }

        mBits = ls::setup::move(tmp);
        mNumBitsReserved = multiple * bits_per_element;
    }

    return mNumBitsReserved;
}



/*--------------------------------------
 * AND all bits
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>& BitSet<ElementType>::set_and(const BitSet& bitSet) noexcept
{
    LS_ASSERT(size() == bitSet.size());
    const size_type numElements = size() / bits_per_element;

    for (size_type i = 0; i < numElements; ++i)
    {
        mBits[i] &= bitSet.mBits[i];
    }

    return *this;
}

/*--------------------------------------
 * OR all bits
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>& BitSet<ElementType>::set_or(const BitSet& bitSet) noexcept
{
    LS_ASSERT(size() == bitSet.size());
    const size_type numElements = size() / bits_per_element;

    for (size_type i = 0; i < numElements; ++i)
    {
        mBits[i] |= bitSet.mBits[i];
    }

    return *this;
}

/*--------------------------------------
 * XOR all bits
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>& BitSet<ElementType>::set_xor(const BitSet& bitSet) noexcept
{
    LS_ASSERT(size() == bitSet.size());
    const size_type numElements = size() / bits_per_element;

    for (size_type i = 0; i < numElements; ++i)
    {
        mBits[i] ^= bitSet.mBits[i];
    }

    return *this;
}

/*--------------------------------------
 * NOT all bits
--------------------------------------*/
template <typename ElementType>
BitSet<ElementType>& BitSet<ElementType>::set_not() noexcept
{
    const size_type numElements = size() / bits_per_element;

    for (size_type i = 0; i < numElements; ++i)
    {
        mBits[i] = ~mBits[i];
    }

    return *this;
}

} // end ls::utils namespace

LS_DEFINE_CLASS_TYPE(ls::utils::BitSet, uint8_t);
LS_DEFINE_CLASS_TYPE(ls::utils::BitSet, uint16_t);
LS_DEFINE_CLASS_TYPE(ls::utils::BitSet, uint32_t);
LS_DEFINE_CLASS_TYPE(ls::utils::BitSet, uint64_t);
