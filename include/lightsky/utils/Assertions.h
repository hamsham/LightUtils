
#ifndef LS_UTILS_ASSERT_H
#define LS_UTILS_ASSERT_H

#include <cassert>

#include "lightsky/setup/Api.h"
#include "lightsky/setup/Macros.h"

namespace ls {
namespace utils {



/**
 *  @brief error_t
 *  A basic enumeration for ls assertions.
 */
enum error_t : int {
    /**
     *  @brief ALERT
     *  when used with runtime_assert, this indicates that a message will print
     *  to std::cout.
     */
    LS_ALERT,

    /**
     *  @brief WARNING
     *  when used with runtime_assert, this indicates that a message will print
     *  to std::cerr.
     */
    LS_WARNING,

    /**
     *  @brief ERROR
     *  when used with runtime_assert, this indicates that a message will print
     *  to std::cerr, and an exception of type ls::utils::error_t is thrown.
     */
    LS_ERROR
};

/**
 *  @brief runtime_assert
 *  Throw an ls::utils::error_t and/or send a message to stdout/stderr.
 *
 *  @param condition
 *  The boolean check to determine if an assertion should be raised. If the
 *  condition tests TRUE, then no assertion is made, otherwise an exception
 *  could be raised (of type ls::utils::error_t), a message could be printed
 *  to stdout, or an error will be sent to stderr.
 *
 *  @param type
 *  An error type that indicates if an assertion is a simple alert message, a
 *  warning, or a critical error.
 *
 *  @param msg
 *  The message that will be printed to an standard output stream if the
 *  condition tests FALSE.
 */
void runtime_assert(bool condition, error_t type, const char* const msg);

} // end Utils namespace
} // end ls namespace

/*-------------------------------------
 * Basic Assertion Template
 * ----------------------------------*/
#ifndef LS_ASSERT_BASIC
    #define LS_ASSERT_BASIC( x, fileName, lineNum, type ) \
        ls::utils::runtime_assert ( \
            (x), type, "Assertion failed on line " LS_STRINGIFY(lineNum) \
            " of " LS_STRINGIFY(fileName) ": (" LS_STRINGIFY(x) ")" \
        )
#endif /* LS_ASSERT_BASIC */

/*-------------------------------------
 * Standard Assertion/Exception
 * ----------------------------------*/
#ifndef LS_ASSERT
    #define LS_ASSERT( x ) LS_ASSERT_BASIC(x, __FILE__, __LINE__, ls::utils::LS_ERROR)
#endif /* ASSERT */

/*-------------------------------------
 * Debug Assertion/Exception
 * ----------------------------------*/
#ifdef LS_DEBUG
    #ifndef LS_DEBUG_ASSERT
        #define LS_DEBUG_ASSERT( x ) LS_ASSERT( x )
    #endif
#else
    #define LS_DEBUG_ASSERT( x )
#endif /* DEBUG */

/*-------------------------------------
 * Warning Message
 * ----------------------------------*/
#ifndef LS_WARN
    #define LS_WARN( x ) LS_ASSERT_BASIC(x, __FILE__, __LINE__, ls::utils::LS_WARNING)
#endif /* ASSERT_WARN */

/*-------------------------------------
 * Simple Alert Message
 * ----------------------------------*/
#ifndef LS_ALERT
    #define LS_ALERT( x ) LS_ASSERT_BASIC(x, __FILE__, __LINE__, ls::utils::LS_ALERT)
#endif /* ASSERT_ALERT */

#endif /* LS_UTILS_ASSERT_H */
