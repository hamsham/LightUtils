/*
 * File:   ls_utils_tuple_test.cpp
 * Author: Miles Lacey
 *
 * Created on October 11, 2015, 6:51 PM
 */

#include <utility>
#include <string>
#include <iostream>

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

    return 0;
}
