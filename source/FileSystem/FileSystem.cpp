#include "FileSystem.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

std::string File::readFromFile(const std::string& pPath)
{
    if (!exists(pPath))
    {
       LOG_ERROR("File with path {} doesnt exist", pPath);
       return "";
    }
    
    std::ifstream file;
    // ensure ifstream objects can throw exceptions
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        file.open(pPath);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();

        return stream.str();
    }
    catch (std::ifstream::failure e)
    {
        LOG_ERROR("File not successfully read, exception thrown: {}", e.what());
    }

    return "";
}

bool File::exists(const std::string& pPath)
{
    return std::filesystem::exists(pPath);
}