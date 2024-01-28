
#include <cstdint>
#include <functional> // std::hash
#include <type_traits>
#include <utility> // std::move()

#include "lightsky/utils/Argument.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{

namespace argparse = ls::utils::argparse;



/*-------------------------------------
 * Fast hash to store command-line args
-------------------------------------*/
constexpr uint32_t ArgHash_FNV1a_32(const char* str, uint32_t hash = 2166136261) noexcept
{
    return (!str || !str[0]) ? hash : ArgHash_FNV1a_32(str + 1, ((uint32_t)str[0] ^ hash) * 16777619);
}



/*-------------------------------------
 * Convert an enum to an integral
-------------------------------------*/
template <typename EnumType>
constexpr typename std::underlying_type<EnumType>::type _enum_to_value(
    typename std::enable_if<std::is_enum<EnumType>::value, EnumType>::type x) noexcept
{
    return static_cast<typename std::underlying_type<EnumType>::type>(x);
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
 * Destructor
-------------------------------------*/
Argument::~Argument() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Argument::Argument(const std::string& longOpt, char shortOpt) noexcept :
    mLongOpt{longOpt},
    mShortOpt{shortOpt},
    mDefaultVal{},
    mConstVal{},
    mDescription{},
    mHelp{},
    mType{ArgType::STRING},
    mNumArgs{_enum_to_value<ArgCount>(ArgCount::ONE)},
    mRequired{true}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
Argument::Argument(const Argument& arg) noexcept :
    mLongOpt{arg.mLongOpt},
    mShortOpt{arg.mShortOpt},
    mDefaultVal{arg.mDefaultVal},
    mConstVal{arg.mConstVal},
    mDescription{arg.mDescription},
    mHelp{arg.mHelp},
    mType{arg.mType},
    mNumArgs{arg.mNumArgs},
    mRequired{arg.mRequired}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
Argument::Argument(Argument&& arg) noexcept :
    mLongOpt{std::move(arg.mLongOpt)},
    mShortOpt{arg.mShortOpt},
    mDefaultVal{std::move(arg.mDefaultVal)},
    mConstVal{std::move(arg.mConstVal)},
    mDescription{std::move(arg.mDescription)},
    mHelp{std::move(arg.mHelp)},
    mType{arg.mType},
    mNumArgs{arg.mNumArgs},
    mRequired{arg.mRequired}
{
    arg.mShortOpt = '\0';
    arg.mType = ArgType::STRING;
    arg.mNumArgs = _enum_to_value<ArgCount>(ArgCount::ONE);
    arg.mRequired = true;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
Argument& Argument::operator=(const Argument& arg) noexcept
{
    mLongOpt = arg.mLongOpt;
    mShortOpt = arg.mShortOpt;
    mDefaultVal = arg.mDefaultVal;
    mDescription = arg.mDescription;
    mHelp = arg.mHelp;
    mType = arg.mType;
    mConstVal = arg.mConstVal;
    mNumArgs = arg.mNumArgs;
    mRequired = arg.mRequired;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
Argument& Argument::operator=(Argument&& arg) noexcept
{
    mLongOpt = std::move(arg.mLongOpt);

    mShortOpt = arg.mShortOpt;
    arg.mShortOpt = '\0';

    mDefaultVal = std::move(arg.mDefaultVal);

    mConstVal = std::move(arg.mConstVal);

    mDescription = std::move(arg.mDescription);

    mHelp = std::move(arg.mHelp);

    mType = arg.mType;
    arg.mType = ArgType::STRING;

    mNumArgs = arg.mNumArgs;
    arg.mNumArgs = _enum_to_value<ArgCount>(ArgCount::ONE);

    mRequired = arg.mRequired;
    arg.mRequired = true;

    return *this;
}



/*-------------------------------------
 * Get eh long version of an argument's name
-------------------------------------*/
const std::string& Argument::long_name() const noexcept
{
    return mLongOpt;
}



/*-------------------------------------
 * Get eh long version of an argument's name
-------------------------------------*/
char Argument::short_name() const noexcept
{
    return mShortOpt;
}


/*-------------------------------------
 * Get the hash for *this
-------------------------------------*/
size_t Argument::hash() const noexcept
{
    return Argument::hash_for_name(mLongOpt, mShortOpt);
}



/*-------------------------------------
 * Get the hash of an argument based on its long name
-------------------------------------*/
size_t Argument::hash_for_name(const std::string& longName) noexcept
{
    return ArgHash_FNV1a_32(longName.c_str());
}



/*-------------------------------------
 * Get the hash of an argument based on its short name
-------------------------------------*/
size_t Argument::hash_for_name(char shortName) noexcept
{
    return (size_t)shortName;
}



/*-------------------------------------
 * Get the hash of an argument
-------------------------------------*/
size_t Argument::hash_for_name(const std::string& longName, char shortName) noexcept
{
    if (!longName.empty())
    {
        return ArgHash_FNV1a_32(longName.c_str());
    }

    return (size_t)shortName;
}



/*-------------------------------------
 * value returned if there's no key+value pair
-------------------------------------*/
Argument& Argument::default_value(const char* defaultVal) noexcept
{
    mDefaultVal = std::vector<std::string>{std::string{defaultVal}};
    mRequired = true;
    return *this;
}




/*-------------------------------------
 * value returned if there's no key+value pair
-------------------------------------*/
const std::vector<std::string>& Argument::default_value() const noexcept
{
    return mDefaultVal;
}



/*-------------------------------------
 * Value returned if a key has no value
-------------------------------------*/
Argument& Argument::const_value(const char* constVal) noexcept
{
    mConstVal = std::vector<std::string>{std::string{constVal}};
    return *this;
}



/*-------------------------------------
 * Value returned if a key has no value
-------------------------------------*/
const std::vector<std::string>& Argument::const_value() const noexcept
{
    return mConstVal;
}



/*-------------------------------------
 * Set the argument description
-------------------------------------*/
Argument& Argument::description(const std::string& shortDesc) noexcept
{
    mDescription = shortDesc;
    return *this;
}



/*-------------------------------------
 * Get the argument description
-------------------------------------*/
const std::string& Argument::description() const noexcept
{
    return mDescription;
}



/*-------------------------------------
 * Set an argument's help message
-------------------------------------*/
Argument& Argument::help_text(const std::string& help) noexcept
{
    mHelp = help;
    return *this;
}



/*-------------------------------------
 * Retrieve an argument's help message
-------------------------------------*/
const std::string& Argument::help_text() const noexcept
{
    return mHelp;
}



/*-------------------------------------
 * Require an arg needs 0, 1, or 1+ values
-------------------------------------*/
Argument& Argument::num_required(ArgCount numParams) noexcept
{
    mNumArgs = static_cast<std::underlying_type<ArgCount>::type>(numParams);
    return *this;
}



/*-------------------------------------
 * Require an arg needs multiple parameters
-------------------------------------*/
Argument& Argument::num_required(size_t numParams) noexcept
{
    mNumArgs = numParams;
    return *this;
}



/*-------------------------------------
 * Get the number of required values for an argument
-------------------------------------*/
size_t Argument::num_required() const noexcept
{
    return mNumArgs;
}



/*-------------------------------------
 * Set that an arg must have a user-provided value
-------------------------------------*/
Argument& Argument::required(bool isRequired) noexcept
{
    mRequired = isRequired;
    return *this;
}



/*-------------------------------------
 * Get if an arg must have a user-provided value
-------------------------------------*/
bool Argument::required() const noexcept
{
    return mRequired;
}



/*-------------------------------------
 * Set an argument's type
-------------------------------------*/
Argument& Argument::type(ArgType dataType) noexcept
{
    mType = dataType;
    return *this;
}



/*-------------------------------------
 * Retrieve an argument's type
-------------------------------------*/
ArgType Argument::type() const noexcept
{
    return mType;
}



} // end argparse namespace
} // end utils namespace
} // end ls namespace
