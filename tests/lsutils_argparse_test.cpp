
#include <iostream>

#include "lightsky/utils/ArgParser.hpp"
#include "lightsky/utils/Argument.hpp"

namespace argparse = ls::utils::argparse;



void print_vector(const std::vector<std::string>& data)
{
    for (const std::string& str : data)
    {
        std::cout << "\t" << str << std::endl;
    }
}




int main(int argc, char* argv[])
{
    argparse::ArgParser parser;
    parser.set_argument("help", 'h')
        .num_required(argparse::ArgCount::ZERO)
        .description("Help")
        .help_text("Print this help and exit.");

    parser.set_argument("test1", 'a')
          .num_required(argparse::ArgCount::ONE)
          .description("Test 1")
          .required(true)
          .type(argparse::ArgType::REAL)
          .help_text("Testing a single argument of 1 element.");

    parser.set_argument("test2", 'b')
          .num_required(argparse::ArgCount::ONE)
          .default_value("Test 2")
          .required(true)
          .type(argparse::ArgType::STRING)
          .description("Test 2")
          .help_text("Testing a single argument with a short and long opt.");

    parser.set_argument("test3")
          .num_required(4)
          .required(true)
          .type(argparse::ArgType::INTEGRAL)
          .description("Test 3")
          .help_text("Testing multiple elements.");

    parser.set_argument("test4")
          .num_required(argparse::ArgCount::ZERO)
          .required(false)
          .description("Test 4")
          .help_text("Testing a flag.");

    parser.set_argument("test5", 'e')
          .num_required(argparse::ArgCount::ZERO)
          .description("Test 5")
          .help_text("Testing a flag with a short option.");

    parser.set_argument("test6", '6')
          .num_required(argparse::ArgCount::LEAST_ONE)
          .const_value("T")
          .required(false)
          .type(argparse::ArgType::CHAR)
          .description("Test 6")
          .help_text("Testing a not-required, const value.");

    parser.parse(argc, argv);

    if (parser.have_value('h'))
    {
        std::cout << "Found help option" << std::endl;
    }

    if (parser.have_value('a'))
    {
        std::cout << "Found 'a'" << std::endl;
        print_vector(parser.value("test1"));
    }

    if (parser.have_value('b'))
    {
        std::cout << "Found 'b'" << std::endl;
        print_vector(parser.value("test2"));
    }

    if (parser.have_value("test3"))
    {
        std::cout << "Found \"test3\"" << std::endl;
        print_vector(parser.value("test3"));
    }

    if (parser.have_value("test4"))
    {
        std::cout << "Found \"test4\"" << std::endl;
        print_vector(parser.value("test4"));
    }

    if (parser.have_value("test5"))
    {
        std::cout << "Found \"test5\"" << std::endl;
    }

    if (parser.have_value("test6"))
    {
        std::cout << "Found \"test6\"" << std::endl;
        print_vector(parser.value('6'));
    }

    return 0;
}
