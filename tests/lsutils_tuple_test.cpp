/*
 * File:   ls_utils_tuple_test.cpp
 * Author: Miles Lacey
 *
 * Created on October 11, 2015, 6:51 PM
 */

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "lightsky/utils/Tuple.h"


template <typename... data_t>
using Tuple = ls::utils::Tuple<data_t...>;

typedef Tuple<int, std::string, float, std::string> test_t;



void print_tuple(const std::string& name, const test_t& t)
{
    std::cout
        << "Tuple " << name << ":"
        << "\n\t" << t.first_of<int>()
        << "\n\t" << t.const_element<1>()
        << "\n\t" << t.first_of<float>()
        << "\n\t" << t.const_element<3>()
        << '\n' << std::endl;
}



template <unsigned numArgs, unsigned currentArg, typename... ArgsType>
struct SoaPrinter;

template <typename... ArgsType, unsigned numArgs>
struct SoaPrinter<numArgs, sizeof...(ArgsType)-1u, ArgsType...>
{
    void operator()(const ls::utils::Tuple<ArgsType...>& tuple)
    {
        const auto& vec = tuple.template const_element<sizeof...(ArgsType)-1u>();
        for (const auto& element : vec)
        {
            std::cout << element << ' ';
        }
        std::cout << std::endl;
    }
};

template <unsigned numArgs, unsigned currentArg, typename... ArgsType>
struct SoaPrinter
{
    void operator()(const ls::utils::Tuple<ArgsType...>& tuple)
    {
        const auto& vec = tuple.template const_element<currentArg>();
        for (const auto& element : vec)
        {
            std::cout << element << ' ';
        }
        std::cout << std::endl;

        SoaPrinter<sizeof...(ArgsType), currentArg+1u, ArgsType...> soaPrinter;
        soaPrinter(tuple);
    }
};



template <typename... ArgsType>
void print_soa(const ls::utils::Tuple<ArgsType...>& tuple) noexcept
{
    SoaPrinter<sizeof...(ArgsType), 0u, ArgsType...> soaPrinter;
    soaPrinter(tuple);
}



void test_soa()
{
    Tuple<std::vector<int>, std::vector<float>, std::vector<std::string>> soa;
    soa.element<0>().push_back(1);
    soa.element<0>().push_back(2);
    soa.element<0>().push_back(3);

    soa.element<1>().push_back(4.4f);
    soa.element<1>().push_back(5.5f);
    soa.element<1>().push_back(6.6f);

    soa.element<2>().push_back("seven");
    soa.element<2>().push_back("eight");
    soa.element<2>().push_back("nine");

    print_soa(soa);
}



int main() {
    test_t a;
    test_t b;
    test_t c;

    a.first_of<int>() = 42;
    a.first_of<float>() = 4.2f;
    a.first_of<std::string>() = "Forty-Two";
    a.element<3>() = "Hello World!";

    print_tuple("A", a);

    b = a;
    print_tuple("B", b);

    c = std::move(b);
    print_tuple("C", c);

    *reinterpret_cast<std::string*>(c.get(1)) = "Foo";
    print_tuple("C", c);

    c.element<3>() = "Goodbye Cruel World!";
    print_tuple("C", c);

    test_soa();

    return 0;
}
