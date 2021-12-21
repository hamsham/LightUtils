
#include <csignal>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"



namespace ls
{



void utils::runtime_assert(bool condition, utils::ErrorLevel severity, const char* const msg)
{
    if (condition)
    {
        return;
    }

    if (severity == utils::ErrorLevel::LS_WARNING)
    {
        ls::utils::log_err("WARNING: ", msg);
    }
    else if (severity == utils::ErrorLevel::LS_ERROR)
    {
        ls::utils::log_err("ERROR: ", msg);
        raise(SIGABRT);
    }
}



} // end ls namespace
