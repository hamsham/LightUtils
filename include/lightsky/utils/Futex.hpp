
#ifndef LS_UTILS_FUTEX_HPP
#define LS_UTILS_FUTEX_HPP

#include <atomic>

#include "lightsky/setup/Arch.h" // LS_ARCH_X86



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief A Futex attempts to mimic a mutex object but will attempt to run in
 * user-space rather than switch to kernel-space unless necessary.
-----------------------------------------------------------------------------*/
class Futex
{
  private:
    std::atomic_int_fast32_t mLock;

  public:
    ~Futex() noexcept;

    Futex() noexcept;

    Futex(const Futex&) = delete;

    Futex(Futex&&) = delete;

    Futex& operator=(const Futex&) = delete;

    Futex& operator=(Futex&&) = delete;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};



} // end utils namespace
} // end ls namespace



#if defined(LS_ARCH_X86)
    #include "lightsky/utils/x86/FutexImpl.hpp"
#else
    #include "lightsky/utils/generic/FutexImpl.hpp"
#endif

#endif /* LS_UTILS_FUTEX_HPP */
