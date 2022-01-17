
#include "lightsky/setup/OS.h" // LS_OS_WINDOWS

#include "lightsky/utils/Log.h"
#include "lightsky/utils/WorkerThread.hpp"

#ifdef LS_OS_UNIX
    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif

    #include <sched.h>
    #include <unistd.h>
    #include <pthread.h>
#endif



/*-----------------------------------------------------------------------------
 * Thread ID
-----------------------------------------------------------------------------*/
size_t ls::utils::get_thread_id() noexcept
{
    #ifdef LS_OS_UNIX
        return (size_t)pthread_self();
    #else
        constexpr std::hash<std::thread::id> hasher;
        return (size_t)hasher(std::this_thread::get_id());
    #endif
}



/*-----------------------------------------------------------------------------
 * Thread Affinity
-----------------------------------------------------------------------------*/
bool ls::utils::set_thread_affinity(size_t threadId, unsigned affinity) noexcept
{
    // TODO: Implement for OSX and Windows
    #ifndef LS_OS_LINUX
        (void)threadId;
        (void)affinity;
        return false;

    #else
        const unsigned numThreads = (unsigned)sysconf(_SC_NPROCESSORS_ONLN);
        bool ret = true;

        if (affinity < numThreads)
        {
            cpu_set_t cpuSet;
            CPU_ZERO(&cpuSet);
            CPU_SET(affinity, &cpuSet);

            pthread_t t = (pthread_t)threadId;
            #ifdef LS_OS_ANDROID
            if (sched_setaffinity((pid_t)t, sizeof(cpu_set_t), &cpuSet))
            {
                LS_LOG_ERR("Unable to set CPU affinity.");
                ret = false;
            }
            #else
            if (pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuSet))
            {
                LS_LOG_ERR("Unable to set CPU affinity.");
                ret = false;
            }
            #endif
        }
        else
        {
            LS_LOG_ERR("Requested CPU affinity is out of range for expected values (", affinity, " vs. 0 - ", numThreads, ").");
            ret = false;
        }

        return ret;
    #endif
}



bool ls::utils::set_thread_affinity(std::thread& t, unsigned affinity) noexcept
{
    return set_thread_affinity((size_t)t.native_handle(), affinity);
}



/*-----------------------------------------------------------------------------
 * WorkerThread
-----------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(ls::utils::WorkerThread, void (*)());
