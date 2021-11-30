#pragma once

#include "string"

namespace File
{   
    std::string readFromFile(const std::string& pPath);
    bool exists(const std::string& pPath);
}