#pragma once
#include <string>
#include <iostream>

namespace CE
{

    struct MotorConfig
    {
        unsigned int vW=0;
        unsigned int vH=0;
        std::string titulo="";
    };

    inline void printDebug(const std::string& msg)
    {
        if constexpr (DEBUG)
            std::cout << "[DEBUG]: " << msg << std::endl;
    }
}
