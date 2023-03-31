
#ifndef LS_UTILS_FUTEX_IMPL_HPP
#define LS_UTILS_FUTEX_IMPL_HPP

#include <limits>

#include "lightsky/setup/CPU.h"



namespace ls
{
namespace utils
{



/*-------------------------------------
 * Set the maximum consecutive pauses before retrying a lock
-------------------------------------*/
inline void Futex::pause_count(FutexPauseCount maxPauses) noexcept
{
    mMaxPauseCount = maxPauses;
}



/*-------------------------------------
 * Get the maximum consecutive pauses
-------------------------------------*/
inline FutexPauseCount Futex::pause_count() const noexcept
{
    return mMaxPauseCount;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FUTEX_IMPL_HPP */