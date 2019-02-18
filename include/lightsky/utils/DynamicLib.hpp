
#ifndef LS_UTILS_DYNAMIC_LIB_HPP
#define LS_UTILS_DYNAMIC_LIB_HPP

#include <string>

#include "lightsky/utils/Pointer.h"

namespace ls
{
namespace utils
{



class DynamicLib
{
  private:
    ls::utils::Pointer<char[]> mLibName;

    void* mHandle;

  public:
    ~DynamicLib() noexcept;

    DynamicLib() noexcept;

    DynamicLib(const DynamicLib&) noexcept;

    DynamicLib(DynamicLib&&) noexcept;

    DynamicLib& operator=(const DynamicLib&) noexcept;

    DynamicLib& operator=(DynamicLib&&) noexcept;

    int load(const char* libPath) noexcept;

    void unload() noexcept;

    bool loaded() const noexcept;

    const char* name() const noexcept;

    const void* native_handle() const noexcept;

    void* native_handle() noexcept;

    void* symbol(const char* const pSymName) const noexcept;
};



/*-------------------------------------
 * Check if a library is loaded
-------------------------------------*/
inline bool DynamicLib::loaded() const noexcept
{
    return mHandle != nullptr;
}



/*-------------------------------------
 * Get the name of a library
-------------------------------------*/
inline const char* DynamicLib::name() const noexcept
{
    return mLibName.get();
}



/*-------------------------------------
 * Get the native library handle
-------------------------------------*/
inline const void* DynamicLib::native_handle() const noexcept
{
    return mHandle;
}



/*-------------------------------------
 * Get the native library handle
-------------------------------------*/
inline void* DynamicLib::native_handle() noexcept
{
    return mHandle;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_DYNAMIC_LIB_HPP */
