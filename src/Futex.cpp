
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
}


/*-------------------------------------
 * Constructor
-------------------------------------*/
Futex::Futex(uint_fast64_t maxPauses) noexcept :
    mLock{},
    mMaxPauseCount{maxPauses}
{}




} // end utils namespace
} // end ls namespace
