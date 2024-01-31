
#include <cctype> // std::isdigit()
#include <cstdio> // std::sscanf
#include <cstdlib> // std::exit()
#include <cstring> // std::strlen
#include <iostream>
#include <limits>
#include <utility> // std::move()

#include "lightsky/utils/ArgParser.hpp"
#include "lightsky/utils/Argument.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
namespace
{

namespace argparse = ls::utils::argparse;



/*-------------------------------------
 *
-------------------------------------*/
enum class ArgUsage
{
    SHORT_ARG,
    SHORT_FLAGS,
    LONG_ARG,
    PARAMETER
};



/*-------------------------------------
 * Check if a value is an integer
-------------------------------------*/
bool param_is_integral(const char* pOpt) noexcept
{
    // negativity check
    if (pOpt[0] == '-')
    {
        ++pOpt;
    }

    while (*pOpt)
    {
        if (!std::isdigit(*pOpt))
        {
            return false;
        }

        ++pOpt;
    }

    return true;
}



/*-------------------------------------
 * Check if a value is a floating-point type
-------------------------------------*/
bool param_is_real(const char* pOpt) noexcept
{
    int len;
    double ignore;
    int ret = std::sscanf(pOpt, "%lf %n", &ignore, &len);
    return ret && len == (int)std::strlen(pOpt);
}



/*-------------------------------------
 * Validate data types
-------------------------------------*/
bool param_matches_type(const char* pOpt, argparse::ArgType type) noexcept
{
    bool ret = false;

    switch (type)
    {
        case argparse::ArgType::STRING:
            ret = true;
            break;

        case argparse::ArgType::CHAR:
            ret = strlen(pOpt) == 1;
            break;

        case argparse::ArgType::INTEGRAL:
            ret = param_is_integral(pOpt);
            break;

        case argparse::ArgType::REAL:
            ret = param_is_real(pOpt);
            break;

        default:
            break;
    }

    return ret;
}



/*-------------------------------------
 * Load a single argument
-------------------------------------*/
ArgUsage parse_arg_type(const char* pOpt, int* numFlags = nullptr) noexcept
{
    size_t len = std::strlen(pOpt);

    if (len >= 3 && pOpt[0] == '-' && pOpt[1] == '-')
    {
        if (numFlags)
        {
            *numFlags = 1;
        }
        return ArgUsage::LONG_ARG;
    }
    else if (len >= 2 && pOpt[0] == '-')
    {
        const size_t numSubOpts = std::strlen(pOpt+1);
        if (numFlags)
        {
            *numFlags = (int)(numSubOpts < (size_t)std::numeric_limits<int>::max() ? numSubOpts : (size_t)std::numeric_limits<int>::max());
        }
        return (numSubOpts > 1) ? ArgUsage::SHORT_FLAGS : ArgUsage::SHORT_ARG;
    }

    if (numFlags)
    {
        *numFlags = 1;
    }
    return ArgUsage::PARAMETER;
}



/*-------------------------------------
 * Get the description of a type
-------------------------------------*/
std::string param_type_str(argparse::Argument arg) noexcept
{
    argparse::ArgType type = arg.type();

    if (arg.num_required() > 0)
    {
        switch (type)
        {
            case argparse::ArgType::STRING:   return std::string{"string"};
            case argparse::ArgType::CHAR:     return std::string{"char"};
            case argparse::ArgType::INTEGRAL: return std::string{"integral"};
            case argparse::ArgType::REAL:     return std::string{"floating-point"};
        }
    }

    return std::string{"flag"};
}



/*-------------------------------------
 * Build a help message and quit
-------------------------------------*/
[[noreturn]]
void print_help_and_quit(
    const std::vector<argparse::Argument>& args,
    argparse::ArgErrCode errCode = argparse::ArgErrCode::SUCCESS) noexcept
{
    for (const argparse::Argument& arg : args)
    {
        const char argShortName = arg.short_name();
        const std::string& argLongName = arg.long_name();

        if (!arg.description().empty())
        {
            std::cout << arg.description() << ": ";
        }

        if (!arg.required())
        {
            std::cout << '[';
        }

        if (!argLongName.empty() && argShortName != '\0')
        {
            std::cout << "--" << argLongName << " / -" << argShortName;
        }
        else if (argLongName.empty() && argShortName != '\0')
        {
            std::cout << '-' << argShortName;
        }
        if (!argLongName.empty() && argShortName == '\0')
        {
            std::cout<< "--" << argLongName;
        }

        if (arg.num_required() == 1)
        {
            std::cout << " value";
        }
        else if (arg.num_required() > 1)
        {
            const size_t numConstVals = arg.const_value().size();
            if (numConstVals == 0)
            {
                std::cout << " [value1[ value2[ ...]]]";
            }
            else
            {
                for (size_t v = 0; v < numConstVals; ++v)
                {
                    const char* pluralism = (v == 0) ? " [" : " ";
                    std::cout << pluralism << arg.const_value()[v];
                }

                std::cout << ']';
            }
        }

        if (!arg.required())
        {
            std::cout << ']';
        }

        std::cout << "\n\tType: " << param_type_str(arg);

        if (arg.num_required() >= 1 && !arg.default_value().empty())
        {
            const size_t numDefaultVals = arg.default_value().size();
            const bool haveMultiValues = numDefaultVals > 1;
            const char* pluralism = haveMultiValues ? "s: " : ": ";

            std::cout << "\n\tDefault value" << pluralism;

            for (size_t v = 0; v < numDefaultVals; ++v)
            {
                std::cout << ((v == 0) ? '[' : ' ') << arg.default_value()[v];
            }

            std::cout << ']';
        }

        if (!arg.help_text().empty())
        {
            std::cout << "\n\t" << arg.help_text();
        }

        std::cout << '\n' << std::endl;
    }

    std::exit(static_cast<int>(errCode));
}



/*-------------------------------------
 * Build an error message, print help, then quit
-------------------------------------*/
[[noreturn]]
void print_err_and_quit(
    const std::vector<argparse::Argument>& args,
    const std::string& errMsg,
    argparse::ArgErrCode errCode) noexcept
{
    std::cerr << errMsg << std::endl;
    print_help_and_quit(args, errCode);
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * Argument Parser
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{
namespace argparse
{

/*-------------------------------------
 * Destructor
-------------------------------------*/
ArgParser::~ArgParser() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
ArgParser::ArgParser() noexcept :
    mLongOptToIndices{},
    mShortOptToIndices{},
    mArgs{},
    mFoundOpts{},
    mValues{},
    mMainFile{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
ArgParser::ArgParser(const ArgParser& parser) noexcept :
    mLongOptToIndices{parser.mLongOptToIndices},
    mShortOptToIndices{parser.mShortOptToIndices},
    mArgs{parser.mArgs},
    mFoundOpts{parser.mFoundOpts},
    mValues{parser.mValues},
    mMainFile{parser.mMainFile}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
ArgParser::ArgParser(ArgParser&& parser) noexcept :
    mLongOptToIndices{std::move(parser.mLongOptToIndices)},
    mShortOptToIndices{std::move(parser.mShortOptToIndices)},
    mArgs{std::move(parser.mArgs)},
    mFoundOpts{std::move(parser.mFoundOpts)},
    mValues{std::move(parser.mValues)},
    mMainFile{std::move(parser.mMainFile)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
ArgParser& ArgParser::operator=(const ArgParser& parser) noexcept
{
    mLongOptToIndices = parser.mLongOptToIndices;
    mShortOptToIndices = parser.mShortOptToIndices;
    mArgs = parser.mArgs;
    mFoundOpts = parser.mFoundOpts;
    mValues = parser.mValues;
    mMainFile = parser.mMainFile;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
ArgParser& ArgParser::operator=(ArgParser&& parser) noexcept
{
    mLongOptToIndices = std::move(parser.mLongOptToIndices);
    mShortOptToIndices = std::move(parser.mShortOptToIndices);
    mArgs = std::move(parser.mArgs);
    mFoundOpts = std::move(parser.mFoundOpts);
    mValues = std::move(parser.mValues);
    mMainFile = std::move(parser.mMainFile);

    return *this;
}



/*-------------------------------------
 * Assign an argument by name
-------------------------------------*/
Argument& ArgParser::set_argument(char shortName) noexcept
{
    size_t hash = Argument::hash_for_name("", shortName);
    size_t idx = ~(size_t)0;

    if (shortName && mShortOptToIndices.count(hash))
    {
        idx = mShortOptToIndices[hash];
    }
    else
    {
        mArgs.emplace_back(Argument{"", shortName});
        idx = mArgs.size()-1;

        // Ensure even a null option gets used.
        mLongOptToIndices[hash] = idx;
        mShortOptToIndices[shortName] = idx;
    }

    return mArgs[idx];
}



/*-------------------------------------
 * Assign an argument by name
-------------------------------------*/
Argument& ArgParser::set_argument(const std::string& longName) noexcept
{
    size_t hash = Argument::hash_for_name(longName, '\0');
    size_t idx = ~(size_t)0;

    if (!longName.empty() && mLongOptToIndices.count(hash))
    {
        idx = mLongOptToIndices[hash];
    }
    else
    {
        mArgs.emplace_back(Argument{longName, '\0'});
        idx = mArgs.size()-1;

        // Ensure even a null option gets used.
        mLongOptToIndices[hash] = idx;
    }

    return mArgs[idx];
}



/*-------------------------------------
 * Assign an argument by name
-------------------------------------*/
Argument& ArgParser::set_argument(const std::string &longName, char shortName) noexcept
{
    size_t hash = Argument::hash_for_name(longName, shortName);
    size_t idx = ~(size_t)0;

    if (!longName.empty() && mLongOptToIndices.count(hash))
    {
        idx = mLongOptToIndices[hash];
    }
    else if (shortName && mShortOptToIndices.count(hash))
    {
        idx = mShortOptToIndices[hash];
    }
    else
    {
        mArgs.emplace_back(Argument{longName, shortName});
        idx = mArgs.size()-1;

        // Ensure even a null option gets used.
        if (!longName.empty())
        {
            mLongOptToIndices[hash] = idx;
        }

        if (shortName)
        {
            mShortOptToIndices[shortName] = idx;
        }
    }

    return mArgs[idx];
}



/*-------------------------------------
 * Arg Count validation
-------------------------------------*/
void ArgParser::_validate_arg_counts() const noexcept
{
    for (const Argument& arg : mArgs)
    {
        if (arg.const_value().empty() && (!arg.required() || arg.num_required() != static_cast<size_t>(ArgCount::LEAST_ONE)))
        {
            continue;
        }

        std::vector<std::string>::size_type constSize = arg.const_value().size();
        bool constValidation0 = (constSize == 0 && arg.num_required() == static_cast<size_t>(ArgCount::ZERO));
        bool constValidation1 = (constSize == 1 && arg.num_required() == static_cast<size_t>(ArgCount::ONE));
        bool constValidation2 = (constSize >= 1 && arg.num_required() == static_cast<size_t>(ArgCount::LEAST_ONE));
        bool constValidation3 = (constSize == arg.num_required());

        if (!(constValidation0 || constValidation1 || constValidation2 || constValidation3))
        {
            print_err_and_quit(
                mArgs,
                "Internal error: Constant argument count does not match number of required arguments.",
                ArgErrCode::INTERNAL_CONST_ARG_COUNT_MISMATCH);
        }
        else
        {
            for (const std::string& argN : arg.const_value())
            {
                if (!param_matches_type(argN.c_str(), arg.type()))
                {
                    std::string&& err = std::string{"Const Value element \""};
                    err += argN;
                    err += "\" within \"";
                    err += arg.long_name();
                    err += std::string{"\" does not match expected data type: "};
                    err += param_type_str(arg);

                    print_err_and_quit(mArgs, err, ArgErrCode::INTERNAL_CONST_ARG_TYPE_MISMATCH);
                }
            }
        }
    }
}



/*-------------------------------------
 * Validate arg values
-------------------------------------*/
void ArgParser::_validate_args(int argc, char* const* argv) const noexcept
{
    const char* pOpt = nullptr;
    int i = 1;
    int numFlags = 0;
    std::string currentOpt;
    char currentFlag = '\0';
    size_t currentHash = ~(size_t)0;
    size_t currentArgIndex = ~(size_t)0;
    size_t numArgsForOpt = 0;

    const auto&& param_validator = [&]()->void
    {
        const Argument& arg = mArgs[currentArgIndex];

        if (arg.num_required() && !numArgsForOpt && arg.const_value().empty())
        {
            const std::string msgParam = !currentOpt.empty() ? currentOpt : std::string{currentFlag};
            print_err_and_quit(
                mArgs,
                std::string{"No parameters provided for argument \""} + msgParam + '\"',
                ArgErrCode::NO_VALUES_AVAILABLE);
        }

        if (arg.num_required() == static_cast<size_t>(ArgCount::LEAST_ONE) && !numArgsForOpt && arg.const_value().empty())
        {
            const std::string msgParam = !currentOpt.empty() ? currentOpt : std::string{currentFlag};
            print_err_and_quit(
                mArgs,
                std::string{"Argument \""} + msgParam + std::string{"\" requires at least one parameter."},
                ArgErrCode::NO_SINGLE_VALUE_AVAILABLE);
        }
        else if (arg.num_required() != static_cast<size_t>(ArgCount::LEAST_ONE) && arg.num_required() > numArgsForOpt)
        {
            const std::string msgParam = !currentOpt.empty() ? currentOpt : std::string{currentFlag};
            std::string err = "Insufficient parameters for argument \"";
            err.append(msgParam);
            err.append(".\" Have ");
            err.append(std::to_string(numArgsForOpt));
            err.append(" of ");
            err.append(std::to_string(arg.num_required()));
            err.append(" parameters.");
            print_err_and_quit(mArgs, err, ArgErrCode::INSUFFICIENT_NUM_VALUES);
        }
        else if (arg.num_required() < numArgsForOpt)
        {
            const std::string msgParam = !currentOpt.empty() ? currentOpt : std::string{currentFlag};
            std::string err = "Too many parameters for argument \"";
            err.append(msgParam);
            err.append(".\" Have ");
            err.append(std::to_string(numArgsForOpt));
            err.append(" of ");
            err.append(std::to_string(arg.num_required()));
            err.append(" parameters.");
            print_err_and_quit(mArgs, err, ArgErrCode::TOO_MANY_VALUES);
        }
    };

    while (i < argc)
    {
        pOpt = argv[i];

        ArgUsage usage = parse_arg_type(pOpt, &numFlags);

        // If the current argument changed, validate the last set of arguments
        if (currentHash != ~(size_t)0 && usage != ArgUsage::PARAMETER)
        {
            currentArgIndex = currentOpt.empty() ? mShortOptToIndices.at(currentHash) : mLongOptToIndices.at(currentHash);
            param_validator();
        }

        // Retrieve data & validate
        if (usage == ArgUsage::LONG_ARG)
        {
            pOpt += 2; // skip the '--' prefix

            numArgsForOpt = 0;
            currentOpt = pOpt;
            currentFlag = '\0';
            currentHash = Argument::hash_for_name(currentOpt);

            if (currentOpt == "help")
            {
                print_help_and_quit(mArgs);
            }

            if (!mLongOptToIndices.count(currentHash))
            {
                print_err_and_quit(
                    mArgs,
                    std::string{"Unknown option: "} + currentOpt,
                    ArgErrCode::UNKNOWN_ARG);
            }
        }
        else if (usage == ArgUsage::SHORT_ARG || usage == ArgUsage::SHORT_FLAGS)
        {
            pOpt += 1; // skip the '-' prefix

            numArgsForOpt = 0;
            currentOpt.clear();

            for (int j = 0; j < numFlags; ++j)
            {
                currentFlag = pOpt[j];
                currentHash = Argument::hash_for_name(pOpt[j]);

                if (pOpt[j] == 'h')
                {
                    print_help_and_quit(mArgs);
                }

                if (!mShortOptToIndices.count(currentHash))
                {
                    print_err_and_quit(
                        mArgs,
                        std::string{"Unknown option: "} + pOpt[j],
                        ArgErrCode::UNKNOWN_ARG);
                }

                // validate concatenated short options
                if (j < numFlags-1)
                {
                    currentArgIndex = mShortOptToIndices.at(currentHash);
                    param_validator();
                }
            }
        }
        else
        {
            if (currentHash == ~(size_t)0)
            {
                print_err_and_quit(
                    mArgs,
                    std::string{"Unknown option: "} + currentOpt,
                    ArgErrCode::UNKNOWN_ARG);
            }

            const size_t lastArgIndex = currentOpt.empty()
                ? mShortOptToIndices.at(currentHash)
                : mLongOptToIndices.at(currentHash);
            const Argument& arg = mArgs[lastArgIndex];

            if (!param_matches_type(pOpt, arg.type()))
            {
                std::string&& err = std::string{"Parameter \""};
                err += pOpt;
                err += std::string{"\" does not match expected type: "};
                err += param_type_str(arg);

                print_err_and_quit(mArgs, err, ArgErrCode::INVALID_ARG_TYPE);
            }

            ++numArgsForOpt;
        }

        ++i;
    }

    // validate the final set of arguments
    if (currentHash != ~(size_t)0)
    {
        currentArgIndex = currentOpt.empty()
            ? mShortOptToIndices.at(currentHash)
            : mLongOptToIndices.at(currentHash);

        param_validator();
    }
}



/*-------------------------------------
 * Load arguments usign their long names
-------------------------------------*/
int ArgParser::_parse_long_opt(const std::string& currentOpt, int argId, int argc, char* const* argv) noexcept
{
    int numFlags = 0;
    int i = argId+1;
    const size_t idx = mLongOptToIndices.at(Argument::hash_for_name(currentOpt));

    mFoundOpts[idx] = true;
    mValues[idx].clear();

    for (; i < argc; ++i)
    {
        const char* pOpt = argv[i];
        ArgUsage usage = parse_arg_type(pOpt, &numFlags);

        if (usage != ArgUsage::PARAMETER)
        {
            break;
        }

        mValues[idx].emplace_back(std::string{pOpt});
    }

    return i-argId;
}



/*-------------------------------------
 * Get arguments using their short names
-------------------------------------*/
int ArgParser::_parse_short_opts(const char* pFlags, char lastFlag, int argId, int argc, char* const* argv) noexcept
{
    int numFlags = 0;
    int i = argId+1;
    size_t idx = 0;

    for (const char* pFlagIter = pFlags; *pFlagIter; ++pFlagIter)
    {
        idx = mShortOptToIndices.at(Argument::hash_for_name(*pFlagIter));
        mFoundOpts[idx] = true;
        mValues[idx].clear();
    }

    idx = mShortOptToIndices.at(Argument::hash_for_name(lastFlag));
    mFoundOpts[idx] = true;
    mValues[idx].clear();

    for (; i < argc; ++i)
    {
        const char* pOpt = argv[i];
        ArgUsage usage = parse_arg_type(pOpt, &numFlags);

        if (usage != ArgUsage::PARAMETER)
        {
            break;
        }

        mValues[idx].emplace_back(std::string{pOpt});
    }

    return i-argId;
}


/*-------------------------------------
 * Parse data
-------------------------------------*/
bool ArgParser::parse(int argc, char* const*argv) noexcept
{
    mFoundOpts.clear();
    mFoundOpts.resize(mArgs.size(), false);

    mValues.clear();
    mValues.resize(mArgs.size(), std::vector<std::string>{});

    _validate_arg_counts();
    _validate_args(argc, argv);

    const char* pOpt = nullptr;
    int i = 1;
    int numFlags = 0;
    char currentFlag = '\0';
    std::string currentOpt;

    while (i < argc)
    {
        pOpt = argv[i];

        ArgUsage usage = parse_arg_type(pOpt, &numFlags);
        switch (usage)
        {
            case ArgUsage::LONG_ARG:
                pOpt += 2;
                currentFlag = '\0';
                currentOpt = pOpt;
                i += _parse_long_opt(currentOpt, i, argc, argv);
                break;

            case ArgUsage::SHORT_FLAGS:
            case ArgUsage::SHORT_ARG:
                pOpt += 1;
                currentFlag = pOpt[numFlags-1];
                currentOpt.clear();
                i += _parse_short_opts(pOpt, currentFlag, i, argc, argv);
                break;

            // Should be skipped anyway
            case ArgUsage::PARAMETER:
            default:
                ++i;
                break;
        }
    }

    for (const Argument& arg : mArgs)
    {
        const size_t hash = Argument::hash_for_name(arg.long_name());
        const size_t idx = mLongOptToIndices.count(hash)
            ? mLongOptToIndices.at(hash)
            : mShortOptToIndices.at(arg.short_name());

        if (!mFoundOpts[idx])
        {
            if (arg.required() && arg.num_required() != 0 && arg.default_value().empty())
            {
                print_err_and_quit(
                    mArgs,
                    std::string{"No default value provided for argument \""} + arg.long_name() + "\".",
                    ArgErrCode::NO_DEFAULT_VALUE_AVAILABLE);
            }

            mValues[idx] = arg.default_value();
        }
        else
        {
            if (mValues[idx].empty())
            {
                if (arg.required() && arg.const_value().empty())
                {
                    print_err_and_quit(
                        mArgs,
                        std::string{"No const value provided for argument \""} + arg.long_name() + "\".",
                        ArgErrCode::NO_CONST_VALUE_AVAILABLE);
                }

                mValues[idx] = arg.const_value();
            }
        }
    }

    mMainFile = argv[0];
    return true;
}



/*-------------------------------------
 * Main file name
-------------------------------------*/
const std::string& ArgParser::main_file_path() const noexcept
{
    return mMainFile;
}



/*-------------------------------------
 * Check if a value was set (long version)
-------------------------------------*/
bool ArgParser::value_exists(const std::string& longName) const noexcept
{
    size_t hash = Argument::hash_for_name(longName);
    if (!mLongOptToIndices.count(hash))
    {
        return false;
    }

    size_t idx = mLongOptToIndices.at(hash);
    return mFoundOpts[idx];
}



/*-------------------------------------
 * Check if a value was set (short version)
-------------------------------------*/
bool ArgParser::value_exists(char shortName) const noexcept
{
    if (!mShortOptToIndices.count(shortName))
    {
        return false;
    }

    size_t idx = mShortOptToIndices.at(shortName);
    return mFoundOpts[idx];
}



/*-------------------------------------
 * Get a value as a string
-------------------------------------*/
const std::vector<std::string>& ArgParser::value(const std::string& longName) const
{
    size_t hash = Argument::hash_for_name(longName);
    size_t idx = mLongOptToIndices.at(hash);
    return mValues[idx];
}



/*-------------------------------------
 * Get a value as a string
-------------------------------------*/
const std::vector<std::string>& ArgParser::value(char shortName) const
{
    size_t hash = Argument::hash_for_name(shortName);
    size_t idx = mShortOptToIndices.at(hash);
    return mValues[idx];
}


/*-------------------------------------
 * Get the first integral argument referenced by its command-line name
-------------------------------------*/
long long int ArgParser::value_as_int(const std::string& longName) const
{
    const std::string& argVal = this->value_as_string(longName);
    return std::atoll(argVal.c_str());
}



/*-------------------------------------
 * Get the first integral argument referenced by its command-line name
-------------------------------------*/
long long int ArgParser::value_as_int(char shortName) const
{
    const std::string& argVal = this->value_as_string(shortName);
    return std::atoll(argVal.c_str());
}



/*-------------------------------------
 * Get the first character argument referenced by its command-line name
-------------------------------------*/
char ArgParser::value_as_char(const std::string& longName) const
{
    const std::string& argVal = this->value_as_string(longName);
    return (char)std::atoi(argVal.c_str());
}



/*-------------------------------------
 * Get the first character argument referenced by its command-line name
-------------------------------------*/
char ArgParser::value_as_char(char shortName) const
{
    const std::string& argVal = this->value_as_string(shortName);
    return (char)std::atoi(argVal.c_str());
}



/*-------------------------------------
 * Get the first character argument referenced by its command-line name
-------------------------------------*/
double ArgParser::value_as_real(const std::string& longName) const
{
    const std::string& argVal = this->value_as_string(longName);
    return std::atof(argVal.c_str());
}



/*-------------------------------------
 * Get the first character argument referenced by its command-line name
-------------------------------------*/
double ArgParser::value_as_real(char shortName) const
{
    const std::string& argVal = this->value_as_string(shortName);
    return std::atof(argVal.c_str());
}



/*-------------------------------------
 * Get the list of all integral arguments referenced by their command-line name
-------------------------------------*/
std::vector<long long int> ArgParser::value_as_ints(const std::string& longName) const
{
    const std::vector<std::string>& argVals = this->value_as_strings(longName);
    std::vector<long long int> results;
    size_t iter = 0;

    results.resize(argVals.size());
    for (const std::string& argVal : argVals)
    {
        results[iter++] = std::atoll(argVal.c_str());
    }

    return results;
}



/*-------------------------------------
 * Get the list of all integral arguments referenced by their command-line name
-------------------------------------*/
std::vector<long long int> ArgParser::value_as_ints(char shortName) const
{
    const std::vector<std::string>& argVals = this->value_as_strings(shortName);
    std::vector<long long int> results;
    size_t iter = 0;

    results.resize(argVals.size());
    for (const std::string& argVal : argVals)
    {
        results[iter++] = std::atoll(argVal.c_str());
    }

    return results;
}



/*-------------------------------------
 * Get the list of all character arguments referenced by their command-line name
-------------------------------------*/
std::vector<char> ArgParser::value_as_chars(const std::string& longName) const
{
    const std::vector<std::string>& argVals = this->value_as_strings(longName);
    std::vector<char> results;
    size_t iter = 0;

    results.resize(argVals.size());
    for (const std::string& argVal : argVals)
    {
        results[iter++] = (char)std::atoi(argVal.c_str());
    }

    return results;
}



/*-------------------------------------
 * Get the list of all character arguments referenced by their command-line name
-------------------------------------*/
std::vector<char> ArgParser::value_as_chars(char shortName) const
{
    const std::vector<std::string>& argVals = this->value_as_strings(shortName);
    std::vector<char> results;
    size_t iter = 0;

    results.resize(argVals.size());
    for (const std::string& argVal : argVals)
    {
        results[iter++] = (char)std::atoi(argVal.c_str());
    }

    return results;
}



/*-------------------------------------
 * Get the list of all float arguments referenced by their command-line name
-------------------------------------*/
std::vector<double> ArgParser::value_as_reals(const std::string& longName) const
{
    const std::vector<std::string>& argVals = this->value_as_strings(longName);
    std::vector<double> results;
    size_t iter = 0;

    results.resize(argVals.size());
    for (const std::string& argVal : argVals)
    {
        results[iter++] = std::atof(argVal.c_str());
    }

    return results;
}



/*-------------------------------------
 * Get the list of all float arguments referenced by their command-line name
-------------------------------------*/
std::vector<double> ArgParser::value_as_reals(char shortName) const
{
    const std::vector<std::string>& argVals = this->value_as_strings(shortName);
    std::vector<double> results;
    size_t iter = 0;

    results.resize(argVals.size());
    for (const std::string& argVal : argVals)
    {
        results[iter++] = std::atof(argVal.c_str());
    }

    return results;
}


} // end argparse namespace
} // end utils namespace
} // end ls namespace