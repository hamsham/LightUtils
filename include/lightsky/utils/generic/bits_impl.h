
namespace ls {

/*-------------------------------------
    getByte
-------------------------------------*/
template <typename key_t>
constexpr const utils::bitMask* utils::getByte(const key_t* k, unsigned iter) {
    return (iter < sizeof(k))
        ? reinterpret_cast<const utils::bitMask*>(k) + iter
        : nullptr;
}

/*-------------------------------------
    getByte (char string specialization)
-------------------------------------*/
template <>
constexpr const utils::bitMask* utils::getByte(const char* str, unsigned iter) {
    return (str[iter / sizeof(char)] != '\0')
        ? reinterpret_cast<const utils::bitMask*>(str) + iter
        : nullptr;
}

/*-------------------------------------
    getByte (wchar_t string specialization)
-------------------------------------*/
template <>
constexpr const utils::bitMask* utils::getByte(const wchar_t* str, unsigned iter) {
    return (str[iter / sizeof(wchar_t)] != '\0')
        ? reinterpret_cast<const utils::bitMask*>(str) + iter
        : nullptr;
}

/*-------------------------------------
    getByte (char16_t string specialization)
-------------------------------------*/
template <>
constexpr const utils::bitMask* utils::getByte(const char16_t* str, unsigned iter) {
    return (str[iter / sizeof(char16_t)] != '\0')
        ? reinterpret_cast<const utils::bitMask*>(str) + iter
        : nullptr;
}

/*-------------------------------------
    getByte (char32_t string specialization)
-------------------------------------*/
template <>
constexpr const utils::bitMask* utils::getByte(const char32_t* str, unsigned iter) {
    return (str[iter / sizeof(char32_t)] != '\0')
        ? reinterpret_cast<const utils::bitMask*>(str) + iter
        : nullptr;
}

} // end ls namespace
