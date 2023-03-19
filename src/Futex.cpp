
#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Futex.hpp"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Futex
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
Futex::~Futex() noexcept
{
    (void)mPad;

#if defined(LS_UTILS_HAVE_LINUX_FUTEX)
    __atomic_store_n(&mLock, 0, __ATOMIC_RELEASE);

#elif defined(LS_UTILS_HAVE_WIN32_FUTEX)
#else
    mLock.store(0, std::memory_order_release);
#endif
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Futex::Futex(FutexPauseCount maxPauses) noexcept :
#if !defined(LS_UTILS_HAVE_WIN32_FUTEX)
    mLock{0},
#endif
    mMaxPauseCount{LS_ENUM_VAL(maxPauses) > LS_ENUM_VAL(FutexPauseCount::FUTEX_PAUSE_COUNT_MAX) ? FutexPauseCount::FUTEX_PAUSE_COUNT_MAX : maxPauses},
    mPad{0}
{
    #if defined(LS_UTILS_HAVE_WIN32_FUTEX)
        InitializeSRWLock(&mLock);
    #endif
}



} // end utils namespace
} // end ls namespace
