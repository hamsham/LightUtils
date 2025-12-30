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
    typedef ElementType value_type;

    enum : size_type
    {
        bytes_per_bucket = sizeof(value_type),
        bits_per_bucket = (value_type)(CHAR_BIT*sizeof(value_type))
    };

  private:
    ls::utils::UniqueAlignedArray<value_type> mBits;
    size_type mNumBitsActive;
    size_type mNumBitsReserved;

  public:
    ~BitSet() noexcept = default;

    BitSet() noexcept;
    BitSet(size_type numBits, const value_type* bits = nullptr) noexcept;

    BitSet(const BitSet& bitSet) noexcept;
    BitSet(BitSet&& bitSet) noexcept;

    BitSet& operator=(const BitSet& bitSet) noexcept;
    BitSet& operator=(BitSet&& bitSet) noexcept;

    bool operator==(const BitSet& bitSet) const noexcept;
    bool operator!=(const BitSet& bitSet) const noexcept;

    void clear() noexcept;
    bool empty() const noexcept;

    size_type resize(size_type numBits, const value_type* bits = nullptr) noexcept;
    size_type reserve(size_type numBits) noexcept;

    size_type size() const noexcept; // returns number of bits used
    size_type capacity() const noexcept; // returns number of bits available
    size_type bucket_size() const noexcept;
    size_type bucket_count() const noexcept; // returns the number of bytes, words, dwords, or qwords
    size_type max_bucket_count() const noexcept; // returns the number of reserved bytes, words, dwords, or qwords

    const value_type* data() const noexcept;
    value_type bucket(size_type bucketIndex) const noexcept;
    value_type& bucket(size_type bucketIndex) noexcept;

    value_type get(size_type bitIndex) const noexcept;
    value_type set(size_type bitIndex, value_type val) noexcept;

    value_type cbit_and(size_type bitIndex, value_type val) const noexcept;
    value_type cbit_or(size_type bitIndex, value_type val) const noexcept;
    value_type cbit_xor(size_type bitIndex, value_type val) const noexcept;
    value_type cbit_not(size_type bitIndex) const noexcept;

    value_type bit_and(size_type bitIndex, value_type val) noexcept;
    value_type bit_or(size_type bitIndex, value_type val) noexcept;
    value_type bit_xor(size_type bitIndex, value_type val) noexcept;
    value_type bit_not(size_type bitIndex) noexcept;

    value_type cbucket_and(size_type bucketIndex, value_type val) const noexcept;
    value_type cbucket_or(size_type bucketIndex, value_type val) const noexcept;
    value_type cbucket_xor(size_type bucketIndex, value_type val) const noexcept;
    value_type cbucket_not(size_type bucketIndex) const noexcept;

    value_type bucket_and(size_type bucketIndex, value_type val) noexcept;
    value_type bucket_or(size_type bucketIndex, value_type val) noexcept;
    value_type bucket_xor(size_type bucketIndex, value_type val) noexcept;
    value_type bucket_not(size_type bucketIndex) noexcept;

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
 * Check capacity
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::capacity() const noexcept
{
    return mNumBitsReserved;
}



/*--------------------------------------
 * Bits per bucket
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::bucket_size() const noexcept
{
    return bits_per_bucket;
}



/*--------------------------------------
 * Check bucket size
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::bucket_count() const noexcept
{
    return mNumBitsActive / bits_per_bucket;
}



/*--------------------------------------
 * Check bucket capacity
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::size_type BitSet<ElementType>::max_bucket_count() const noexcept
{
    return mNumBitsReserved / bits_per_bucket;
}



/*--------------------------------------
 * Data retrieval
--------------------------------------*/
template <typename ElementType>
inline const BitSet<ElementType>::value_type* BitSet<ElementType>::data() const noexcept
{
    return mBits.get();
}



/*--------------------------------------
 * Bucket retrieval (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::bucket(size_type bucketIndex) const noexcept
{
    return mBits[bucketIndex];
}



/*--------------------------------------
 * Bucket retrieval
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type& BitSet<ElementType>::bucket(size_type bucketIndex) noexcept
{
    return mBits[bucketIndex];
}



/*--------------------------------------
 * Bit retrieval
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::get(size_type bitIndex) const noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_bucket;
    const size_type bitOffset   = bitIndex & (bits_per_bucket-1);

    const value_type bucket = mBits[bucketIndex];
    const value_type bit    = bucket & (value_type)1 << bitOffset;
    return bit != 0;
}



/*--------------------------------------
 * Bit assignment
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::set(size_type bitIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_bucket;
    const size_type bitOffset   = bitIndex & (bits_per_bucket-1);

    const value_type bucket = mBits[bucketIndex];
    const value_type ones   = ~((value_type)1 << bitOffset);
    const value_type bit    = (value_type)(val != 0);
    const value_type result = (bucket & ones) | (bit << bitOffset);

    mBits[bucketIndex] = result;
    return static_cast<value_type>(bit);
}



/*--------------------------------------
 * AND a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::cbit_and(size_type bitIndex, value_type val) const noexcept
{
    return static_cast<value_type>(this->get(bitIndex) && val);
}



/*--------------------------------------
 * OR a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::cbit_or(size_type bitIndex, value_type val) const noexcept
{
    return static_cast<value_type>(this->get(bitIndex) || val);
}



/*--------------------------------------
 * XOR a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::cbit_xor(size_type bitIndex, value_type val) const noexcept
{
    return static_cast<value_type>(this->get(bitIndex) != (val != 0));
}



/*--------------------------------------
 * NOT a single bit (const)
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::cbit_not(size_type bitIndex) const noexcept
{
    return static_cast<value_type>(!this->get(bitIndex));
}



/*--------------------------------------
 * AND a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::bit_and(size_type bitIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_bucket;
    const size_type bitOffset   = bitIndex & (bits_per_bucket-1);

    const value_type bucket = mBits[bucketIndex];
    const value_type bit    = (value_type)(val != 0) << bitOffset;
    const value_type mask   = ~(value_type)0 & bit;
    const value_type result = bucket & mask;

    mBits[bucketIndex] = result;
    return static_cast<value_type>((result & bit) != 0);
}



/*--------------------------------------
 * OR a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::bit_or(size_type bitIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_bucket;
    const size_type bitOffset   = bitIndex & (bits_per_bucket-1);

    const value_type bucket = mBits[bucketIndex];
    const value_type bit    = (value_type)(val != 0) << bitOffset;
    const value_type result = bucket | bit;

    mBits[bucketIndex] = result;
    return static_cast<value_type>((result & bit) != 0);
}



/*--------------------------------------
 * XOR a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::bit_xor(size_type bitIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_bucket;
    const size_type bitOffset   = bitIndex & (bits_per_bucket-1);

    const value_type bucket = mBits[bucketIndex];
    const value_type bit    = (value_type)(val != 0) << bitOffset;
    const value_type result = bucket ^ bit;

    mBits[bucketIndex] = result;

    return static_cast<value_type>((result & bit) != 0);
}



/*--------------------------------------
 * NOT a single bit
--------------------------------------*/
template <typename ElementType>
inline BitSet<ElementType>::value_type BitSet<ElementType>::bit_not(size_type bitIndex) noexcept
{
    LS_DEBUG_ASSERT(bitIndex < mNumBitsActive);
    const size_type bucketIndex = bitIndex / bits_per_bucket;
    const size_type bitOffset   = bitIndex & (bits_per_bucket-1);

    const value_type bucket = mBits[bucketIndex];
    const value_type bit    = ((value_type)1 << bitOffset);
    const value_type result = bucket ^ bit;

    mBits[bucketIndex] = result;
    return static_cast<value_type>((result & bit) != 0);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::cbucket_and(size_type bucketIndex, value_type val) const noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return mBits[bucketIndex] & val;
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::cbucket_or(size_type bucketIndex, value_type val) const noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return mBits[bucketIndex] | val;
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::cbucket_xor(size_type bucketIndex, value_type val) const noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return mBits[bucketIndex] ^ val;
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::cbucket_not(size_type bucketIndex) const noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return ~mBits[bucketIndex];
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::bucket_and(size_type bucketIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return (mBits[bucketIndex] &= val);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::bucket_or(size_type bucketIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return (mBits[bucketIndex] |= val);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::bucket_xor(size_type bucketIndex, value_type val) noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return (mBits[bucketIndex] ^= val);
}



/*--------------------------------------
--------------------------------------*/
template <typename ElementType>
typename BitSet<ElementType>::value_type BitSet<ElementType>::bucket_not(size_type bucketIndex) noexcept
{
    LS_DEBUG_ASSERT(bucketIndex < bucket_count());
    return (mBits[bucketIndex] = ~mBits[bucketIndex]);
}



} // end ls::utils namespace

LS_DECLARE_CLASS_TYPE(BitSet8, ls::utils::BitSet, uint8_t);
LS_DECLARE_CLASS_TYPE(BitSet16, ls::utils::BitSet, uint16_t);
LS_DECLARE_CLASS_TYPE(BitSet32, ls::utils::BitSet, uint32_t);
LS_DECLARE_CLASS_TYPE(BitSet64, ls::utils::BitSet, uint64_t);

#endif /* LS_UTILS_BITSET_HPP */
