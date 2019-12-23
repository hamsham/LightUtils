
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

class Argument;



class ArgParser
{
  private:
    std::unordered_map<size_t, size_t> mLongOptToIndices;

    std::unordered_map<char, size_t> mShortOptToIndices;

    std::vector<Argument> mArgs;

    std::vector<bool> mFoundOpts;

    std::vector<std::vector<std::string>> mValues;

    std::string mMainFile;

    void validate_internal() const noexcept;

    void validate_args(int argc, char* const* argv) const noexcept;

    int parse_long_opt(const std::string& currentOpt, int argId, int argc, char* const* argv) noexcept;

    int parse_short_opts(const char* pFlags, char lastFlag, int argId, int argc, char* const* argv) noexcept;

  public:
    ~ArgParser() noexcept;

    ArgParser() noexcept;

    ArgParser(const ArgParser& parser) noexcept;

    ArgParser(ArgParser&& parser) noexcept;

    ArgParser& operator=(const ArgParser& parser) noexcept;

    ArgParser& operator=(ArgParser&& parser) noexcept;

    Argument& set_argument(const std::string& longName, char shortName = '\0') noexcept;

    bool parse(int argc, char* const* argv) noexcept;

    bool have_value(const std::string& longName) const noexcept;

    bool have_value(char shortName) const noexcept;

    const std::vector<std::string>& value(const std::string& longName) const;

    const std::vector<std::string>& value(char shortName) const;

    const std::string& main_file_path() const noexcept;
};



/*-------------------------------------
 *
-------------------------------------*/
inline const std::string& ArgParser::main_file_path() const noexcept
{
    return mMainFile;
}



} // end argparse namespace
} // end utils namespace
} // end ls namespace

#endif /* ARGPARSE_ARG_PARSER_HPP */
