
#include "lightsky/utils/Pointer.h"

namespace ls {
namespace utils {

/*-----------------------------------------------------------------------------
 * Dynamic Pointer Types
 * --------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(Pointer, bool);
LS_DEFINE_CLASS_TYPE(Pointer, signed char);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned char);
LS_DEFINE_CLASS_TYPE(Pointer, wchar_t);
LS_DEFINE_CLASS_TYPE(Pointer, char16_t);
LS_DEFINE_CLASS_TYPE(Pointer, char32_t);
LS_DEFINE_CLASS_TYPE(Pointer, signed short);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned short);
LS_DEFINE_CLASS_TYPE(Pointer, signed int);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned int);
LS_DEFINE_CLASS_TYPE(Pointer, signed long);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned long);
LS_DEFINE_CLASS_TYPE(Pointer, signed long long);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned long long);
LS_DEFINE_CLASS_TYPE(Pointer, float);
LS_DEFINE_CLASS_TYPE(Pointer, double);
LS_DEFINE_CLASS_TYPE(Pointer, long double);

/*-----------------------------------------------------------------------------
 * Dynamic Array Types
 * --------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(Pointer, bool[]);
LS_DEFINE_CLASS_TYPE(Pointer, signed char[]);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned char[]);
LS_DEFINE_CLASS_TYPE(Pointer, wchar_t[]);
LS_DEFINE_CLASS_TYPE(Pointer, char16_t[]);
LS_DEFINE_CLASS_TYPE(Pointer, char32_t[]);
LS_DEFINE_CLASS_TYPE(Pointer, signed short[]);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned short[]);
LS_DEFINE_CLASS_TYPE(Pointer, signed int[]);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned int[]);
LS_DEFINE_CLASS_TYPE(Pointer, signed long[]);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned long[]);
LS_DEFINE_CLASS_TYPE(Pointer, signed long long[]);
LS_DEFINE_CLASS_TYPE(Pointer, unsigned long long[]);
LS_DEFINE_CLASS_TYPE(Pointer, float[]);
LS_DEFINE_CLASS_TYPE(Pointer, double[]);
LS_DEFINE_CLASS_TYPE(Pointer, long double[]);

} // end utils namespace
} // end ls namespace
