/*
 * File:   RingBufferImpl.hpp
 * Author: hammy
 *
 * Created on Dec 22, 2023 at 12:03 AM
 */

#ifndef LS_UTILS__RING_BUFFER_IMPL_HPP
#define LS_UTILS__RING_BUFFER_IMPL_HPP

#include "lightsky/utils/Assertions.h"

namespace ls
{
namespace utils
{



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
RingBuffer<T>::~RingBuffer() noexcept
{
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
constexpr RingBuffer<T>::RingBuffer() noexcept :
    mHead{0},
    mTail{0},
    mCapacity{1},
    mPad{0},
    mData{nullptr}
{
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
RingBuffer<T>::RingBuffer(unsigned long long requestedCapacity) noexcept :
    RingBuffer{}
{
    reserve(requestedCapacity);
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
RingBuffer<T>::RingBuffer(const RingBuffer& buffer) noexcept :
    RingBuffer{}
{
    if (buffer.empty())
    {
        return;
    }

    mData = ls::utils::make_unique_array<T>(buffer.mCapacity);
    if (mData.get() != nullptr)
    {
        mHead = buffer.mHead;
        mTail = buffer.mTail;
        mCapacity = buffer.mCapacity;

        for (unsigned long long i = buffer.mHead; i != buffer.mTail; i = (i + 1ull) % buffer.mCapacity)
        {
            mData[i] = buffer.mData[i];
        }
    }
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
RingBuffer<T>::RingBuffer(RingBuffer&& buffer) noexcept :
    mHead{buffer.mHead},
    mTail{buffer.mTail},
    mCapacity{buffer.mCapacity},
    mPad{0},
    mData{std::move(buffer.mData)}
{
    buffer.mHead = 0;
    buffer.mTail = 0;
    buffer.mCapacity = 1;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
RingBuffer<T>& RingBuffer<T>::operator=(const RingBuffer& buffer) noexcept
{
    if (this == &buffer)
    {
        return *this;
    }

    clear();

    if (buffer.empty())
    {
        return *this;
    }

    mData = ls::utils::make_unique_array<T>(buffer.mCapacity);
    if (mData.get() != nullptr)
    {
        mHead = buffer.mHead;
        mTail = buffer.mTail;
        mCapacity = buffer.mCapacity;

        for (unsigned long long i = buffer.mHead; i != buffer.mTail; i = (i + 1ull) % buffer.mCapacity)
        {
            mData[i] = buffer.mData[i];
        }
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
RingBuffer<T>& RingBuffer<T>::operator=(RingBuffer&& buffer) noexcept
{
    if (this != &buffer)
    {
        mHead = buffer.mHead;
        mTail = buffer.mTail;
        mCapacity = buffer.mCapacity;
        mData = std::move(buffer.mData);

        buffer.mHead = 0;
        buffer.mTail = 0;
        buffer.mCapacity = 1;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
bool RingBuffer<T>::reserve(unsigned long long requestedCapacity) noexcept
{
    if (!requestedCapacity)
    {
        clear();
        return true;
    }

    const unsigned long long currentCapacity = capacity();
    const unsigned long long currentSize = size();
    if (requestedCapacity == currentCapacity)
    {
        return true;
    }
    else if (requestedCapacity < currentSize)
    {
        requestedCapacity = currentSize;
    }

    // Use an extra element to help detect if the buffer is full
    const unsigned long long newCapacity = requestedCapacity + 1ull;

    ls::utils::Pointer<T[]>&& tmp = ls::utils::make_unique_array<T>(newCapacity);
    if (!tmp)
    {
        return false;
    }

    unsigned long long oldBegin = mHead;
    unsigned long long oldEnd = mTail;
    unsigned long long iter = oldBegin;
    unsigned long long newEnd = 0ull;

    for (; iter != oldEnd; iter = ((iter + 1ull) % mCapacity), ++newEnd)
    {
        tmp[newEnd] = mData[iter];
    }

    mHead = 0ull;
    mTail = newEnd;
    mCapacity = newCapacity;
    mData = std::move(tmp);

    return true;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline bool RingBuffer<T>::empty() const noexcept
{
    return mHead == mTail;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline bool RingBuffer<T>::full() const noexcept
{
    return mHead == ((mTail + 1ull) % mCapacity);
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline unsigned long long RingBuffer<T>::size() const noexcept
{
    return (mTail - mHead) % mCapacity;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline unsigned long long RingBuffer<T>::capacity() const noexcept
{
    return mCapacity - 1ull;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline void RingBuffer<T>::clear() noexcept
{
    mHead = 0ull;
    mTail = 0ull;
    mCapacity = 1ull;
    mData.reset();
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline void RingBuffer<T>::shrink_to_fit() noexcept
{
    const unsigned long long numElements = size();
    reserve(numElements);
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline void RingBuffer<T>::push(const T& val) noexcept
{
    LS_DEBUG_ASSERT(!full());

    mData[mTail] = val;
    mTail = (mTail + 1ull) % mCapacity;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline void RingBuffer<T>::push(T&& val) noexcept
{
    LS_DEBUG_ASSERT(!full());

    mData[mTail] = std::move(val);
    mTail = (mTail + 1ull) % mCapacity;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline void RingBuffer<T>::emplace() noexcept
{
    LS_DEBUG_ASSERT(!full());

    mData[mTail] = T{};
    mTail = (mTail + 1ull) % mCapacity;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
template <typename... ArgsType>
inline void RingBuffer<T>::emplace(ArgsType&&... args) noexcept
{
    LS_DEBUG_ASSERT(!full());

    mData[mTail] = T{std::forward<ArgsType>(args)...};
    mTail = (mTail + 1ull) % mCapacity;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline T RingBuffer<T>::pop() noexcept
{
    LS_DEBUG_ASSERT(!empty());

    T&& result = std::move(mData[mHead]);
    mHead = (mHead + 1ull) % mCapacity;

    return result;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline const T& RingBuffer<T>::front() const noexcept
{
    LS_DEBUG_ASSERT(!empty());
    return mData[mHead];
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename T>
inline const T& RingBuffer<T>::back() const noexcept
{
    LS_DEBUG_ASSERT(!empty());
    const unsigned long long index = mTail != 0ull ? mTail : mCapacity;
    return mData[index-1ull];
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS__RING_BUFFER_IMPL_HPP */
