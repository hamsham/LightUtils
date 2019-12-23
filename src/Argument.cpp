
#include <functional> // std::hash
#include <type_traits>
#include <utility> // std::move()

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
constexpr uint32_t ArgHash_FNV1a_32(const char* str, uint32_t hash = 2166136261) noexcept
{
    return (!str || !str[0]) ? hash : ArgHash_FNV1a_32(str + 1, ((uint32_t)str[0] ^ hash) * 16777619);
}



template <typename EnumType>
constexpr auto _enum_to_value(typename std::enable_if<std::is_enum<EnumType>::value, EnumType>::type x) noexcept -> typename std::underlying_type<EnumType>::type
{
    return static_cast<typename std::underlying_type<EnumType>::type>(x);
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
namespace std
{



template <>
struct hash<argparse::Argument>
{
    typedef argparse::Argument argument_type;
    typedef std::size_t result_type;

    inline result_type operator()(const argparse::Argument& arg) noexcept
    {
        return arg.hash();

    }
};



} // end std namespace



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
 *
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
 *
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
 *
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
 *
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
 *
-------------------------------------*/
size_t Argument::hash() const noexcept
{
    return Argument::hash_for_name(mLongOpt, mShortOpt);
}



/*-------------------------------------
 *
-------------------------------------*/
size_t Argument::hash_for_name(const std::string& longName) noexcept
{
    return ArgHash_FNV1a_32(longName.c_str());
}



/*-------------------------------------
 *
-------------------------------------*/
size_t Argument::hash_for_name(char shortName) noexcept
{
    return (size_t)shortName;
}



/*-------------------------------------
 *
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
 *
-------------------------------------*/
Argument& Argument::default_value(const char* defaultVal) noexcept
{
    mDefaultVal = std::vector<std::string>{std::string{defaultVal}};
    mRequired = true;
    return *this;
}




/*-------------------------------------
 *
-------------------------------------*/
const std::vector<std::string>& Argument::default_value() const noexcept
{
    return mDefaultVal;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::const_value(const char* constVal) noexcept
{
    mConstVal = std::vector<std::string>{std::string{constVal}};
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<std::string>& Argument::const_value() const noexcept
{
    return mConstVal;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::description(const std::string& shortDesc) noexcept
{
    mDescription = shortDesc;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
const std::string& Argument::description() const noexcept
{
    return mDescription;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::help_text(const std::string& help) noexcept
{
    mHelp = help;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
const std::string& Argument::help_text() const noexcept
{
    return mHelp;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::num_required(ArgCount numParams) noexcept
{
    mNumArgs = static_cast<std::underlying_type<ArgCount>::type>(numParams);
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::num_required(size_t numParams) noexcept
{
    mNumArgs = numParams;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
size_t Argument::num_required() const noexcept
{
    return mNumArgs;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::required(bool isRequired) noexcept
{
    mRequired = isRequired;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
bool Argument::required() const noexcept
{
    return mRequired;
}



/*-------------------------------------
 *
-------------------------------------*/
Argument& Argument::type(ArgType dataType) noexcept
{
    mType = dataType;
    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
ArgType Argument::type() const noexcept
{
    return mType;
}





} // end argparse namespace
} // end utils namespace
} // end ls namespace
