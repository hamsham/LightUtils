
#include "lightsky/utils/SharedMutex.hpp"

namespace ls
{
namespace utils
{



LS_DEFINE_CLASS_TYPE(SharedMutexType, std::mutex);
LS_DEFINE_CLASS_TYPE(SharedMutexType, ls::utils::Futex);



} // end utils namespace
} // end ls namespace
