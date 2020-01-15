
#include <clocale> // std::setlocale()
#include <cstdlib> // rand(), srand()
#include <ctime>
#include <iostream>

#include "lightsky/utils/StringUtils.h"

namespace utils = ls::utils;



int main()
{
    srand(time(nullptr));
    std::setlocale(LC_CTYPE, ""); // for converting unicode to multibyte

    for (int i = 0; i < 100; ++i)
    {
        float x = -(float)rand() / (float)rand();
        std::cout << x << ": " << utils::to_str(x) << std::endl;
    }

    float testf;
    double testd;
    int testi;

    testf = -0.f;
    std::cout << testf << ": " << utils::to_str(testf) << std::endl;

    testf = 1.f / 3.f;
    std::cout << testf << ": " << utils::to_str(testf) << std::endl;

    testf = -0.000001f;
    std::cout << testf << ": " << utils::to_str(testf) << std::endl;

    testd = -123.000456;
    std::cout << testd << ": " << utils::to_str(testd) << std::endl;

    testd = 65536.0;
    std::cout << testd << ": " << utils::to_str(testd) << std::endl;

    testd = 0.0000987654321;
    std::cout << testd << ": " << utils::to_str(testd) << std::endl;

    testi = -123;
    std::cout << testi << ": " << utils::to_str(testi) << std::endl;

    testi = 123;
    std::cout << testi << ": " << utils::to_str(testi) << std::endl;

    testi = -0;
    std::cout << testi << ": " << utils::to_str(testi) << std::endl;

    std::cout << std::string{u8"ひらがな"} << ": " << utils::to_str(std::wstring{L"ひらがな"}) << std::endl;
    std::cout << std::string{u8"カタカナ"} << ": " << utils::to_str(std::u16string{u"カタカナ"}) << std::endl;
    std::cout << std::string{u8"絵文字"} << ": " << utils::to_str(std::u32string{U"絵文字"}) << std::endl;

    return 0;
}

