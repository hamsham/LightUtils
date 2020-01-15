/*
 * File:   utils/string_utils.h
 * Author: Miles Lacey
 *
 * Created on May 23, 2014, 7:10 PM
 */

#ifndef LS_UTILS_STRING_UTILS_H
#define LS_UTILS_STRING_UTILS_H

#include <string>

namespace ls
{
namespace utils
{



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
std::string to_str(char);
std::string to_str(unsigned char);
std::string to_str(short);
std::string to_str(unsigned short);
std::string to_str(int);
std::string to_str(unsigned int);
std::string to_str(long);
std::string to_str(unsigned long);
std::string to_str(long long);
std::string to_str(unsigned long long);
std::string to_str(float);
std::string to_str(double);
std::string to_str(long double);



/**
 *  @brief Convert a Wide String to a Multi-Byte Character String
 *
 *  This function assists in making an application cope with UTF-8
 *  compatibility issues.
 *
 *  @note To correctly convert between different character sets, you must set
 *  the current program's locale using std::setlocale(). For example, call
 *  std::setlocale(LC_CTYPE, ""); before calling this function.
 *
 *  @param wstr
 *  A wide-character string object which needs to be converted into a
 *  multi-byte string representation.
 *
 *  @return std::string
 *  A std::string object that uses 'char' types instead of the input
 *  parameter's 'wchar_t' type.
 */
std::string to_str(const std::wstring& wstr);
std::string to_str(const std::u16string& wstr);
std::string to_str(const std::u32string& wstr);



} // end utils namespace
} // end ls namespace

#endif  /* LS_UTILS_STRING_UTILS_H */
