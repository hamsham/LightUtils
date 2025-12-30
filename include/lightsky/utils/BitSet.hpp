/*
 * File:   BitSet.hpp
 * Author: miles
 * Created on December 28, 2025, at 10:10 p.m.
 */

#ifndef LS_UTILS_BITSET_HPP
#define LS_UTILS_BITSET_HPP

#include <limits> // CHAR_BIT
#include <utility> // std::move

#include "lightsky/setup/Macros.h" // LS_DECLARE_CLASS_TYPE, LS_DEFINE_CLASS_TYPE
#include "lightsky/setup/Types.h" // setup::move(), setup::IsUnsigned
#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Pointer.h"



/*-----------------------------------------------------------------------------
 * BitSet Declaration & Inlines
-----------------------------------------------------------------------------*/
namespace ls::utils
{


/**
 * Standard Bit-Set implementation. Not thread-safe
 */
template <typename ElementType>
class BitSet final
{
    static_assert(ls::setup::IsUnsigned<ElementType>::value,
        "Template parameter, 'ElementType,' must be an unsigned integral type.");

public:
    typedef unsigned long long size_type;
    typedef ElementType element_type;

    enum : size_type
    {
        bytes_per_element = sizeof(element_type),
        bits_per_element = (element_type)(CHAR_BIT*sizeof(element_type))
    };

  private:
    ls::utils::UniqueAlignedArray<element_type> mBits;
    size_type mNumBitsActive;
    size_type mNumBitsReserved;

  public:
    ~BitSet() noexcept = default;

    BitSet() noexcept;
    BitSet(size_type numBits, const element_type* bits = nullptr) noexcept;

    BitSet(const BitSet& bitSet) noexcept;
    BitSet(BitSet&& bitSet) noexcept;

    BitSet& operator=(const BitSet& bitSet) noexcept;
    BitSet& operator=(BitSet&& bitSet) noexcept;

    bool operator==(const BitSet& bitSet) const noexcept;
    bool operator!=(const BitSet& bitSet) const noexcept;

    void clear() noexcept;
    bool empty() const noexcept;

    size_type resize(size_type numBits, const element_type* bits = nullptr) noexcept;
    size_type reserve(size_type numBits) noexcept;

    size_type size() const noexcept; // returns number of bits used
    size_type elements() const noexcept; // returns the number of bytes, words, dwords, or qwords
    size_type capacity() const noexcept; // returns number of bits available

    size_type bit_width() const noexcept;
    size_type byte_width() const noexcept;
    const element_type* bits() const noexcept;

    element_type get(size_type bitIndex) const noexcept;
    element_type set(size_type bitIndex, element_type val) noexcept;

    element_type cbit_and(size_type bitIndex, element_type val) const noexcept;
    element_type cbit_or(size_type bitIndex, element_type val) const noexcept;
    element_type cbit_xor(size_type bitIndex, element_type val) const noexcept;
    element_type cbit_not(size_type bitIndex) const noexcept;

    element_type bit_and(size_type bitIndex, element_type val) noexcept;
    element_type bit_or(size_type bitIndex, element_type val) noexcept;
    element_type bit_xor(size_type bitIndex, element_type val) noexcept;
    element_type bit_not(size_type bitIndex) noexcept;

    element_type celement_and(size_type elementIndex, element_type val) const noexcept;
    element_type celement_or(size_type elementIndex, element_type val) const noexcept;
    element_type celement_xor(size_type elementIndex, element_type val) const noexcept;
    element_type celement_not(size_type elementIndex) const noexcept;

    element_type element_and(size_type elementIndex, element_type val) noexcept;
    element_type element_or(size_type elementIndex, element_type val) noexcept;
    element_type element_xor(size_type elementIndex, element_type val) noexcept;
    element_type element_not(size_type elementIndex) noexcept;

    BitSet& set_and(const BitSet& bitSet) noexcept;
    BitSet& set_or(const BitSet& bitSet) noexcept;
    BitSet& set_xor(const BitSet& bitSet) noexcept;
    BitSet& set_not() noexcept;
};



/*--------------------------------------
 * Default Constructor
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::BitSet() noexcept :
    mBits{nullptr},
    mNumBitsActive{0},
    mNumBitsReserved{0}
{
}

/*--------------------------------------
 * Check emptiness
--------------------------------------*/
template <typename ElementType>
inline bool BitSet<ElementType>::empty() const noexcept
{
    return mBits == nullptr;
}



/*--------------------------------------
 * Check size
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::size() const noexcept
{
    return mNumBitsActive;
}



/*--------------------------------------
 * Check size
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::elements() const noexcept
{
    return mNumBitsActive / bits_per_element;
}



/*--------------------------------------
 * Check capacity
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::capacity() const noexcept
{
    return mNumBitsReserved;
}



/*--------------------------------------
 * Bits per element
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::bit_width() const noexcept
{
    return bits_per_element;
}



/*--------------------------------------
 * Bytes per element
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::byte_width() const noexcept
{
    return bytes_per_element;
}



/*--------------------------------------
 * Data retrieval
--------------------------------------*/
template <typename ElementType>
inline const BitSet<ElementType>::element_type* BitSet<ElementType>::bits() const noexcept
{
    return mBits.get();
}



/*--------------------------------------
 * Bit retrieval
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::get(size_type bitIndex) const noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_element;
    const size_type bitOffset   = bitIndex % bits_per_element;

    const element_type bucket = mBits[bucketIndex];
    const element_type bit    = bucket & (element_type)1 << bitOffset;
    return bit != 0;
}



/*--------------------------------------
 * Bit assignment
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::set(size_type bitIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_element;
    const size_type bitOffset   = bitIndex % bits_per_element;

    const element_type bucket = mBits[bucketIndex];
    const element_type ones   = ~((element_type)1 << bitOffset);
    const element_type bit    = (element_type)(val != 0);
    const element_type result = (bucket & ones) | (bit << bitOffset);

    mBits[bucketIndex] = result;
    return static_cast<element_type>(bit);
}



/*--------------------------------------
 * AND a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::cbit_and(size_type bitIndex, element_type val) const noexcept
{
    return static_cast<element_type>(this->get(bitIndex) && val);
}



/*--------------------------------------
 * OR a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::cbit_or(size_type bitIndex, element_type val) const noexcept
{
    return static_cast<element_type>(this->get(bitIndex) || val);
}



/*--------------------------------------
 * XOR a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::cbit_xor(size_type bitIndex, element_type val) const noexcept
{
    return static_cast<element_type>(this->get(bitIndex) != (val != 0));
}



/*--------------------------------------
 * NOT a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::cbit_not(size_type bitIndex) const noexcept
{
    return static_cast<element_type>(!this->get(bitIndex));
}



/*--------------------------------------
 * AND a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::bit_and(size_type bitIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_element;
    const size_type bitOffset   = bitIndex % bits_per_element;

    const element_type bucket = mBits[bucketIndex];
    const element_type bit    = (element_type)(val != 0) << bitOffset;
    const element_type mask   = ~(element_type)0 & bit;
    const element_type result = bucket & mask;

    mBits[bucketIndex] = result;
    return static_cast<element_type>((result & bit) != 0);
}



/*--------------------------------------
 * OR a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::bit_or(size_type bitIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_element;
    const size_type bitOffset   = bitIndex % bits_per_element;

    const element_type bucket = mBits[bucketIndex];
    const element_type bit    = (element_type)(val != 0) << bitOffset;
    const element_type result = bucket | bit;

    mBits[bucketIndex] = result;
    return static_cast<element_type>((result & bit) != 0);
}



/*--------------------------------------
 * XOR a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::bit_xor(size_type bitIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_element;
    const size_type bitOffset   = bitIndex % bits_per_element;

    const element_type bucket = mBits[bucketIndex];
    const element_type bit    = (element_type)(val != 0) << bitOffset;
    const element_type result = bucket ^ bit;

    mBits[bucketIndex] = result;

    return static_cast<element_type>((result & bit) != 0);
}



/*--------------------------------------
 * NOT a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::element_type BitSet<ElementType>::bit_not(size_type bitIndex) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_element;
    const size_type bitOffset   = bitIndex % bits_per_element;

    const element_type bucket = mBits[bucketIndex];
    const element_type bit    = ((element_type)1 << bitOffset);
    const element_type result = bucket ^ bit;

    mBits[bucketIndex] = result;
    return static_cast<element_type>((result & bit) != 0);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::celement_and(size_type elementIndex, element_type val) const noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return mBits[elementIndex] & val;
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::celement_or(size_type elementIndex, element_type val) const noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return mBits[elementIndex] | val;
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::celement_xor(size_type elementIndex, element_type val) const noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return mBits[elementIndex] ^ val;
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::celement_not(size_type elementIndex) const noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return ~mBits[elementIndex];
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::element_and(size_type elementIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return (mBits[elementIndex] &= val);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::element_or(size_type elementIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return (mBits[elementIndex] |= val);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::element_xor(size_type elementIndex, element_type val) noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return (mBits[elementIndex] ^= val);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::element_type BitSet<ElementType>::element_not(size_type elementIndex) noexcept
{
    LS_DEBUG_ASSERT(elementIndex < elements());
    return (mBits[elementIndex] = ~mBits[elementIndex]);
}



} // end ls::utils namespace

LS_DECLARE_CLASS_TYPE(BitSet8, ls::utils::BitSet, uint8_t);
LS_DECLARE_CLASS_TYPE(BitSet16, ls::utils::BitSet, uint16_t);
LS_DECLARE_CLASS_TYPE(BitSet32, ls::utils::BitSet, uint32_t);
LS_DECLARE_CLASS_TYPE(BitSet64, ls::utils::BitSet, uint64_t);

#endif /* LS_UTILS_BITSET_HPP */
