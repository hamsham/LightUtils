
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
 *
-------------------------------------*/
bool param_is_integral(const char* pOpt) noexcept
{
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
 *
-------------------------------------*/
bool param_is_real(const char* pOpt) noexcept
{
    int len;
    double ignore;
    int ret = std::sscanf(pOpt, "%lf %n", &ignore, &len);
    return ret && len == (int)std::strlen(pOpt);
}



/*-------------------------------------
 *
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
 *
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
 *
-------------------------------------*/
std::string param_type_str(argparse::ArgType type) noexcept
{
    switch (type)
    {
        case argparse::ArgType::STRING:
            break;

        case argparse::ArgType::CHAR:
            return std::string{"char"};

        case argparse::ArgType::INTEGRAL:
            return std::string{"integral"};

        case argparse::ArgType::REAL:
            return std::string{"floating-point"};

        default:
            break;
    }

    return std::string{"string"};
}



/*-------------------------------------
 *
-------------------------------------*/
[[noreturn]]
void print_help_and_quit(const std::vector<argparse::Argument>& args, int errCode = 0) noexcept
{
    for (const argparse::Argument& arg : args)
    {
        if (!arg.description().empty())
        {
            std::cout << arg.description() << ": ";
        }

        if (!arg.required())
        {
            std::cout << "[ ";
        }

        if (!arg.long_name().empty() && arg.short_name() != '\0') {
            std::cout << "--" << arg.long_name() << " / -" << arg.short_name();
        }
        else if (arg.long_name().empty() && arg.short_name() != '\0') {
            std::cout << '-' << arg.short_name();
        }
        if (!arg.long_name().empty() && arg.short_name() == '\0') {
            std::cout<< "--" << arg.long_name();
        }

        if (!arg.required())
        {
            std::cout << " ]";
        }

        if (arg.num_required() == 1)
        {
            std::cout << " [ param ]";
        }
        else if (arg.num_required() == static_cast<size_t>(argparse::ArgCount::LEAST_ONE))
        {
            std::cout << " [ param_1 [ param_2 [ ... ] ]";
            if (!arg.const_value().empty())
            {
                std::cout << "\n\tDefault Value[s] (if flag enabled):";
                for (size_t c = 0; c < arg.const_value().size(); ++c)
                {
                    std::cout << " [ param_" << (c+1) << '=' << arg.const_value()[c] << " ]";
                }
            }
        }
        else if (arg.num_required() > 1)
        {
            for (size_t c = 0; c < arg.num_required(); ++c)
            {
                std::cout << " [ param_" << (c+1);
                if (!arg.const_value().empty())
                {
                    std::cout << '=' << arg.const_value()[c] << " ]";
                }
                else
                {
                    std::cout << " ]";
                }
            }
        }

        std::cout << "\n\tType: " << param_type_str(arg.type());

        if (!arg.default_value().empty())
        {
            std::cout << "\n\tDefault Value[s] (if flag not enabled): ";
            for (size_t v = 0; v < arg.default_value().size(); ++v)
            {
                std::cout << " [ param_" << (v+1) << '=' << arg.default_value()[v] << " ]";
            }
        }

        if (!arg.help_text().empty())
        {
            std::cout << "\n\t" << arg.help_text();
        }

        std::cout << '\n' << std::endl;
    }

    std::exit(errCode);
}



/*-------------------------------------
 *
-------------------------------------*/
[[noreturn]]
void print_err_and_quit(const std::string& errMsg, int errCode) noexcept
{
    std::cerr << errMsg << std::endl;
    std::exit(errCode);
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{
namespace argparse
{



/*-------------------------------------
 *
-------------------------------------*/
ArgParser::~ArgParser() noexcept
{
}



/*-------------------------------------
 *
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
 *
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
 *
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
 *
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
 *
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
 *
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
        mFoundOpts.push_back(false);
        mValues.emplace_back(std::vector<std::string>{});
        idx = mValues.size()-1;

        // Ensure even a null option gets used.
        mLongOptToIndices[hash] = idx;
        mShortOptToIndices[shortName] = idx;

        mArgs.emplace_back(Argument{longName, shortName});
    }

    return mArgs[idx];
}



/*-------------------------------------
 *
-------------------------------------*/
void ArgParser::validate_internal() const noexcept
{
    for (const Argument& arg : mArgs)
    {
        if (!arg.required() || arg.num_required() != static_cast<size_t>(ArgCount::LEAST_ONE))
        {
            continue;
        }

        if (arg.const_value().empty())
        {
            continue;
        }
        else
        {
            if (arg.const_value().size() != arg.num_required())
            {
                print_err_and_quit("Internal error: Constant argument count does not match number of required arguments.", -1);
            }
        }
    }
}



/*-------------------------------------
 *
-------------------------------------*/
void ArgParser::validate_args(int argc, char* const* argv) const noexcept
{
    const char* pOpt = nullptr;
    int i = 1;
    int numFlags = 0;
    std::string currentOpt;
    char currentFlag = '\0';
    size_t currentHash = ~(size_t)0;
    size_t currentArgIndex = ~(size_t)0;
    size_t numArgsForOpt = 0;

    const auto&& param_validator = [&]()->void {
        const Argument& arg = mArgs[currentArgIndex];

        if (arg.num_required() && !numArgsForOpt && arg.const_value().empty())
        {
            const std::string msgParam = !currentOpt.empty() ? currentOpt : std::string{currentFlag};
            print_err_and_quit(std::string{"No parameters provided for argument \""} + msgParam + '\"', -3);
        }
        if (arg.num_required() == static_cast<size_t>(ArgCount::LEAST_ONE) && !numArgsForOpt && arg.const_value().empty())
        {
            const std::string msgParam = !currentOpt.empty() ? currentOpt : std::string{currentFlag};
            print_err_and_quit(std::string{"Argument \""} + msgParam + std::string{"\" requires at least one parameter."}, -3);
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
            print_err_and_quit(err, -4);
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
            print_err_and_quit(err, -5);
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

            if (!mLongOptToIndices.count(currentHash))
            {
                print_err_and_quit(std::string{"Unknown option: "} + currentOpt, -1);
            }

            if (currentOpt == "help")
            {
                print_help_and_quit(mArgs);
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

                if (!mShortOptToIndices.count(currentHash))
                {
                    print_err_and_quit(std::string{"Unknown option: "} + pOpt[j], -2);
                }

                if (pOpt[j] == 'h')
                {
                    print_help_and_quit(mArgs);
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
                print_err_and_quit(std::string{"Unknown option: "} + currentOpt, -3);
            }

            size_t lastArgIndex = currentOpt.empty() ? mShortOptToIndices.at(currentHash) : mLongOptToIndices.at(currentHash);
            const Argument& arg = mArgs[lastArgIndex];
            ArgType type = arg.type();

            if (!param_matches_type(pOpt, type))
            {
                std::string&& err = std::string{"Parameter \""};
                err += pOpt;
                err += std::string{"\" does not match expected type: "};
                err += param_type_str(type);

                print_err_and_quit(err, -4);
            }

            ++numArgsForOpt;
        }

        ++i;
    }

    // validate the final set of arguments
    if (currentHash != ~(size_t)0)
    {
        currentArgIndex = currentOpt.empty() ? mShortOptToIndices.at(currentHash) : mLongOptToIndices.at(currentHash);
        param_validator();
    }
}



/*-------------------------------------
 *
-------------------------------------*/
int ArgParser::parse_long_opt(const std::string& currentOpt, int argId, int argc, char* const* argv) noexcept
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
 *
-------------------------------------*/
int ArgParser::parse_short_opts(const char* pFlags, char lastFlag, int argId, int argc, char* const* argv) noexcept
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
 *
-------------------------------------*/
bool ArgParser::parse(int argc, char* const*argv) noexcept
{
    validate_internal();
    validate_args(argc, argv);

    const char* pOpt = nullptr;
    int i = 1;
    int numFlags = 0;
    char currentFlag = '\0';
    std::string currentOpt;

    mFoundOpts.clear();
    mValues.clear();

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
                i += parse_long_opt(currentOpt, i, argc, argv);
                break;

            case ArgUsage::SHORT_FLAGS:
            case ArgUsage::SHORT_ARG:
                pOpt += 1;
                currentFlag = pOpt[numFlags-1];
                currentOpt.clear();
                i += parse_short_opts(pOpt, currentFlag, i, argc, argv);
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
        size_t hash = Argument::hash_for_name(arg.long_name());
        size_t idx = mLongOptToIndices.count(hash) ? mLongOptToIndices.at(hash) : mShortOptToIndices.at(arg.short_name());
        if (!mFoundOpts[idx])
        {
            mValues[idx] = arg.default_value();
        }
        else
        {
            if (mValues[idx].empty())
            {
                mValues[idx] = arg.const_value();
            }
        }
    }

    mMainFile = argv[0];
    return true;
}



/*-------------------------------------
 *
-------------------------------------*/
bool ArgParser::have_value(const std::string& longName) const noexcept
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
 *
-------------------------------------*/
bool ArgParser::have_value(char shortName) const noexcept
{
    if (!mShortOptToIndices.count(shortName))
    {
        return false;
    }

    size_t idx = mShortOptToIndices.at(shortName);
    return mFoundOpts[idx];
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<std::string>& ArgParser::value(const std::string& longName) const
{
    size_t hash = Argument::hash_for_name(longName);
    size_t idx = mLongOptToIndices.at(hash);
    return mValues[idx];
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<std::string>& ArgParser::value(char shortName) const
{
    size_t hash = Argument::hash_for_name(shortName);
    size_t idx = mShortOptToIndices.at(hash);
    return mValues[idx];
}


} // end argparse namespace
} // end utils namespace
} // end ls namespace