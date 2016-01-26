
#include <iostream>

namespace ls {

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
void utils::log_msg(const arg_t& arg)
{
    std::cout << arg << '\n';
    std::cout.flush();
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
void utils::log_msg(const arg_t& arg, const args_t&... args)
{
    std::cout << arg;
    log_msg(args...);
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
void utils::log_err(const arg_t& arg)
{
    std::cerr << arg << '\n';
    std::cerr.flush();
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
void utils::log_err(const arg_t& arg, const args_t&... args)
{
    std::cerr << arg;
    log_msg(args...);
}

} // end ls namespace
