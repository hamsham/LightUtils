
namespace ls {
namespace utils {

/*-------------------------------------
    Get the size, in bytes, of the current file loaded into memory.
-------------------------------------*/
inline long resource::getByteSize() const {
    return dataSize;
}

/*-------------------------------------
    Get the raw, loaded, data contained within *this.
-------------------------------------*/
inline void* resource::getData() const {
    return pData;
}

/*-------------------------------------
    Open a file with UTF-8
-------------------------------------*/
inline bool resource::loadFile(const std::wstring& filename) {
    // attempt to load the file
    return loadFile(convertWtoMb(filename));
}

/*-------------------------------------
    Save with an UTF-8 filename
-------------------------------------*/
inline bool resource::saveFile(const std::wstring& filename) const {
    // attempt to save the file using a multi-byte string.
    return saveFile(convertWtoMb(filename));
}

} // end utils namespace
} // end ls namespace
