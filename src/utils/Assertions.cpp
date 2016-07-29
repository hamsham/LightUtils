
#include <iostream>
#include <csignal>
#include <string>

#include "ls/utils/Assertions.h"

namespace ls {

void utils::runtime_assert(bool condition, error_t type, const char* const msg) {
    if (condition) {
        return;
    }

    const char* const errorString[] = {"ALERT: ", "WARNING: ", "ERROR: "};
    std::cerr << errorString[type] << msg << std::endl;

    if (type == utils::LS_WARNING) {
        std::exit(EXIT_FAILURE);
    }
    else if (type == utils::LS_ERROR) {
        raise(SIGABRT);
    }
}

} // end ls namespace
