#pragma once

#include "ResourceManager.hpp"

// STD
#include <filesystem>
#include <functional>
#include <string>

namespace Utility
{
    // Represents an image file on the disk.
    // On construction will be loaded into memory using STB_image.
    // This is a relatively expensive object to move around so the preferred use is to access Image instances via s_image_files ResourceManager.
    class Image
    {
        std::byte* m_data;

    public:
        std::filesystem::path m_filepath;
        int m_width;  // Width in pixels
        int m_height; // Height in pixels
        int m_number_of_channels; // RGBA

        Image(const std::filesystem::path& p_path);
        ~Image();

        Image(const Image& p_other) = delete;
        Image(Image&& p_other) = delete;
        Image& operator=(const Image& p_other) = delete;
        Image& operator=(Image&& p_other) = delete;

        // Raw access to the image pixel data. Access for read only.
        std::byte* get_data() const { return m_data; }
        // Return a display-friendly name for this image.
        std::string name() const { return m_filepath.stem().string(); };
    };

    // File access helper.
    class File
    {
    public:
        static bool exists(const std::filesystem::path& pPath) { return std::filesystem::exists(pPath); }
        static void forEachFile(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction);
        static void forEachFileRecursive(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction);

        static std::string readFromFile(const std::filesystem::path& pPath);

        static inline Utility::ResourceManager<Utility::Image> s_image_files;
        using ImageRef = Utility::ResourceRef<Utility::Image>;
    };
} // namespace Utility