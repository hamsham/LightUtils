
#include <cstring>
#include <utility> // std::move

#include "lightsky/setup/OS.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/DynamicLib.hpp"


#ifdef LS_OS_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif



namespace ls
{
namespace utils
{



/*-------------------------------------
 * Destructor
-------------------------------------*/
DynamicLib::~DynamicLib() noexcept
{
    if (loaded())
    {
        unload();
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
DynamicLib::DynamicLib() noexcept :
    mLibName{nullptr},
    mHandle{nullptr}
{}



/*-------------------------------------
 * Library Constructor
-------------------------------------*/
DynamicLib::DynamicLib(const char* libPath) noexcept :
    mLibName{nullptr},
    mHandle{nullptr}
{
    load(libPath);
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
DynamicLib::DynamicLib(const DynamicLib& d) noexcept
{
    if (d.loaded())
    {
        load(d.name());
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
DynamicLib::DynamicLib(DynamicLib&& d) noexcept :
    mLibName{std::move(d.mLibName)},
    mHandle{d.mHandle}
{
    d.mLibName = nullptr;
    d.mHandle = nullptr;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
DynamicLib& DynamicLib::operator=(const DynamicLib& d) noexcept
{
    if (loaded())
    {
        unload();
    }

    if (d.loaded())
    {
        load(d.name());
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
DynamicLib& DynamicLib::operator=(DynamicLib&& d) noexcept
{
    if (loaded())
    {
        unload();
    }

    mLibName = std::move(d.mLibName);
    mHandle = d.mHandle;

    d.mLibName = nullptr;
    d.mHandle = nullptr;

    return *this;
}



/*-------------------------------------
 * Load the dynamic library
-------------------------------------*/
int DynamicLib::load(const char* libPath) noexcept
{
    if (loaded())
    {
        unload();
    }

    if (!libPath)
    {
        return -1;
    }

    #ifdef LS_OS_WINDOWS
        void* pHandle = reinterpret_cast<void*>(LoadLibraryA(libPath));
    #else
        void* pHandle = dlopen(libPath, RTLD_NOW | RTLD_LOCAL);
    #endif

    if (!pHandle)
    {
        return -2;
    }

    size_t pathLen = strlen(libPath);
    ls::utils::UniqueAlignedArray<char>&& other = make_unique_aligned_array<char>(pathLen+1);
    mLibName.swap(other);
    memcpy(mLibName.get(), libPath, pathLen);
    mLibName[pathLen] = '\0';

    mHandle = pHandle;

    return 0;
}



/*-------------------------------------
 * Unload the library
-------------------------------------*/
void DynamicLib::unload() noexcept
{
    if (!loaded())
    {
        return;
    }

    #ifdef LS_OS_WINDOWS
        FreeLibrary((HMODULE)mHandle);
    #else
        dlclose(mHandle);
    #endif

    mLibName.reset();
    mHandle = nullptr;
}



/*-------------------------------------
 * Get a symbol from the library
-------------------------------------*/
void* DynamicLib::symbol(const char* pSymName) const noexcept
{
    LS_DEBUG_ASSERT(mHandle != nullptr);

    #ifdef LS_OS_WINDOWS
        return reinterpret_cast<void*>(GetProcAddress((HMODULE)const_cast<void*>(mHandle), pSymName));
    #else
        return dlsym(mHandle, pSymName);
    #endif
}



} // end utils namespace
} // end ls namespace
