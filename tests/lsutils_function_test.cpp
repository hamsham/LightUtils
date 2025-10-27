/*
 * File:   lsutils_function_test.cpp
 * Author: miles
 * Created on October 26, 2025, at 12:27 p.m.
 */

#include <iostream>

#include "lightsky/utils/Function.hpp"



int main()
{
    int (*op1)(int, int) = [](int a, int b) noexcept -> int { return a + b; };
    ls::utils::Function<int(int, int)> func1{op1};
    ls::utils::Function<int(int, int)> func2;
    func2 = op1;

    int c = 4;
    int d = 5;
    auto&& op2 = [&]() noexcept -> int { return c + d; };
    ls::utils::Function<int()> func3 = op2;

    int result1 = func1(1, 2); // 3
    int result2 = func2(2, 3); // 5
    int result3 = func3(); // 9
    int result4 = 0;
    int result5 = 0;
    int result6 = 0;

    ls::utils::Function<void()> func4 = [&]() noexcept -> void { result4 = c + d; };
    func4(); // 9

    ls::utils::Function<int()> func5 = func3;
    result5 = func5() + func5(); // 18

    ls::utils::Function<int()> func6 = [=]() noexcept -> int { return func1(1, 2) + func3(); };
    result6 = func6(); // 12

    std::cout << "Function result 1 (" << func1.target_size() << " -> " << sizeof(func1) << "): " << result1 << std::endl;
    std::cout << "Function result 2 (" << func2.target_size() << " -> " << sizeof(func2) << "): " << result2 << std::endl;
    std::cout << "Function result 3 (" << func3.target_size() << " -> " << sizeof(func3) << "): " << result3 << std::endl;
    std::cout << "Function result 4 (" << func4.target_size() << " -> " << sizeof(func4) << "): " << result4 << std::endl;
    std::cout << "Function result 5 (" << func5.target_size() << " -> " << sizeof(func5) << "): " << result5 << std::endl;
    std::cout << "Function result 6 (" << func6.target_size() << " -> " << sizeof(func6) << "): " << result6 << std::endl;

    return 0;
}
