/*
 * File:   FairRWLock.cpp
 * Author: hammy
 *
 * Created on Mar 30, 2023 at 9:45 PM
 */

#include "lightsky/utils/FairRWLock.hpp"

namespace ls
{
namespace utils
{



LS_DEFINE_CLASS_TYPE(FairRWLockType, std::mutex);
LS_DEFINE_CLASS_TYPE(FairRWLockType, ls::utils::Futex);
LS_DEFINE_CLASS_TYPE(FairRWLockType, ls::utils::SpinLock);



} // end utils namespace
} // end ls namespace
