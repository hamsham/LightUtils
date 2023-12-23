/*
 * File:   RingBuffer.hpp
 * Author: hammy
 *
 * Created on Dec 22, 2023 at 12:04 AM
 */

#ifndef LS_UTILS_RING_BUFFER_HPP
#define LS_UTILS_RING_BUFFER_HPP

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
public:
    typedef T value_type;
    typedef unsigned long long size_type;
    typedef T& reference;
    typedef const T& const_reference;

private:
    unsigned long long mHead;
    unsigned long long mTail;
    unsigned long long mCapacity;
    unsigned long long mPad;
    ls::utils::Pointer<T[]> mData;

private:
    unsigned long long _realloc_size() const noexcept;

    bool _resize_if_needed() noexcept;

public:
    ~RingBuffer() noexcept = default;

    constexpr RingBuffer() noexcept;

    RingBuffer(size_type requestedCapacity) noexcept;

    RingBuffer(const RingBuffer&) noexcept;

    RingBuffer(RingBuffer&&) noexcept;

    RingBuffer& operator=(const RingBuffer& buffer) noexcept;

    RingBuffer& operator=(RingBuffer&& buffer) noexcept;

    bool reserve(size_type requestedCapacity) noexcept;

    bool empty() const noexcept;

    bool full() const noexcept;

    size_type size() const noexcept;

    size_type capacity() const noexcept;

    void clear() noexcept;

    void shrink_to_fit() noexcept;

    void push_unchecked(const_reference val) noexcept;

    void push_unchecked(T&& val) noexcept;

    void emplace_unchecked() noexcept;

    template <typename... ArgsType>
    void emplace_unchecked(ArgsType&&... args) noexcept;

    value_type pop_unchecked() noexcept;

    bool push(const_reference val) noexcept;

    bool push(T&& val) noexcept;

    bool emplace() noexcept;

    template <typename... ArgsType>
    bool emplace(ArgsType&&... args) noexcept;

    bool pop(reference& result) noexcept;

    const_reference front() const noexcept;

    const_reference back() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/RingBufferImpl.hpp"

#endif /* LS_UTILS_RING_BUFFER_HPP */
