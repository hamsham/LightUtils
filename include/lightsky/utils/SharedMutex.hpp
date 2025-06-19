/*
 * File:   SharedMutex.hpp
 * Author: hammy
 *
 * Created on Jan 05, 2023 at 11:45 PM
 */

#ifndef LS_UTILS_SHARED_MUTEX_HPP
#define LS_UTILS_SHARED_MUTEX_HPP

#include <atomic>
#include <cstdint>
#include <mutex>
#include <condition_variable>

#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Futex.hpp"
#include "lightsky/utils/SpinLock.hpp"



namespace ls
{
namespace utils
{



template <typename mutex_type = ls::utils::Futex>
class SharedMutexType
{
  public:
    typedef mutex_type native_handle_type;

  private:
    enum : unsigned long long
    {
        LOCK_WRITE_BIT = 0x8000000000000000ull
    };

    std::atomic_ullong mShareCount;
    native_handle_type mLock;

  public:
    ~SharedMutexType() noexcept = default;

    SharedMutexType() noexcept;

    SharedMutexType(const SharedMutexType&) noexcept = delete;

    SharedMutexType(SharedMutexType&&) noexcept = delete;

    SharedMutexType& operator=(const SharedMutexType&) noexcept = delete;

    SharedMutexType& operator=(SharedMutexType&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};



LS_DECLARE_CLASS_TYPE(SharedMutex, SharedMutexType, std::mutex);
LS_DECLARE_CLASS_TYPE(SharedFutex, SharedMutexType, ls::utils::Futex);
LS_DECLARE_CLASS_TYPE(SharedSpinLock, SharedMutexType, ls::utils::SpinLock);



class alignas(alignof(uint64_t)) SRWLock
{
public:
    typedef uint64_t native_handle_type;

private:
    enum LockFlags : uint16_t
    {
        LOCK_WRITE_BIT = 0x0001,
        LOCK_TRY_WRITE_BIT = 0x0003
    };

    struct LockFields
    {
        std::atomic_uint8_t lockType;
        std::atomic_uint8_t typePadding;
        std::atomic_uint16_t shareCount;
        std::atomic_uint16_t nextLockId;
        std::atomic_uint16_t currentLockId;
    };

    union
    {
        uint64_t mLockBits;
        LockFields mLockFields;
    };

    static void yield() noexcept;

public:
    ~SRWLock() noexcept = default;

    SRWLock() noexcept;

    SRWLock(const SRWLock&) noexcept = delete;

    SRWLock(SRWLock&&) noexcept = delete;

    SRWLock& operator=(const SRWLock&) noexcept = delete;

    SRWLock& operator=(SRWLock&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SharedMutexImpl.hpp"

#endif /* LS_UTILS_SHARED_MUTEX_HPP */
