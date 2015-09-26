
namespace ls {

/*-------------------------------------
    Type-To-String Conversion
-------------------------------------*/
template <typename T>
std::string utils::toString(const T& data) {
    std::ostringstream oss;
    oss << data;
    return oss.str();
}

} // end ls namespace
