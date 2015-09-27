/*
 * File:   utils/dataResource.cpp
 * Author: Miles Lacey
 *
 * Created on February 1, 2014, 11:09 PM
 */

#include <utility> // std::move
#include <new> // std::nothrow
#include <algorithm> // std::copy
#include <fstream> // std::fstream
#include <string>
#include <sstream>

#include "lightsky/utils/dataResource.h"

namespace ls {
namespace utils {

/*-------------------------------------
 * Destructor
 * ----------------------------------*/
dataResource::~dataResource()
{
    unload();
}

/*-------------------------------------
 * Constructor
 * ----------------------------------*/
dataResource::dataResource()
:
    resource{},
    fileData{}
{}

/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
dataResource::dataResource(const dataResource& f)
:
    resource{}
{
    setData(const_cast<char*>(f.pData), f.dataSize);
}

/*-------------------------------------
 * Move Constructor
 * ----------------------------------*/
dataResource::dataResource(dataResource&& f)
:
    resource{},
    fileData{std::move(f.fileData)}
{
    pData = &fileData[0];
    reassignBaseMembers();

    f.pData = nullptr;
    f.dataSize = 0;
}

/*-------------------------------------
 * Copy Operator
 * ----------------------------------*/
dataResource& dataResource::operator=(const dataResource& f)
{
    setData(const_cast<char*>(f.pData), f.dataSize);
    return *this;
}

/*-------------------------------------
 * Move Operator
 * ----------------------------------*/
dataResource& dataResource::operator =(dataResource&& f)
{
    unload();

    fileData = std::move(f.fileData);
    reassignBaseMembers();

    f.pData = nullptr;
    f.dataSize = 0;

    return *this;
}

/*-------------------------------------
 * Reassign base class members
 * ----------------------------------*/
void dataResource::reassignBaseMembers()
{
    pData = &fileData[0];
    dataSize = sizeof(decltype(fileData)::value_type) * fileData.size();
}

/*-------------------------------------
 * Unload a resource
 * ----------------------------------*/
void dataResource::unload()
{
    fileData.clear();
    pData = nullptr;
    dataSize = 0;
}

/*-------------------------------------
 * Open a file using UTF-8
 *
 * This method converts a file's input stream to an std::ostringstream's read
 * buffer. See the following link on why this is a better idea than seeking
 * to/from the beginning and end of a binary file to get it's size or using
 * stream iterators to populate a string object:
 *
 * http://cpp.indi.frih.net/blog/2014/09/how-to-read-an-entire-file-into-memory-in-cpp/
 * ----------------------------------*/
bool dataResource::loadFile(const std::string& filename)
{
    unload();

    std::ifstream fin;
    fin.open(filename, std::ios_base::binary | std::ios_base::in);

    if (!fin.good())
    {
        return false;
    }

    // Determine of the file can successfully scanned,
    if (fin.bad() || fin.fail())
    {
        fin.close();
        return false;
    }

    // convert the input file's stream to an std::ostringstream's output buffer.
    std::ostringstream oss{};
    oss << fin.rdbuf();

    // redundancy
    if (oss.bad() || oss.fail())
    {
        fin.close();
        return false;
    }

    // move the string stream's buffer into a string
    fileData = std::move(oss.str());
    reassignBaseMembers();

    return true;
}

/*-------------------------------------
 * Save with a UTF-8 filename
 * ----------------------------------*/
bool dataResource::saveFile(const std::string& filename) const {
    std::ofstream fout;

    fout.open(filename, std::ios_base::binary);

    if (!fout.good())
    {
        return false;
    }

    fout.write(pData, dataSize);

    const bool ret = fout.good();

    fout.close();

    return ret;
}

/*-------------------------------------
 * Set a resource's data
 * ----------------------------------*/
bool dataResource::setData(const char* const data, long size)
{
    unload();

    if (data == nullptr || size == 0)
    {
        return true;
    }


    const unsigned byteSize = sizeof(decltype(fileData)::value_type);
    const unsigned valueSize = (size/byteSize) + (size%byteSize);

    fileData.assign(data, valueSize);
    reassignBaseMembers();

    return true;
}

} // end utils namespace
} // end ls namespace
