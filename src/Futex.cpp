
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
Futex::Futex() noexcept :
    mLock{0}
{}




} // end utils namespace
} // end ls namespace
