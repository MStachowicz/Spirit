#pragma once

#include "string"
#include "vector"

class File
{
public:
    static std::string executablePath;
    static std::string rootDirectory;
    static std::string GLSLShaderDirectory;
    static std::string textureDirectory;

    static std::string removeFileExtension(const std::string& pFileName);
    static std::string readFromFile(const std::string& pPath);
    static std::vector<std::string> getAllFileNames(const std::string& pDirectory);
    static std::vector<std::string> getAllFilePaths(const std::string& pDirectory);

    static bool exists(const std::string& pPath);
    static void setupDirectories(const std::string& pExecutePath);
};