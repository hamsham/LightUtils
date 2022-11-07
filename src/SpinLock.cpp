
#include "lightsky/utils/SpinLock.hpp"



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * SpinLock
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SpinLock::~SpinLock() noexcept
{
    (void)mPadding;
    mLock.clear(std::memory_order_release);
}



} // end utils namespace
} // end ls namespace
