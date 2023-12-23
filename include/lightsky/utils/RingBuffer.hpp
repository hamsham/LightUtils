/*
 * File:   RingBuffer.hpp
 * Author: hammy
 *
 * Created on Dec 22, 2023 at 12:04 AM
 */

#ifndef LS_UTILS__RING_BUFFER_HPP
#define LS_UTILS__RING_BUFFER_HPP

#include "lightsky/utils/Pointer.h"

namespace ls
{
namespace utils
{



/**
 * Generic Bing Buffer Type
 */
template <typename T>
class RingBuffer
{
private:
    unsigned long long mHead;
    unsigned long long mTail;
    unsigned long long mCapacity;
    unsigned long long mPad;
    ls::utils::Pointer<T[]> mData;

public:
    typedef T value_type;
    typedef unsigned long long size_type;
    typedef T& reference;
    typedef const T& const_reference;

public:
    ~RingBuffer() noexcept;

    constexpr RingBuffer() noexcept;

    RingBuffer(unsigned long long requestedCapacity) noexcept;

    RingBuffer(const RingBuffer&) noexcept;

    RingBuffer(RingBuffer&&) noexcept;

    RingBuffer& operator=(const RingBuffer& buffer) noexcept;

    RingBuffer& operator=(RingBuffer&& buffer) noexcept;

    bool reserve(unsigned long long requestedCapacity) noexcept;

    bool empty() const noexcept;

    bool full() const noexcept;

    unsigned long long size() const noexcept;

    unsigned long long capacity() const noexcept;

    void clear() noexcept;

    void shrink_to_fit() noexcept;

    void push(const T& val) noexcept;

    void push(T&& val) noexcept;

    void emplace() noexcept;

    template <typename... ArgsType>
    void emplace(ArgsType&&... args) noexcept;

    T pop() noexcept;

    const T& front() const noexcept;

    const T& back() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/RingBufferImpl.hpp"

#endif /* LS_UTILS__RING_BUFFER_HPP */
