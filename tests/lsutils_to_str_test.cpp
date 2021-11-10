
#include <clocale> // std::setlocale()
#include <cmath>
#include <cstdlib> // rand(), srand()
#include <ctime>
#include <iostream>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"

namespace utils = ls::utils;



struct StdToString
{
    template <typename data_type>
    std::string operator()(const data_type& n) const noexcept
    {
        return std::to_string(n);
    }
};

struct LSToString
{
    template <typename data_type>
    std::string operator()(const data_type& n) const noexcept
    {
        return utils::to_str(n);
    }
};



template <typename ToStringType>
void test_str_conversion() noexcept
{
    constexpr ToStringType converter;
    float testf;
    double testd;
    int testi;

    testf = -0.f;
    std::cout << testf << ": " << converter(testf) << std::endl;

    testf = INFINITY;
    std::cout << testf << ": " << converter(testf) << std::endl;

    testf = -INFINITY;
    std::cout << testf << ": " << converter(testf) << std::endl;

    testf = NAN;
    std::cout << testf << ": " << converter(testf) << std::endl;

    testf = 1.f / 3.f;
    std::cout << testf << ": " << converter(testf) << std::endl;

    testf = -0.000001f;
    std::cout << testf << ": " << converter(testf) << std::endl;

    testd = -123.000456;
    std::cout << testd << ": " << converter(testd) << std::endl;

    testd = 65536.0;
    std::cout << testd << ": " << converter(testd) << std::endl;

    testd = 0.0000987654321;
    std::cout << testd << ": " << converter(testd) << std::endl;

    testi = -123;
    std::cout << testi << ": " << converter(testi) << std::endl;

    testi = 123;
    std::cout << testi << ": " << converter(testi) << std::endl;

    testi = -0;
    std::cout << testi << ": " << converter(testi) << std::endl;
}



template <typename ToStringType, unsigned numIters = 50000000u>
unsigned long long bench_str_conversionf() noexcept
{
    constexpr ToStringType converter;
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ticks.start();

    for (unsigned i = numIters; i--;)
    {
        float x = -(float)rand() / (float)rand();

        try
        {
            float val = std::stof(converter(x));
            (void)val;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return 0ull;
        }
    }

    ticks.tick();
    return ticks.tick_time().count();
}



template <typename ToStringType, unsigned numIters = 50000000u>
unsigned long long bench_str_conversioni() noexcept
{
    constexpr ToStringType converter;
    ls::utils::Clock<unsigned long long, std::ratio<1, 1000>> ticks;
    ticks.start();

    for (unsigned i = numIters; i--;)
    {
        int x = (int)rand();

        try
        {
            int val = std::stoi(converter(x));
            (void)val;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return 0ull;
        }
    }

    ticks.tick();
    return ticks.tick_time().count();
}



int main()
{
    srand(time(nullptr));
    std::setlocale(LC_CTYPE, ""); // for converting unicode to multibyte

    std::cout
        << "----------------------------------------\n"
        << "Testing std::to_string conversion\n"
        << "----------------------------------------"
        << std::endl;
    test_str_conversion<StdToString>();
    std::cout << "\tDone.\n" << std::endl;

    std::cout
        << "----------------------------------------\n"
        << "Testing utils::to_str conversion\n"
        << "----------------------------------------"
        << std::endl;
    test_str_conversion<LSToString>();
    std::cout << "\tDone.\n" << std::endl;

    std::cout
        << std::string{u8"ひらがな"} << ": " << utils::to_str(std::wstring{L"ひらがな"}) << '\n'
        << std::string{u8"カタカナ"} << ": " << utils::to_str(std::u16string{u"カタカナ"}) << '\n'
        << std::string{u8"絵文字"} << ": " << utils::to_str(std::u32string{U"絵文字"}) << '\n'
        << std::endl;

    std::cout
        << "----------------------------------------\n"
        << "Benchmarking std::to_string(float) conversion\n"
        << "----------------------------------------"
        << std::endl;
    unsigned long long cvtTimeStdf = bench_str_conversionf<StdToString>();
    std::cout << "\tDone.\n" << std::endl;

    std::cout
        << "----------------------------------------\n"
        << "Benchmarking utils::to_str(float) conversion\n"
        << "----------------------------------------"
        << std::endl;
    unsigned long long cvtTimeLUf = bench_str_conversionf<LSToString>();
    std::cout << "\tDone.\n" << std::endl;

    std::cout
        << "----------------------------------------\n"
        << "Benchmarking std::to_string(int) conversion\n"
        << "----------------------------------------"
        << std::endl;
    unsigned long long cvtTimeStdi = bench_str_conversioni<StdToString>();
    std::cout << "\tDone.\n" << std::endl;

    std::cout
        << "----------------------------------------\n"
        << "Benchmarking utils::to_str(float) conversion\n"
        << "----------------------------------------"
        << std::endl;
    unsigned long long cvtTimeLUi = bench_str_conversioni<LSToString>();
    std::cout << "\tDone.\n" << std::endl;

    std::cout << "STD Conversion time (float): " << utils::to_str((double)cvtTimeStdf*0.001) << std::endl;
    std::cout << "LS Conversion time (float): " << utils::to_str((double)cvtTimeLUf*0.001) << std::endl;

    std::cout << "STD Conversion time (int): " << utils::to_str((double)cvtTimeStdi*0.001) << std::endl;
    std::cout << "LS Conversion time (int): " << utils::to_str((double)cvtTimeLUi*0.001) << std::endl;

    return 0;
}

