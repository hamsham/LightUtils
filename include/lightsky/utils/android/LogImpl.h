
#include <android/log.h>
#include <utility>
#include <sstream>

namespace ls {
namespace utils {

/*-----------------------------------------------------------------------------
 *  Android Logging Helpers
 * --------------------------------------------------------------------------*/
template <typename arg_t>
inline
void log_android(std::ostringstream& os, const android_LogPriority priority, const arg_t& arg) {
    os << arg;
    __android_log_write(priority, "", os.str().c_str());
}

template <typename arg_t, typename... args_t>
inline
void log_android(std::ostringstream& os, const android_LogPriority priority, const arg_t& arg, const args_t&... args) {
    os << arg;
    log_android(os, priority, args...);
}

} // end utils namespace

/*-----------------------------------------------------------------------------
 *  Standard Logging Interfaces
 * --------------------------------------------------------------------------*/

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
void utils::log_msg(const arg_t& arg) {
    std::ostringstream os;
    log_android(os, android_LogPriority::ANDROID_LOG_INFO, arg);
}

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
void utils::log_msg(const arg_t& arg, const args_t&... args) {
    std::ostringstream os;
    log_android(os, android_LogPriority::ANDROID_LOG_INFO, arg, args...);
}

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
void utils::log_err(const arg_t& arg) {
    std::ostringstream os;
    log_android(os, android_LogPriority::ANDROID_LOG_ERROR, arg);
}

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
void utils::log_err(const arg_t& arg, const args_t&... args) {
    std::ostringstream os;
    log_android(os, android_LogPriority::ANDROID_LOG_ERROR, arg, args...);
}

} // end ls namespace
