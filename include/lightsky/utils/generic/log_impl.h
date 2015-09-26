
namespace ls {
namespace utils {

template <typename Arg>
inline void logMsg(const Arg& arg) {
    std::cout << arg << '\n';
    std::cout.flush();
}

template <typename Arg, typename... Args>
inline void logMsg(const Arg& arg, const Args&... args) {
    std::cout << arg;
    logMsg(args...);
}

template <typename Arg>
inline void logErr(const Arg& arg) {
    std::cerr << arg << '\n';
    std::cerr.flush();
}

template <typename Arg, typename... Args>
inline void logErr(const Arg& arg, const Args&... args) {
    std::cerr << arg;
    logErr(args...);
}

} // end utils namespace
} // end ls namespace
