
#include "lightsky/utils/Tuple.h"

namespace ls {
namespace utils {

/*-------------------------------------
 * Extern Templates for plain data types.
 * ----------------------------------*/
template class Tuple_t<signed char>;
template class Tuple_t<signed char*>;
template class Tuple_t<signed short>;
template class Tuple_t<signed short*>;
template class Tuple_t<signed int>;
template class Tuple_t<signed int*>;
template class Tuple_t<signed long>;
template class Tuple_t<signed long*>;
template class Tuple_t<signed long long>;
template class Tuple_t<signed long long*>;

template class Tuple_t<unsigned char>;
template class Tuple_t<unsigned char*>;
template class Tuple_t<unsigned short>;
template class Tuple_t<unsigned short*>;
template class Tuple_t<unsigned int>;
template class Tuple_t<unsigned int*>;
template class Tuple_t<unsigned long>;
template class Tuple_t<unsigned long*>;
template class Tuple_t<unsigned long long>;
template class Tuple_t<unsigned long long*>;

template class Tuple_t<float>;
template class Tuple_t<float*>;
template class Tuple_t<double>;
template class Tuple_t<double*>;
template class Tuple_t<long double>;
template class Tuple_t<long double*>;

// data and pointer pairs
template class Tuple_t<signed char, signed char*>;
template class Tuple_t<signed short, signed short*>;
template class Tuple_t<signed int, signed int*>;
template class Tuple_t<signed long, signed long*>;
template class Tuple_t<signed long long, signed long long*>;

template class Tuple_t<unsigned char, unsigned char*>;
template class Tuple_t<unsigned short, unsigned short*>;
template class Tuple_t<unsigned int, unsigned int*>;
template class Tuple_t<unsigned long, unsigned long*>;
template class Tuple_t<unsigned long long, unsigned long long*>;

template class Tuple_t<float, float*>;
template class Tuple_t<double, double*>;
template class Tuple_t<long double, long double*>;

} // end utils namespace
} // end ls namespace