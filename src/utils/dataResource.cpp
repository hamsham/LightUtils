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
DataResource::~DataResource()
{
    unload();
}

/*-------------------------------------
 * Constructor
 * ----------------------------------*/
DataResource::DataResource()
:
    Resource{},
    fileData{}
{}

/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
DataResource::DataResource(const DataResource& f)
:
    Resource{}
{
    set_data(const_cast<char*>(f.pData), f.dataSize);
}

/*-------------------------------------
 * Move Constructor
 * ----------------------------------*/
DataResource::DataResource(DataResource&& f)
:
    Resource{},
    fileData{std::move(f.fileData)}
{
    pData = &fileData[0];
    reassign_base_members();

    f.pData = nullptr;
    f.dataSize = 0;
}

/*-------------------------------------
 * Copy Operator
 * ----------------------------------*/
DataResource& DataResource::operator=(const DataResource& f)
{
    set_data(const_cast<char*>(f.pData), f.dataSize);
    return *this;
}

/*-------------------------------------
 * Move Operator
 * ----------------------------------*/
DataResource& DataResource::operator =(DataResource&& f)
{
    unload();

    fileData = std::move(f.fileData);
    reassign_base_members();

    f.pData = nullptr;
    f.dataSize = 0;

    return *this;
}

/*-------------------------------------
 * Reassign base class members
 * ----------------------------------*/
void DataResource::reassign_base_members()
{
    pData = &fileData[0];
    dataSize = sizeof(decltype(fileData)::value_type) * fileData.size();
}

/*-------------------------------------
 * Unload a resource
 * ----------------------------------*/
void DataResource::unload()
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
bool DataResource::load_file(const std::string& filename)
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
    reassign_base_members();

    return true;
}

/*-------------------------------------
 * Save with a UTF-8 filename
 * ----------------------------------*/
bool DataResource::save_file(const std::string& filename) const {
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
bool DataResource::set_data(const char* const data, long size)
{
    unload();

    if (data == nullptr || size == 0)
    {
        return true;
    }


    const unsigned byteSize = sizeof(decltype(fileData)::value_type);
    const unsigned valueSize = (size/byteSize) + (size%byteSize);

    fileData.assign(data, valueSize);
    reassign_base_members();

    return true;
}

} // end utils namespace
} // end ls namespace
