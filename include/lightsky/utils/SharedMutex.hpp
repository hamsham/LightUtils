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



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/SharedMutexImpl.hpp"

#endif /* LS_UTILS_SHARED_MUTEX_HPP */
