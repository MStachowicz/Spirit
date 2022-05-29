#pragma once

#include <functional>
#include <filesystem>
#include <string>
#include "Logger.hpp"

namespace util
{
    template <typename T>
    static constexpr auto toIndex(const T& pEnum) noexcept // Returns the underlying type. Used to convert enum types to indexes into arrays
    {
        return static_cast<std::underlying_type_t<T>>(pEnum);
    }

    class File
    {
    public:
        static inline std::filesystem::path executablePath;
        static inline std::filesystem::path rootDirectory;
        static inline std::filesystem::path GLSLShaderDirectory;
        static inline std::filesystem::path textureDirectory;

        static bool exists(const std::filesystem::path& pPath)  { return std::filesystem::exists(pPath); }
        //static bool exists(const std::string& pPath)            { return std::filesystem::exists(pPath); }

        static void initialise(const std::string& pExecutePath)
        {
            std::string exectuablePathStr = pExecutePath;
            std::replace(exectuablePathStr.begin(), exectuablePathStr.end(), '\\', '/');
            executablePath = exectuablePathStr;
            ZEPHYR_ASSERT(util::File::exists(executablePath), "Could not find the exectuable path")

            const auto &found = exectuablePathStr.find("Zephyr");
            ZEPHYR_ASSERT(found != std::string::npos, "Failed to find Zephyr string in the supplied executable path {}", executablePath.string()) // #C++20 if switched logger to use std::format, direct use of std::filesystem::path is available
            rootDirectory = exectuablePathStr.substr(0, found + 6); // offset substr by length of "Zephyr"
            ZEPHYR_ASSERT(util::File::exists(rootDirectory), "Could not find the rootDirectory path")

            GLSLShaderDirectory = rootDirectory.string() + "/source/OpenGLAPI/GLSL/";
            ZEPHYR_ASSERT(util::File::exists(GLSLShaderDirectory), "Could not find the GLSL shader directory")

            textureDirectory = rootDirectory.string() + "/source/Resources/Textures/";
            ZEPHYR_ASSERT(util::File::exists(textureDirectory), "Could not find the texture directory")

            LOG_INFO("Executable location initialised to: \"{}\"", executablePath.string());
            LOG_INFO("Root directory initialised to: \"{}\"", rootDirectory.string());
            LOG_INFO("Texture directory initialised to: \"{}\"", textureDirectory.string());
            LOG_INFO("GLSL Shader directory initialised to: \"{}\"", GLSLShaderDirectory.string());
        }

        static void ForEachFile(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction)
        {
            ZEPHYR_ASSERT(util::File::exists(pDirectory), "Directory does not exist, cannot iterate over its contents.")

            for (const auto& entry : std::filesystem::directory_iterator(pDirectory))
                pFunction(entry);
        }
        static void ForEachFileRecursive(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction)
        {
            ZEPHYR_ASSERT(util::File::exists(pDirectory), "Directory does not exist, cannot iterate over its contents.")

            for (const auto& entry : std::filesystem::recursive_directory_iterator(pDirectory))
                pFunction(entry);
        }
    };
}