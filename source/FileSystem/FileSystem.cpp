#include "FileSystem.hpp"

#include "Logger.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

std::string File::executablePath;
std::string File::rootDirectory;
std::string File::GLSLShaderDirectory;
std::string File::textureDirectory;

std::string File::removeFileExtension(const std::string& pFileName)
{
    if (pFileName == "." || pFileName == "..")
        return pFileName;

    size_t pos = pFileName.find_last_of("\\/.");
    if (pos != std::string::npos && pFileName[pos] == '.')
        return pFileName.substr(0, pos);

    return pFileName;
}

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

std::vector<std::string> File::getAllFileNames(const std::string& pDirectory)
{
    std::vector<std::string> files;

    for (const auto& file : std::filesystem::directory_iterator(pDirectory))
        files.push_back(file.path().filename().string());

    return files;
}

std::vector<std::string> File::getAllFilePaths(const std::string& pDirectory)
{
    std::vector<std::string> paths;

    for (const auto& file : std::filesystem::directory_iterator(pDirectory))
    {
        paths.push_back(file.path().string());
    }

    return paths;
}

std::vector<std::filesystem::directory_entry> File::getFiles(const std::string& pDirectory)
{
    std::vector<std::filesystem::directory_entry> files;

    for (const auto& file : std::filesystem::directory_iterator(pDirectory))
    {
        files.push_back(file);
    }

    return files;
}

bool File::exists(const std::string& pPath)
{
    return std::filesystem::exists(pPath);
}

void File::setupDirectories(const std::string& pExecutePath)
{
    ZEPHYR_ASSERT(!pExecutePath.empty(), "Cannot initialise directories with no executable path given")
    executablePath = pExecutePath;
    std::replace(executablePath.begin(), executablePath.end(), '\\', '/');

    const auto& found = executablePath.find("Zephyr");
    ZEPHYR_ASSERT(found != std::string::npos, "Failed to find Zephyr in the supplied executable path {}", executablePath)

    rootDirectory = executablePath.substr(0, found + 6); // offset substr by length of "Zephyr"
    LOG_INFO("Root directory initialised to \"{}\"", rootDirectory);

    GLSLShaderDirectory = rootDirectory + "/source/Renderer/OpenGL/GLSL/";
    LOG_INFO("Shader directory initialised to \"{}\"", GLSLShaderDirectory);

    textureDirectory = rootDirectory + "/source/Resources/Textures/";
    LOG_INFO("Texture directory initialised to \"{}\"", textureDirectory);
};