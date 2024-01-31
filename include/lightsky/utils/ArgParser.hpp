
#ifndef ARGPARSE_ARG_PARSER_HPP
#define ARGPARSE_ARG_PARSER_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace ls
{
namespace utils
{
namespace argparse
{



/*-----------------------------------------------------------------------------
 * Forward declarations
-----------------------------------------------------------------------------*/
class Argument;




/*-----------------------------------------------------------------------------
 * Enums
-----------------------------------------------------------------------------*/
enum class ArgErrCode : int
{
    SUCCESS = 0,
    INTERNAL_NO_FLAG_VALUE_SET = -1,
    INTERNAL_CONST_ARG_COUNT_MISMATCH = -2,
    INTERNAL_CONST_ARG_TYPE_MISMATCH = -3,
    NO_DEFAULT_VALUE_AVAILABLE = -4,
    NO_CONST_VALUE_AVAILABLE = -5,
    NO_VALUES_AVAILABLE = -6,
    NO_SINGLE_VALUE_AVAILABLE = -7,
    INSUFFICIENT_NUM_VALUES = -8,
    TOO_MANY_VALUES = -9,
    UNKNOWN_ARG = -10,
    INVALID_ARG_TYPE
};



/*-----------------------------------------------------------------------------
 * Command-line argument parser
-----------------------------------------------------------------------------*/
class ArgParser
{
  private:
    std::unordered_map<size_t, size_t> mLongOptToIndices;

    std::unordered_map<size_t, size_t> mShortOptToIndices;

    std::vector<Argument> mArgs;

    std::vector<bool> mFoundOpts;

    std::vector<std::vector<std::string>> mValues;

    std::string mMainFile;

    [[noreturn]]
    void _print_help_and_quit(
        const std::vector<argparse::Argument>& args,
        argparse::ArgErrCode errCode = argparse::ArgErrCode::SUCCESS) const noexcept;

    [[noreturn]]
    void _print_err_and_quit(
        const std::vector<argparse::Argument>& args,
        const std::string& errMsg,
        argparse::ArgErrCode errCode) const noexcept;

    void _validate_arg_counts() const noexcept;

    void _validate_args(int argc, char* const* argv) const noexcept;

    int _parse_long_opt(const std::string& currentOpt, int argId, int argc, char* const* argv) noexcept;

    int _parse_short_opts(const char* pFlags, char lastFlag, int argId, int argc, char* const* argv) noexcept;

  public:
    ~ArgParser() noexcept;

    ArgParser() noexcept;

    ArgParser(const ArgParser& parser) noexcept;

    ArgParser(ArgParser&& parser) noexcept;

    ArgParser& operator=(const ArgParser& parser) noexcept;

    ArgParser& operator=(ArgParser&& parser) noexcept;

    Argument& set_argument(char shortName) noexcept;

    Argument& set_argument(const std::string& longName) noexcept;

    Argument& set_argument(const std::string& longName, char shortName) noexcept;

    bool parse(int argc, char* const* argv) noexcept;

    const std::string& main_file_path() const noexcept;

    bool value_exists(const std::string& longName) const noexcept;

    bool value_exists(char shortName) const noexcept;

    const std::vector<std::string>& value(const std::string& longName) const;

    const std::vector<std::string>& value(char shortName) const;

    long long int value_as_int(const std::string& longName) const;

    long long int value_as_int(char shortName) const;

    char value_as_char(const std::string& longName) const;

    char value_as_char(char shortName) const;

    double value_as_real(const std::string& longName) const;

    double value_as_real(char shortName) const;

    const std::string& value_as_string(const std::string& longName) const;

    const std::string& value_as_string(char shortName) const;

    std::vector<long long int> value_as_ints(const std::string& longName) const;

    std::vector<long long int> value_as_ints(char shortName) const;

    std::vector<char> value_as_chars(const std::string& longName) const;

    std::vector<char> value_as_chars(char shortName) const;

    std::vector<double> value_as_reals(const std::string& longName) const;

    std::vector<double> value_as_reals(char shortName) const;

    const std::vector<std::string>& value_as_strings(const std::string& longName) const;

    const std::vector<std::string>& value_as_strings(char shortName) const;
};



/*-------------------------------------
 * Get the first string argument referenced by its command-line name
-------------------------------------*/
inline const std::string& ArgParser::value_as_string(const std::string& longName) const
{
    return this->value(longName)[0];
}



/*-------------------------------------
 * Get the first string argument referenced by its command-line name
-------------------------------------*/
inline const std::string& ArgParser::value_as_string(char shortName) const
{
    return this->value(shortName)[0];
}



/*-------------------------------------
 * Get the list of string argument referenced by their command-line name
-------------------------------------*/
inline const std::vector<std::string>& ArgParser::value_as_strings(const std::string& longName) const
{
    return this->value(longName);
}



/*-------------------------------------
 * Get the list of string argument referenced by their command-line name
-------------------------------------*/
inline const std::vector<std::string>& ArgParser::value_as_strings(char shortName) const
{
    return this->value(shortName);
}



} // end argparse namespace
} // end utils namespace
} // end ls namespace

#endif /* ARGPARSE_ARG_PARSER_HPP */
