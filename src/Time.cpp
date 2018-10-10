
#include <utility> // std::move

#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Time.hpp"

namespace ls
{
namespace utils
{



// Let the compiler catch any errors in the class definitions
LS_DEFINE_CLASS_TYPE(Clock, unsigned);
LS_DEFINE_CLASS_TYPE(Clock, unsigned long);
LS_DEFINE_CLASS_TYPE(Clock, unsigned long long);
LS_DEFINE_CLASS_TYPE(Clock, float);
LS_DEFINE_CLASS_TYPE(Clock, double);
LS_DEFINE_CLASS_TYPE(Clock, long double);



} // end utils namespace
} // end ls namespace
