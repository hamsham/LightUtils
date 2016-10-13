
#include <csignal> // raise(), SIGABRT
#include <cstdlib> // std::exit, EXIT_FAILURE
#include <exception> // std::terminate

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

namespace ls {

void utils::runtime_assert(bool condition, error_t type, const char* const msg) {
    if (condition) {
        return;
    }

    const char* const errorString[] = {"ALERT: ", "WARNING: ", "ERROR: "};
    LS_LOG_ERR(errorString[type], msg);

    if (type == utils::LS_WARNING) {
        exit(EXIT_FAILURE);
    }
    else if (type == utils::LS_ERROR) {
        abort();
    }
}

} // end ls namespace
