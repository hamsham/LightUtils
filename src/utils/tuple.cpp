
#include "lightsky/utils/tuple.h"

namespace ls {
namespace utils {

/*-------------------------------------
 * Extern Templates for plain data types.
 * ----------------------------------*/
template class tuple_t<signed char>;
template class tuple_t<signed char*>;
template class tuple_t<signed short>;
template class tuple_t<signed short*>;
template class tuple_t<signed int>;
template class tuple_t<signed int*>;
template class tuple_t<signed long>;
template class tuple_t<signed long*>;
template class tuple_t<signed long long>;
template class tuple_t<signed long long*>;

template class tuple_t<unsigned char>;
template class tuple_t<unsigned char*>;
template class tuple_t<unsigned short>;
template class tuple_t<unsigned short*>;
template class tuple_t<unsigned int>;
template class tuple_t<unsigned int*>;
template class tuple_t<unsigned long>;
template class tuple_t<unsigned long*>;
template class tuple_t<unsigned long long>;
template class tuple_t<unsigned long long*>;

template class tuple_t<float>;
template class tuple_t<float*>;
template class tuple_t<double>;
template class tuple_t<double*>;
template class tuple_t<long double>;
template class tuple_t<long double*>;

// data and pointer pairs
template class tuple_t<signed char,      signed char*>;
template class tuple_t<signed short,     signed short*>;
template class tuple_t<signed int,       signed int*>;
template class tuple_t<signed long,      signed long*>;
template class tuple_t<signed long long, signed long long*>;

template class tuple_t<unsigned char,        unsigned char*>;
template class tuple_t<unsigned short,       unsigned short*>;
template class tuple_t<unsigned int,         unsigned int*>;
template class tuple_t<unsigned long,        unsigned long*>;
template class tuple_t<unsigned long long,   unsigned long long*>;

template class tuple_t<float,        float*>;
template class tuple_t<double,       double*>;
template class tuple_t<long double,  long double*>;

} // end utils namespace
} // end ls namespace
