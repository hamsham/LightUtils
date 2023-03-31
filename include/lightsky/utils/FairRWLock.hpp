/*
 * File:   FairRWLockType.hpp
 * Author: hammy
 *
 * Created on Mar 30, 2023 at 9:40 PM
 */

#ifndef LS_UTILS_FAIR_RW_LOCK_HPP
#define LS_UTILS_FAIR_RW_LOCK_HPP

#include <mutex>

#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Futex.hpp"
#include "lightsky/utils/SpinLock.hpp"

namespace ls
{
namespace utils
{



template <typename MutexType = ls::utils::Futex>
class FairRWLockType
{
  public:
    typedef MutexType native_handle_type;

    struct alignas(64) RWLockNode
    {
        alignas(64) native_handle_type mLock;
        alignas(64) std::atomic<native_handle_type*> mNextLock;
        std::atomic<RWLockNode*> pNext;
        std::atomic<RWLockNode*> pPrev;
    };

    static_assert(alignof(RWLockNode) == 64, "Misaligned locking node.");

  private:
    RWLockNode mHead;
    RWLockNode mTail;
    std::atomic_llong mNumUsers;

    void _insert_node(RWLockNode& lock) noexcept;

    bool _try_insert_node(RWLockNode& lock) noexcept;

    void _pop_node_impl(RWLockNode& lock, bool lockedNextNode) noexcept;

    bool _wait_impl(RWLockNode& lock) noexcept;

  public:
    ~FairRWLockType() noexcept = default;

    FairRWLockType() noexcept;

    FairRWLockType(const FairRWLockType&) noexcept = delete;

    FairRWLockType(FairRWLockType&&) noexcept = delete;

    FairRWLockType& operator=(const FairRWLockType&) noexcept = delete;

    FairRWLockType& operator=(FairRWLockType&&) noexcept = delete;

    void lock_shared() noexcept;

    void lock() noexcept;

    bool try_lock_shared() noexcept;

    bool try_lock() noexcept;

    void unlock_shared() noexcept;

    void unlock() noexcept;

    const native_handle_type& native_handle() const noexcept;
};



LS_DECLARE_CLASS_TYPE(FairRWMutex, FairRWLockType, std::mutex);
LS_DECLARE_CLASS_TYPE(FairRWFutex, FairRWLockType, ls::utils::Futex);
LS_DECLARE_CLASS_TYPE(FairRWSpinLock, FairRWLockType, ls::utils::SpinLock);



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/FairRWLockImpl.hpp"

#endif /* LS_UTILS_FAIR_RW_LOCK_HPP */
