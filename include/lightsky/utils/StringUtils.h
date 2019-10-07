/*
 * File:   utils/string_utils.h
 * Author: Miles Lacey
 *
 * Created on May 23, 2014, 7:10 PM
 */

#ifndef LS_UTILS_STRING_UTILS_H
#define LS_UTILS_STRING_UTILS_H

#include <string>
#include <sstream>

#include "lightsky/setup/Api.h"

namespace ls {
namespace utils {

/**
 *  @brief Convert a basic data type to an std::string.
 *
 *  This function is only here because not all C++11-supporting versions of GCC
 *  have std::to_string().
 *
 *  @param T
 *  A generic type that can be printed to an std::ostream using the '<<'
 *  operator.
 *
 *  @return std::string
 *  A string-representation of the input parameter
 */
template <typename T>
std::string to_string(const T& data) {
    std::ostringstream oss;
    oss << data;
    return oss.str();
}

/**
 *  @brief Convert a Wide String to a Multi-Byte Character String
 *
 *  This function assists in making an application cope with UTF-8 and UTF-16
 *  compatibility issues.
 *
 *  @param wstr
 *  A wide-character string object which needs to be converted into a
 *  multi-byte string representation.
 *
 *  @return std::string
 *  A std::string object that uses 'char' types instead of the input
 *  parameter's 'wchar_t' type.
 */
std::string wide_to_mb_string(const std::wstring& wstr);

} // end utils namespace
} // end ls namespace

#endif  /* LS_UTILS_STRING_UTILS_H */
