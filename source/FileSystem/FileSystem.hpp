#pragma once

#include "string"

class File
{   
public:
    static std::string executablePath;
    static std::string rootDirectory;
    static std::string shaderDirectory;

    static std::string readFromFile(const std::string& pPath);
    static bool exists(const std::string& pPath);

    static void setupDirectories(const std::string& pExecutePath);
};