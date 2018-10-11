
#include <utility> // std::move

#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Time.hpp"

namespace ls
{
namespace utils
{



// Let the compiler catch any errors in the class definitions
LS_DEFINE_CLASS_TYPE(Clock, unsigned,           std::ratio<1, 1000>);
LS_DEFINE_CLASS_TYPE(Clock, unsigned long,      std::ratio<1, 1000>);
LS_DEFINE_CLASS_TYPE(Clock, unsigned long long, std::ratio<1, 1000>);
LS_DEFINE_CLASS_TYPE(Clock, float,              std::ratio<1, 1000>);
LS_DEFINE_CLASS_TYPE(Clock, double,             std::ratio<1, 1000>);
LS_DEFINE_CLASS_TYPE(Clock, long double,        std::ratio<1, 1000>);



} // end utils namespace
} // end ls namespace
