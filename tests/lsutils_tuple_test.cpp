/* 
 * File:   lsutils_tuple_test.cpp
 * Author: Miles Lacey
 *
 * Created on October 11, 2015, 6:51 PM
 */

#include <utility>
#include <string>
#include <iostream>

#include "lightsky/utils/tuple.h"


template <typename... data_t> using Tuple_t = ls::utils::Tuple_t<data_t...>;

typedef Tuple_t<int, float, std::string> test_t;



void print_tuple(const std::string& name, const test_t& t) {
    std::cout << "Tuple " << name << ":"
        << "\n\t" << *t.get_object<int>()
        << "\n\t" << *t.get_object<float>()
        << "\n\t" << *t.get_object<std::string>()
        << '\n' << std::endl;
}



int main() {
    test_t a;
    test_t b;
    test_t c;
    
    *a.get_object<int>() = 42;
    *a.get_object<float>() = 4.2f;
    *a.get_object<std::string>() = "Forty-Two";
    
    print_tuple("A", a);
    
    b = a;
    
    print_tuple("B", b);
    
    c = std::move(b);
    
    print_tuple("C", c);
    
    
    return 0;
}

