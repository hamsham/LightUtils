
#include "lightsky/utils/pointer.h"

namespace ls {
namespace utils {

/*-----------------------------------------------------------------------------
 * Dynamic Pointer Types
 * --------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(pointer, bool);
LS_DEFINE_CLASS_TYPE(pointer, signed char);
LS_DEFINE_CLASS_TYPE(pointer, unsigned char);
LS_DEFINE_CLASS_TYPE(pointer, wchar_t);
LS_DEFINE_CLASS_TYPE(pointer, char16_t);
LS_DEFINE_CLASS_TYPE(pointer, char32_t);
LS_DEFINE_CLASS_TYPE(pointer, signed short);
LS_DEFINE_CLASS_TYPE(pointer, unsigned short);
LS_DEFINE_CLASS_TYPE(pointer, signed int);
LS_DEFINE_CLASS_TYPE(pointer, unsigned int);
LS_DEFINE_CLASS_TYPE(pointer, signed long);
LS_DEFINE_CLASS_TYPE(pointer, unsigned long);
LS_DEFINE_CLASS_TYPE(pointer, signed long long);
LS_DEFINE_CLASS_TYPE(pointer, unsigned long long);
LS_DEFINE_CLASS_TYPE(pointer, float);
LS_DEFINE_CLASS_TYPE(pointer, double);
LS_DEFINE_CLASS_TYPE(pointer, long double);

/*-----------------------------------------------------------------------------
 * Dynamic Array Types
 * --------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(pointer, bool[]);
LS_DEFINE_CLASS_TYPE(pointer, signed char[]);
LS_DEFINE_CLASS_TYPE(pointer, unsigned char[]);
LS_DEFINE_CLASS_TYPE(pointer, wchar_t[]);
LS_DEFINE_CLASS_TYPE(pointer, char16_t[]);
LS_DEFINE_CLASS_TYPE(pointer, char32_t[]);
LS_DEFINE_CLASS_TYPE(pointer, signed short[]);
LS_DEFINE_CLASS_TYPE(pointer, unsigned short[]);
LS_DEFINE_CLASS_TYPE(pointer, signed int[]);
LS_DEFINE_CLASS_TYPE(pointer, unsigned int[]);
LS_DEFINE_CLASS_TYPE(pointer, signed long[]);
LS_DEFINE_CLASS_TYPE(pointer, unsigned long[]);
LS_DEFINE_CLASS_TYPE(pointer, signed long long[]);
LS_DEFINE_CLASS_TYPE(pointer, unsigned long long[]);
LS_DEFINE_CLASS_TYPE(pointer, float[]);
LS_DEFINE_CLASS_TYPE(pointer, double[]);
LS_DEFINE_CLASS_TYPE(pointer, long double[]);

} // end utils namespace
} // end ls namespace
