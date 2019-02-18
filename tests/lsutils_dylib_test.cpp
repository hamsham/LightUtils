
#include <iostream>

#include "lightsky/setup/OS.h"

#include "lightsky/utils/DynamicLib.hpp"



int main()
{
    ls::utils::DynamicLib lib;

    #ifndef LS_OS_WINDOWS
        constexpr char libName[] = "libGL.so";
    #else
        constexpr char libName[] = "opengl32.dll";
    #endif

    if (lib.load(libName) != 0)
    {
        std::cerr << "Unable to load the OpenGL library." << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Successfully loaded the OpenGL library." << std::endl;
    }

    void* testFunction = lib.symbol("glActiveTexture");
    if (!testFunction)
    {
        std::cerr << "Unable to locate a test function within the OpenGL library." << std::endl;
    }
    else
    {
        std::cout << "Successfully located the test function \"glActiveTexture\" within the OpenGL library: " << testFunction << std::endl;
    }

    return 0;
}

