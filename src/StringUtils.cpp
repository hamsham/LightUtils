/*
 * File:   utils/string_utils.cpp
 * Author: Miles Lacey
 *
 * Created on May 23, 2014, 7:10 PM
 */

#include <climits> // CHAR_BIT
#include <cmath> // std::abs(), std::pow(), std::trunc()
#include <codecvt> // std::c16rtomb(), std::c32rtomb(), std::mbstate_t
#include <cwchar> // std::wcstombs
#include <utility> // std::move

#include "lightsky/setup/OS.h"

#if defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
#include <locale> // std::wstring_convert<>
#else
#include <cuchar> // std::c16rtombs(), std::c32rtombs()
#endif

#include "lightsky/setup/Types.h"

#include "lightsky/utils/StringUtils.h"



/* ----------------------------------------------------------------------------
 * Private (anonymous) namespace
 *
 * TODO: Add support for different numerical bases.
 * --------------------------------------------------------------------------*/
namespace
{



/*-------------------------------------
    Absolute Value, Integer implementation
-------------------------------------*/
template <typename IntegralType>
constexpr IntegralType _impl_abs_integral(IntegralType x, IntegralType mask) noexcept
{
    return (x^mask) - mask;
}



/*-------------------------------------
    Absolute Value, Integer specialization
-------------------------------------*/
template <typename IntegralType>
constexpr IntegralType _impl_abs(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value, IntegralType>::type x) noexcept
{
    return ls::setup::IsUnsigned<IntegralType>::value ? x : _impl_abs_integral<IntegralType>(x, x >> ((sizeof(IntegralType)*CHAR_BIT)-(IntegralType)1));
}



/*-------------------------------------
    Absolute Value, float specialization
-------------------------------------*/
template <typename FloatingType>
constexpr FloatingType _impl_abs(typename ls::setup::EnableIf<ls::setup::IsFloat<FloatingType>::value, FloatingType>::type x) noexcept
{
    return (x >= (FloatingType)0) ? x : -x;
}



/*-------------------------------------
 * Count the number of decimals in a floating-point number
 * ----------------------------------*/
template <typename FloatingType, long long base = 10>
inline size_t _count_printable_decimals(typename ls::setup::EnableIf<ls::setup::IsFloat<FloatingType>::value, FloatingType>::type x)
{
    size_t numDecimals = 0;

    while ((x-std::trunc(x)) > (FloatingType)0)
    {
        x *= (FloatingType)base;
        ++numDecimals;
    }

    return numDecimals;
}



template <size_t base = 10>
inline size_t _count_leading_decimal_zeroes(size_t decimal, size_t numDecimals)
{
    size_t diff = 0;
    while (decimal)
    {
        decimal /= base;
        ++diff;
    }

    return (size_t)numDecimals - diff;
}



template <typename IntegralType, IntegralType base = 10l>
inline size_t _count_printable_digits(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value && ls::setup::IsSigned<IntegralType>::value, IntegralType>::type x)
{
    size_t signByte = (int)x < 0;
    size_t numDigits = 0 || !x;

    while (x)
    {
        x /= base;
        ++numDigits;
    }

    return numDigits + signByte;
}



template <typename IntegralType, IntegralType base = 10l>
inline size_t _count_printable_digits(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value && ls::setup::IsUnsigned<IntegralType>::value, IntegralType>::type x)
{
    size_t numDigits = 0 || !x;

    while (x)
    {
        x /= base;
        ++numDigits;
    }

    return numDigits;
}



template <typename FloatingType, size_t base = 10>
inline size_t _float_info_to_int(
    typename ls::setup::EnableIf<ls::setup::IsFloat<FloatingType>::value, FloatingType>::type x,
    size_t* pIntegral,
    size_t* pDecimal,
    size_t* pNumDecimals,
    size_t* pLeadingZeroes)
{
    FloatingType n = _impl_abs<FloatingType>(x);

    *pIntegral      = (size_t)n;
    *pNumDecimals   = _count_printable_decimals<FloatingType, base>(n);
    *pDecimal       = (size_t)((n - std::trunc(n)) * std::pow((FloatingType)base, (int)(*pNumDecimals)));
    *pLeadingZeroes = _count_leading_decimal_zeroes<base>(*pDecimal, *pNumDecimals);

    return (size_t)(x < (FloatingType)0);
}



/*
template <typename FloatingType, long long base = 10>
inline size_t _count_printable_digits(typename ls::setup::EnableIf<ls::setup::IsFloat<FloatingType>::value, FloatingType>::type x)
{
    size_t integral, decimal, numDecimals, leadingZeroes, haveSign;

    haveSign = _float_info_to_int<FloatingType, base>(x, &integral, &decimal, &numDecimals, &leadingZeroes);

    size_t ret = haveSign;
    ret += integral ? _count_printable_digits<long long, base>(integral) : 1;
    ret += 1; // decimal point
    ret += (size_t)leadingZeroes;
    ret += _count_printable_digits<long long, base>(decimal);

    return ret;
}
*/



template <typename FloatingType, long long base = 10>
inline size_t _count_printable_digits(
    typename ls::setup::EnableIf<ls::setup::IsFloat<FloatingType>::value, FloatingType>::type x,
    size_t* haveSign,
    size_t* pIntegral,
    size_t* pNumIntegrals,
    size_t* pDecimal,
    size_t* pNumDecimals,
    size_t* pLeadingZeroes)
{
    *haveSign = _float_info_to_int<FloatingType, base>(x, pIntegral, pDecimal, pNumDecimals, pLeadingZeroes);

    size_t ret = *haveSign;
    *pNumIntegrals = _count_printable_digits<long long, base>(*pIntegral);
    ret += *pIntegral ? *pNumIntegrals : 1;
    ret += 1; // decimal point
    ret += *pLeadingZeroes;
    ret += _count_printable_digits<long long, base>(*pDecimal);

    return ret;
}



/*
template <typename IntegralType, IntegralType base = 10l>
inline size_t _integral_to_char_buffer(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value, IntegralType>::type x, char* pBuf)
{
    static_assert(base == 10, "Support not added for bases other than 10.");
    constexpr char asciiTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
    size_t iter = _count_printable_digits<IntegralType, base>(x);
    size_t numDigits = iter;
    char* pOut = (pBuf + iter) - 1;

    while (iter--)
    {
        (*pOut--) = asciiTable[x % base];
        x /= base;
    }

    return numDigits;
}
*/



template <typename IntegralType, IntegralType base = 10l>
inline size_t _integral_to_char_buffer(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value, IntegralType>::type x, char* pBuf, size_t numPrintable)
{
    static_assert(base == 10, "Suport not added for bases other than 10.");
    constexpr char asciiTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
    size_t iter = numPrintable;
    size_t numDigits = numPrintable;
    char* pOut = (pBuf + iter) - 1;

    while (iter--)
    {
        (*pOut--) = asciiTable[x % base];
        x /= base;
    }

    return numDigits;
}



template <typename FloatingType, size_t base = 10>
std::string _impl_to_string(typename ls::setup::EnableIf<ls::setup::IsFloat<FloatingType>::value, FloatingType>::type x)
{
    constexpr size_t nanVal = (size_t)((~0ull >> 1ull)+1ull);
    size_t integral, numIntegrals, decimal, numDecimals, leadingZeroes, haveSign;
    const size_t numPrintable = _count_printable_digits<FloatingType, base>(x, &haveSign, &integral, &numIntegrals, &decimal, &numDecimals, &leadingZeroes);

    if (decimal == nanVal || integral == ~(size_t)0)
    {

        if (integral == nanVal)
        {
            return std::string{"NaN"};
        }
        else if (x > (FloatingType)0)
        {
            return std::string{"Inf"};
        }
        else
        {
            return std::string{"-Inf"};
        }
    }

    std::string ret(numPrintable, 'x');
    typename std::string::size_type iter = 0;

    if (haveSign)
    {
        ret[iter++] = '-';
    }

    if (!integral)
    {
        ret[iter++] = '0';
    }
    else
    {
        iter += _integral_to_char_buffer<long long, base>(integral, &ret[iter], numIntegrals);
    }

    ret[iter++] = '.';

    for (size_t lz = 0; lz < leadingZeroes; --lz)
    {
        char c = '0';
        ret[iter++] = c;
    }

    _integral_to_char_buffer<long long, base>(decimal, &ret[iter], numDecimals ? numDecimals : 1);

    return ret;
}



template <typename IntegralType, size_t base = 10>
std::string _impl_to_string(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value && ls::setup::IsSigned<IntegralType>::value, IntegralType>::type x)
{
    size_t integral, haveSign, numPrintable;

    integral = (size_t)_impl_abs<IntegralType>(x);
    haveSign = (int)x < 0;
    numPrintable = _count_printable_digits<IntegralType, base>(x);

    std::string ret(numPrintable, 'x');
    typename std::string::size_type iter = 0;

    if (haveSign)
    {
        ret[iter++] = '-';
    }

    if (!integral)
    {
        ret[iter++] = '0';
    }
    else
    {
        iter += _integral_to_char_buffer<long long, base>(integral, &ret[iter], numPrintable-iter);
    }

    return ret;
}



template <typename IntegralType, size_t base = 10>
std::string _impl_to_string(typename ls::setup::EnableIf<ls::setup::IsIntegral<IntegralType>::value && ls::setup::IsUnsigned<IntegralType>::value, IntegralType>::type x)
{
    const size_t integral = (size_t)_impl_abs<IntegralType>(x);
    const size_t numPrintable = _count_printable_digits<IntegralType, base>(x);

    std::string ret(numPrintable, 'x');
    typename std::string::size_type iter = 0;

    if (!integral)
    {
        ret[iter++] = '0';
    }
    else
    {
        iter += _integral_to_char_buffer<long long, base>(integral, &ret[iter], numPrintable);
    }

    return ret;
}



} // end anonymous namespace



/* ----------------------------------------------------------------------------
 * LightSky namespace
 * --------------------------------------------------------------------------*/
namespace ls
{



/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(char c)
{
    return std::string(1, c);
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(unsigned char c)
{
    return std::string(1, c);
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(short x)
{
    return _impl_to_string<short>(x);
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(unsigned short x)
{
    return std::move(_impl_to_string<unsigned short>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(int x)
{
    return std::move(_impl_to_string<int>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(unsigned int x)
{
    return std::move(_impl_to_string<unsigned int>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(long x)
{
    return std::move(_impl_to_string<long>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(unsigned long x)
{
    return std::move(_impl_to_string<unsigned long>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(long long x)
{
    return std::move(_impl_to_string<long long>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(unsigned long long x)
{
    return std::move(_impl_to_string<unsigned long long>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(float x)
{
    return std::move(_impl_to_string<float>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(double x)
{
    return std::move(_impl_to_string<double>(x));
}

/*-------------------------------------
 * Character string conversion
 * ----------------------------------*/
std::string utils::to_str(long double x)
{
    return std::move(_impl_to_string<long double>(x));
}



/*-------------------------------------
 * Wide-String to Multi-Byte
 * ----------------------------------*/
std::string utils::to_str(const std::wstring& wstr)
{
    std::mbstate_t mb{};
    std::string ret;
    char temp[MB_LEN_MAX];

    //std::setlocale(LC_CTYPE, "");

    ret.reserve(wstr.size()*sizeof(std::wstring::value_type));

    for (const typename std::wstring::value_type w : wstr)
    {
        std::size_t numBytes = 0;

        numBytes = std::wcrtomb(temp, w, &mb);

        if (numBytes == static_cast<std::size_t>(-1))
        {
            ret.clear();
            break;
        }

        ret.append(temp, numBytes);
    }

    return ret;
}



/*-------------------------------------
 * UTF-16 to Multi-Byte
 * ----------------------------------*/
std::string utils::to_str(const std::u16string& wstr)
{
    #if defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
        std::string u8 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(wstr);
        return u8;
    #else
        std::mbstate_t mb{};
        std::string ret;
        char temp[MB_LEN_MAX];

        //std::setlocale(LC_CTYPE, "");

        ret.reserve(wstr.size()*sizeof(std::u16string::value_type));

        for (const typename std::u16string::value_type c16 : wstr)
        {
            std::size_t numBytes = 0;

            numBytes = std::c16rtomb(temp, c16, &mb);

            if (numBytes == static_cast<std::size_t>(-1))
            {
                ret.clear();
                break;
            }

            ret.append(temp, numBytes);
        }

        return ret;
    #endif
}



/*-------------------------------------
 * WUTF-32 to Multi-Byte
 * ----------------------------------*/
std::string utils::to_str(const std::u32string& wstr)
{
    #if defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
        std::string u8 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(wstr);
        return u8;
    #else
        std::mbstate_t mb{};
        std::string ret;
        char temp[MB_LEN_MAX];

        //std::setlocale(LC_CTYPE, "");

        ret.reserve(wstr.size()*sizeof(std::u32string::value_type));

        for (const typename std::u32string::value_type c32 : wstr)
        {
            std::size_t numBytes = 0;

            numBytes = std::c32rtomb(temp, c32, &mb);

            if (numBytes == static_cast<std::size_t>(-1))
            {
                ret.clear();
                break;
            }

            ret.append(temp, numBytes);
        }

        return ret;
    #endif
}



} // end ls namespace
