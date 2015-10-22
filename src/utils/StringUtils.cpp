/*
 * File:   utils/string_utils.cpp
 * Author: Miles Lacey
 *
 * Created on May 23, 2014, 7:10 PM
 */

#include <cstdlib> // std::wcstombs
#include <cstring> // std::memset, std::wcslen
#include <string> // std::wstring

#include "lightsky/utils/StringUtils.h"

namespace ls {

/*-------------------------------------
 * Wide-String to Multi-Byte
 * ----------------------------------*/
std::string utils::wide_to_mb_string(const std::wstring& wstr) {
    // return value
    std::string ret{};

    // get the size difference between the two data types
    unsigned sizeDifference = sizeof(wchar_t) / sizeof(char);

    // convert the wide string to a multi-byte char string
    std::size_t maxBytes = (wstr.size()*sizeDifference) + 1;

    // Allocate a new character array, 0-initialize the string, just in case
    ret.resize(maxBytes, '\0');

    // convert the wide char to multi-byte
    std::size_t err = std::wcstombs(&ret[0], wstr.c_str(), maxBytes);

    // make sure the conversion worked
    if (err == static_cast<std::size_t>(-1)) {
        ret.clear();
    }

    return ret;
}

} // end ls namespace
