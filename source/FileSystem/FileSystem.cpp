#include "FileSystem.hpp"

#include "Logger.hpp"
#define STB_IMAGE_IMPLEMENTATION // This modifies the header such that it only contains the relevant definition source code
#include "stb_image.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

std::string File::executablePath;
std::string File::rootDirectory;
std::string File::GLSLShaderDirectory;
std::string File::textureDirectory;

File::Texture::~Texture()
{
    stbi_image_free(mData);
}

std::string File::readFromFile(const std::string &pPath)
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

File::Texture File::getTexture(const std::string &pFileName)
{
    const std::string path = File::textureDirectory + pFileName;
    ZEPHYR_ASSERT(File::exists(path), "The texture file with path {} could not be found.", path)
    int width, height, numberOfChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &numberOfChannels, 0);
    ZEPHYR_ASSERT(data != nullptr, "Failed to load texture")
    // Construct in return statement to avoid Texture::mData being garbage after copying stack copy
    return {width, height, numberOfChannels, data};
}

std::vector<std::string> File::getAllFileNames(const std::string &pDirectory)
{
    std::vector<std::string> files;

    for (const auto &file : std::filesystem::directory_iterator(pDirectory))
        files.push_back(file.path().filename().string());

    return files;
}

std::vector<std::string> File::getAllFilePaths(const std::string &pDirectory)
{
    std::vector<std::string> paths;

    for (const auto &file : std::filesystem::directory_iterator(pDirectory))
    {
        paths.push_back(file.path().string());
        LOG_INFO(file.path().string());
    }

    return paths;
}

bool File::exists(const std::string &pPath)
{
    return std::filesystem::exists(pPath);
}

void File::setupDirectories(const std::string &pExecutePath)
{
    ZEPHYR_ASSERT(!pExecutePath.empty(), "Cannot initialise directories with no executable path given")
    executablePath = pExecutePath;
    std::replace(executablePath.begin(), executablePath.end(), '\\', '/');

    const auto &found = executablePath.find("Zephyr");
    ZEPHYR_ASSERT(found != std::string::npos, "Failed to find Zephyr in the supplied executable path {}", executablePath)

    rootDirectory = executablePath.substr(0, found + 6); // offset substr by length of "Zephyr"
    LOG_INFO("Root directory initialised to \"{}\"", rootDirectory);

    GLSLShaderDirectory = rootDirectory + "/source/OpenGLAPI/GLSL/";
    LOG_INFO("Shader directory initialised to \"{}\"", GLSLShaderDirectory);

    textureDirectory = rootDirectory + "/source/Resources/Textures/";
    LOG_INFO("Texture directory initialised to \"{}\"", textureDirectory);

    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
};