#include "File.hpp"

#include "Logger.hpp"

#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace Utility
{
    std::filesystem::path File::executablePath;
    std::filesystem::path File::rootDirectory;
    std::filesystem::path File::GLSLShaderDirectory;
    std::filesystem::path File::textureDirectory;
    std::filesystem::path File::modelDirectory;

    void File::setupDirectories(const std::string& pExecutePath)
    {
        ASSERT(!pExecutePath.empty(), "Cannot initialise directories with no executable path given");
        ASSERT(exists(pExecutePath), "path to exe not found");
        executablePath = pExecutePath;

        const auto& found = executablePath.string().find("Zephyr");
        ASSERT(found != std::string::npos, "Failed to find Zephyr in the supplied executable path {}", executablePath.string());

        rootDirectory = executablePath.string().substr(0, found + 6); // offset substr by length of "Zephyr"
        LOG_INFO("Root directory initialised to '{}'", rootDirectory.string());

        GLSLShaderDirectory = std::filesystem::path(rootDirectory / "source" / "OpenGL" / "GLSL");
        LOG_INFO("Shader directory initialised to '{}'", GLSLShaderDirectory.string());

        textureDirectory = std::filesystem::path(rootDirectory / "source" / "Resources" / "Textures");
        LOG_INFO("Texture directory initialised to '{}'", textureDirectory.string());

        modelDirectory = std::filesystem::path(rootDirectory / "source" / "Resources" / "Models");
        LOG_INFO("Model directory initialised to '{}'", modelDirectory.string());
    };

    std::string File::readFromFile(const std::filesystem::path& pPath)
    {
        if (!exists(pPath))
        {
            LOG_ERROR("File with path {} doesnt exist", pPath.string());
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

    void File::forEachFile(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction)
    {
        ASSERT(Utility::File::exists(pDirectory), "Directory {} doesn't exist, cannot iterate over its contents.", pDirectory.string());

        for (const auto& entry : std::filesystem::directory_iterator(pDirectory))
            pFunction(entry);
    }
    void File::forEachFileRecursive(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction)
    {
        ASSERT(Utility::File::exists(pDirectory), "Directory {} doesn't exist, cannot iterate over its contents.", pDirectory.string());

        for (const auto& entry : std::filesystem::recursive_directory_iterator(pDirectory))
            pFunction(entry);
    }
} // namespace Utility