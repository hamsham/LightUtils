
#include "lightsky/utils/WorkerThread.hpp"


namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Worker
-----------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(ls::utils::Worker, void (*)());



/*-----------------------------------------------------------------------------
 * WorkerThread
-----------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(ls::utils::WorkerThread, void (*)());



/*-----------------------------------------------------------------------------
 * WorkerThreadGroup
-----------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(ls::utils::WorkerThreadGroup, void (*)());



} // end utils namespace
} // end ls namespace
