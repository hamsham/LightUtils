
#ifndef ARGPARSE_ARGUMENT_HPP
#define ARGPARSE_ARGUMENT_HPP

#include <string>
#include <vector>

namespace ls
{
namespace utils
{
namespace argparse
{



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
enum class ArgCount : size_t
{
    LEAST_ONE = ~(size_t)0,
    ZERO = 0,
    ONE = 1,
};



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
enum class ArgType
{
    STRING,
    CHAR,
    INTEGRAL,
    REAL
};



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class Argument
{
  private:
    std::string mLongOpt;

    char mShortOpt;

    std::vector<std::string> mDefaultVal;

    std::vector<std::string> mConstVal;

    std::string mDescription;

    std::string mHelp;

    ArgType mType;

    size_t mNumArgs;

    bool mRequired;

    template <typename StringableArg>
    static void value_push(std::vector<std::string>& argList, const StringableArg& defaultVal) noexcept;

    template <typename StringableArg, typename... StringableArgs>
    static void value_push(std::vector<std::string>& argList, const StringableArg& defaultVal, const StringableArgs&... defaultVals) noexcept;

  public:
    Argument() = delete;

    Argument(const std::string& longOpt, char shortOpt = '\0') noexcept;

    Argument(const Argument& arg) noexcept;

    Argument(Argument&& arg) noexcept;

    Argument& operator=(const Argument& arg) noexcept;

    Argument& operator=(Argument&& arg) noexcept;

    const std::string& long_name() const noexcept;

    char short_name() const noexcept;

    size_t hash() const noexcept;

    static size_t hash_for_name(const std::string& longName) noexcept;

    static size_t hash_for_name(char shortName) noexcept;

    static size_t hash_for_name(const std::string& longName, char shortName) noexcept;

    template <typename... StringableArgs>
    Argument& default_value(const StringableArgs&... defaultVals) noexcept;

    template <typename StringableArg>
    Argument& default_value(const StringableArg& defaultVal) noexcept;

    Argument& default_value(const char* defaultVal) noexcept;

    const std::vector<std::string>& default_value() const noexcept;

    template <typename... StringableArgs>
    Argument& const_value(const StringableArgs&... defaultVals) noexcept;

    template <typename StringableArg>
    Argument& const_value(const StringableArg& defaultVal) noexcept;

    Argument& const_value(const char* defaultVal) noexcept;

    const std::vector<std::string>& const_value() const noexcept;

    Argument& description(const std::string& shortDesc) noexcept;

    const std::string& description() const noexcept;

    Argument& help_text(const std::string& help) noexcept;

    const std::string& help_text() const noexcept;

    Argument& num_required(ArgCount numParams) noexcept;

    size_t num_required() const noexcept;

    Argument& num_required(size_t numParams) noexcept;

    Argument& required(bool isRequired) noexcept;

    bool required() const noexcept;

    Argument& type(ArgType dataType) noexcept;

    ArgType type() const noexcept;

};



/*-------------------------------------
 *
-------------------------------------*/
inline const std::string& Argument::long_name() const noexcept
{
    return mLongOpt;
}



/*-------------------------------------
 *
-------------------------------------*/
inline char Argument::short_name() const noexcept
{
    return mShortOpt;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename StringableArg>
void Argument::value_push(std::vector<std::string>& argList, const StringableArg& defaultVal) noexcept
{
    argList.emplace_back(std::to_string(defaultVal));
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename StringableArg, typename... StringableArgs>
void Argument::value_push(std::vector<std::string>& argList, const StringableArg& defaultVal, const StringableArgs&... defaultVals) noexcept
{
    argList.emplace_back(std::to_string(defaultVal));
    Argument::value_push<StringableArgs...>(argList, defaultVals...);
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename... StringableArgs>
Argument& Argument::default_value(const StringableArgs&... defaultVals) noexcept
{
    mDefaultVal.clear();
    Argument::value_push<StringableArgs...>(mDefaultVal, defaultVals...);
    mRequired = true;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename StringableArg>
Argument& Argument::default_value(const StringableArg& defaultVal) noexcept
{
    mDefaultVal.clear();
    Argument::value_push<StringableArg>(mDefaultVal, defaultVal);
    mRequired = true;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename... StringableArgs>
Argument& Argument::const_value(const StringableArgs&... constVals) noexcept
{
    mConstVal.clear();
    Argument::value_push<StringableArgs...>(mConstVal, constVals...);
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename StringableArg>
Argument& Argument::const_value(const StringableArg& constVal) noexcept
{
    mConstVal.clear();
    Argument::value_push<StringableArg>(mConstVal, constVal);
    return *this;
}



} // end argparse namespace
} // end utils namespace
} // end ls namespace



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
namespace std
{
    template <>
    struct hash<ls::utils::argparse::Argument>;
}



#endif /* ARGPARSE_ARGUMENT_HPP */
