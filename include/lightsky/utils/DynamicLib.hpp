
#ifndef LS_UTILS_DYNAMIC_LIB_HPP
#define LS_UTILS_DYNAMIC_LIB_HPP

#include <string>

#include "lightsky/utils/Pointer.h"

namespace ls
{
namespace utils
{



/**
 * @brief The DynamicLib class represents a handle to a shared library on the
 * local file system.
 */
class DynamicLib
{
  private:
    ls::utils::Pointer<char[]> mLibName;

    void* mHandle;

  public:
    /**
     * @brief Destructor
     *
     * Closes the current library handle and releases all resources back to the
     * operating system.
     */
    ~DynamicLib() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all internal variables to their defaults.
     */
    DynamicLib() noexcept;

    /**
     * @brief Library Constructor
     *
     * @param libPath
     * A C-style string which contains the relative path to a dynamic library
     * which should be loaded by this constructor.
     *
     * No error will be thrown on failure, it is recommended to check the
     * "loaded()" function for status.
     */
    DynamicLib(const char* libPath) noexcept;

    /**
     * @brief Copy Constructor
     *
     * Copies all data from the input parameter into *this, followed by a
     * subsequent call to "load(...)" to reload all dynamic library symbols.
     */
    DynamicLib(const DynamicLib&) noexcept;

    /**
     * @brief Move Constructor
     *
     * Moves all internal data from the input parameter into *this without
     * reloading symbols or reallocating data.
     */
    DynamicLib(DynamicLib&&) noexcept;

    /**
     * @brief Copy Operator
     *
     * Copies all data from the input parameter into *this, followed by a
     * subsequent call to "load(...)" to reload all dynamic library symbols.
     *
     * Any symbols or memory previously allocated by *this will be unloaded
     * before copying data from the input parameter.
     *
     * @return A reference to *this.
     */
    DynamicLib& operator=(const DynamicLib&) noexcept;

    /**
     * @brief Move Operator
     *
     * Moves all internal data from the input parameter into *this without
     * reloading symbols or reallocating data.
     *
     * Any symbols or memory previously allocated by *this will be unloaded
     * before moving data from the input parameter.
     *
     * @return A reference to *this.
     */
    DynamicLib& operator=(DynamicLib&&) noexcept;

    /**
     * @brief Load a dynamic library from the local file system.
     *
     * Any symbols or memory previously allocated by *this will be unloaded
     * before loading the new library.
     *
     * @param libPath
     * A C-style string which contains the relative path to a dynamic library
     * which should be loaded by this constructor.
     *
     * @return 0 if the library was successfully loaded, -1 if the input
     * library contained an invalid string, or -2 if there was an error loading
     * the library.
     */
    int load(const char* libPath) noexcept;

    /**
     * @brief Unload the current library handle, all symbols, and memory
     * allocated for internal purposes.
     */
    void unload() noexcept;

    /**
     * @brief Check if a dynamic libray has beed loaded by *this.
     *
     * @return TRUE if a library is currently loaded, FALSE if not.
     */
    bool loaded() const noexcept;

    /**
     * @brief Retrieve the name, or path to the currently loaded library.
     *
     * @return A copy of the library name which was passed into the "load(...)"
     * function, or NULL if a library is not currently loaded.
     */
    const char* name() const noexcept;

    /**
     * @brief Retrieve the native operating system's handle to the current
     * library.
     *
     * @return On Windows systems, a HMODULE is returned, cast as a void*
     * pointer. On Unix or Linux systems, the native void* pointer returned
     * from dlopen(...) is returned.
     */
    const void* native_handle() const noexcept;

    /**
     * @brief Retrieve the native operating system's handle to the current
     * library.
     *
     * @return On Windows systems, a HMODULE is returned, cast as a void*
     * pointer. On Unix or Linux systems, the native void* pointer returned
     * from dlopen(...) is returned.
     */
    void* native_handle() noexcept;

    /**
     * @brief Lookup and retrieve a symbol from the currently loaded shared
     * library. NULL will be returned if no library is currently loaded.
     *
     * @param pSymName
     * A C-style string, containing the name of the symbol to lookup.
     *
     * @return A pointer to the symbol located within the shared library, or
     * NULL if the symbol was not found. On Windows systems, a FARPROC is
     * returned, cast as a void* pointer.
     *
     */
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
