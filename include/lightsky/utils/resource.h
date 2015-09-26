/* 
 * File:   resource.h
 * Author: Miles Lacey
 *
 * Created on February 1, 2014, 11:09 PM
 */

#ifndef __LS_UTILS_RESOURCE_H__
#define	__LS_UTILS_RESOURCE_H__

#include <string>

#include "lightsky/utils/setup.h"
#include "lightsky/utils/string_utils.h"

namespace ls {
namespace utils {

/**
 * @brief Basic file/resource abstraction.
 *
 * Resource Objects can open a file by calling either openFile() or saveFile()
 * using a wide character string, The function merely converts the wide string
 * into a multi-byte character string, then delegates the rest of the file
 * operation to the c-string openFile/closeFile methods. This means that if this
 * class is derived from, there is little need to overload the wide-string
 * methods.
 */
class resource {
    protected:
        /**
         * pData is a pointer to an array of bytes that hold a complete resource
         * in RAM. Data is automatically freed during a file object's
         * destruction but can also be freed by calling "unload()."
         */
        char* pData = nullptr;

        /**
         * dataSize holds the current size, in bytes, of the memory being used
         * by pData.
         */
        long dataSize = 0l;

    public:

        /**
         * @brief Constructor
         *
         * Initializes all members in *this.
         */
        resource() = default;

        /**
         * @brief Copy Constructor
         *
         * Attempts to copy all data from the source operand into *this.
         *
         * @param r
         * A constant reference to a resource object.
         */
        resource(const resource& r) = delete;

        /**
         * @brief Move Constructor
         *
         * Move all memory from the source object into *this. No copies are
         * performed during this operation.
         *
         * @param r
         * An r-value reference to a resource object.
         */
        resource(resource&& r) = delete;

        /**
         * Destructor.
         * Make sure to call "unload()" before an object goes out of scope.
         */
        virtual ~resource() = 0;

        /**
         * @brief Copy operator
         *
         * Attempts to copy all data from the source operand into *this.
         *
         * @param r
         * A constant reference to a resource object.
         * 
         * @return a reference to *this.
         */
        resource& operator =(const resource& r) = delete;
        
        /**
         * @brief Move operator
         *
         * Move all memory from the source object into *this. No copies are
         * performed during this operation.
         *
         * @param r
         * An r-value reference to a resource object.
         *
         * @return a reference to *this.
         */
        resource& operator =(resource&& r) = delete;

        /**
         * @brief Load a file
         * 
         * @param filename
         * A string object containing the relative path name to a file that
         * should be loadable into memory.
         * 
         * @return true if the file was successfully loaded. False if not.
         */
        virtual bool loadFile(const std::string& filename) = 0;

        /**
         * @brief Load a file using a c-style wide string.
         * 
         * This method merely converts the filename into a multi-byte string
         * and calls "openFile()" using the ANSI equivalent string.
         * 
         * @param filename
         * A string object containing the relative path name to a file that
         * should be loadable into memory.
         * 
         * @return true if the file was successfully loaded. False if not.
         */
        virtual bool loadFile(const std::wstring& filename);

        /**
         * @brief Save a file
         *
         * Use this method to save data to a file, specific to the type of
         * resource used by derived classes.
         * 
         * @param filename
         * A string object containing the relative path name to a file that
         * should be saved to the computer.
         * 
         * @return true if the file was successfully saved. False if not.
         */
        virtual bool saveFile(const std::string& filename) const = 0;

        /**
         * @brief Save a file using a c-style string of wide (UTF-8) characters
         * 
         * This method merely converts the filename into a multi-byte string
         * and calls "saveFile()" using the ANSI equivalent string.
         * 
         * @param filename
         * A string object containing the relative path name to a file that
         * should be saved to the computer.
         * 
         * @return true if the file was successfully saved. False if not.
         */
        virtual bool saveFile(const std::wstring& filename) const;

        /**
         * @brief Unload
         *
         * Free all memory used by *this.
         */
        virtual void unload() = 0;

        /**
         *  Get the size, in bytes, of the current file loaded into memory.
         *  
         *  @return a long integral type, used to determine how many bytes
         *  had been loaded from a file.
         */
        virtual long getByteSize() const;

        /**
         *  @brief Get the raw, loaded, data contained within *this.
         *  
         *  This method may be overridden by derived classes in order to
         *  provide data specific to a certain module's needs. In such a case,
         *  please see the associated documentation in order to determine how
         *  to use this method.
         *  
         *  @return a pointer to a chunk of data loaded from a file.
         */
        virtual void* getData() const;
};

} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/resource_impl.h"

#endif	/* __LS_UTILS_RESOURCE_H__ */
