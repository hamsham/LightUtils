
namespace ls {

/*-------------------------------------
    getArgByteSize (Sentinel)
-------------------------------------*/
constexpr
unsigned utils::getArgByteSize(unsigned size) {
    return size;
}

/*-------------------------------------
    getArgByteSize
-------------------------------------*/
template <typename... integral_t> constexpr
unsigned utils::getArgByteSize(unsigned size, integral_t... sizeN) {
    return size + utils::getArgByteSize(sizeN...);
}

/*-------------------------------------
    getByteSize
-------------------------------------*/
template <typename... integral_t> constexpr
unsigned utils::getByteSize() {
    return utils::getArgByteSize(sizeof(integral_t)...);
}

} // end ls namespace