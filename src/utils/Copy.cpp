
#include "ls/utils/Bits.h"
#include "ls/utils/Copy.h"



namespace ls {



/*-------------------------------------
 * fast_memcpy
-------------------------------------*/
void* utils::fast_memcpy(void* const dest, const void* const src, const std::size_t count) {
    char* to = reinterpret_cast<char*>(dest);
    const char* from = reinterpret_cast<const char*>(src);
    
    LS_UTILS_LOOP_UNROLL_32(count, (*to++ = *from++))

    return dest;
}



/*-------------------------------------
 * fast_memset
-------------------------------------*/
void* utils::fast_memset(void* dest, const unsigned char fillByte, std::size_t count) {
    char* to = reinterpret_cast<char*>(dest);
    
    LS_UTILS_LOOP_UNROLL_32(count, (*to++ = fillByte))

    return dest;
}



} // end ls namespace
