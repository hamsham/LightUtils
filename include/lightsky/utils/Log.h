
/**
 * utils/log.h
 *
 * This file contains wrappers around std::cout and std::cerr. It's just a
 * simple way to clean up some of the syntax when writing to a standard output
 * stream.
 */

#ifndef __LS_UTILS_LOG_H__
#define __LS_UTILS_LOG_H__

#include "lightsky/setup/OS.h"

namespace ls {
namespace utils {

/**
 * @brief Print multiple parameters to std::cout.
 *
 * This method is syntactic sugar to write data to a standard output stream. It
 * is similar to the printf() family of functions, however all values are
 * separated by commas. Use of this function will cause a new line to be
 * printed to the output stream afterwards; std::cout.flush() will also be
 * called.
 *
 * @param arg
 * A constant reference to a data type which can be send to std::cout using the
 * 'std::ostream::operator::<<()' overload.
 */
template <typename arg_t>
inline
void log_msg(const arg_t& arg);

/**
 * @brief Print multiple parameters to std::cout.
 *
 * This method is syntactic sugar to write data to a standard output stream. It
 * is similar to the printf() family of functions, however all values are
 * separated by commas. Use of this function will cause a new line to be
 * printed to the output stream afterwards; std::cout.flush() will also be
 * called.
 *
 * @param arg
 * A constant reference to a data type which can be send to std::cout using the
 * 'std::ostream::operator::<<()' overload.
 *
 * @param args
 * A constant reference to multiple variadic arguments that will be written to
 * std::cout after 'arg' is written.
 */
template <typename arg_t, typename... args_t>
inline
void log_msg(const arg_t& arg, const args_t&... args);

/**
 * @brief Print multiple parameters to std::cerr.
 *
 * This method is syntactic sugar to write data to a standard output stream. It
 * is similar to the printf() family of functions, however all values are
 * separated by commas. Use of this function will cause a new line to be
 * printed to the output stream afterwards; std::cerr.flush() will also be
 * called.
 *
 * @param arg
 * A constant reference to a data type which can be send to std::cerr using the
 * 'std::ostream::operator::<<()' overload.
 */
template <typename arg_t>
inline
void log_err(const arg_t& arg);

/**
 * @brief Print multiple parameters to std::cerr.
 *
 * This method is syntactic sugar to write data to a standard output stream. It
 * is similar to the printf() family of functions, however all values are
 * separated by commas. Use of this function will cause a new line to be
 * printed to the output stream afterwards; std::cerr.flush() will also be
 * called.
 *
 * @param arg
 * A constant reference to a data type which can be send to std::cerr using the
 * 'std::ostream::operator::<<()' overload.
 *
 * @param args
 * A constant reference to multiple variadic arguments that will be written to
 * std::cerr after 'arg' is written.
 */
template <typename arg_t, typename... args_t>
inline
void log_err(const arg_t& arg, const args_t&... args);

} // end utils namespace
} // end ls namespace

#ifdef LS_OS_ANDROID
    #include "lightsky/utils/android/LogImpl.h"
#else
    #include "lightsky/utils/generic/LogImpl.h"
#endif

/*-----------------------------------------------------------------------------
    Debugging Various Messages.
-----------------------------------------------------------------------------*/
#ifdef LS_DEBUG
    #ifndef LS_LOG_MSG
        #define LS_LOG_MSG(...) ls::utils::log_msg(__VA_ARGS__)
    #endif

    #ifndef LS_LOG_ERR
        #define LS_LOG_ERR(...) ls::utils::log_err(__VA_ARGS__)
    #endif
#else
    template <typename arg_t>
    constexpr
    int LS_LOG_MSG(const arg_t&)
    {
        return 0;
    }

    template <typename arg_t, typename... args_t>
    constexpr
    int LS_LOG_MSG(const arg_t&, const args_t&...)
    {
        return 0;
    }

    template <typename arg_t>
    constexpr
    int LS_LOG_ERR(const arg_t&)
    {
        return 0;
    }

    template <typename arg_t, typename... args_t>
    constexpr
    int LS_LOG_ERR(const arg_t&, const args_t&...)
    {
        return 0;
    }
#endif

#endif /* __LS_UTILS_LOG_H__ */
