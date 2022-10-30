#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace std
{
    template <class>
    class function;
}

namespace Utility
{
    // File access helper.
    class File
    {
    public:
        static std::filesystem::path executablePath;
        static std::filesystem::path rootDirectory;
        static std::filesystem::path GLSLShaderDirectory;
        static std::filesystem::path textureDirectory;
        static void setupDirectories(const std::string& pExecutePath);

        static bool exists(const std::filesystem::path& pPath) { return std::filesystem::exists(pPath); }
        static void forEachFile(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction);
        static void forEachFileRecursive(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction);

        static std::string readFromFile(const std::filesystem::path& pPath);
    };
} // namespace Utility