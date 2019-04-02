
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
    mLock.clear(std::memory_order_release);
}


/*-------------------------------------
 * Constructor
-------------------------------------*/
SpinLock::SpinLock() noexcept :
    mLock{false}
{}



} // end utils namespace
} // end ls namespace
