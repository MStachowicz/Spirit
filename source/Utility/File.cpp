#include "File.hpp"

#include "Logger.hpp"
#include "Utility/Config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace Utility
{
    Image::Image(const std::filesystem::path& p_path)
        : m_data{nullptr}
        , m_filepath{p_path}
        , m_width{0}
        , m_height{0}
        , m_number_of_channels{0}
    {
        ASSERT(File::exists(p_path), "[FILE][IMAGE] Path '{}' does not exist.", p_path.string());

        // OpenGL expects 0 coordinate on y-axis to be the bottom side of the image, images usually have 0 at the top of y-axis
        // Flip textures here to account for this.
        stbi_set_flip_vertically_on_load(false);

        m_data = (std::byte*)(stbi_load(m_filepath.string().c_str(), &m_width, &m_height, &m_number_of_channels, 0));
        ASSERT(m_data != nullptr, "Failed to load texture at path '{}'", m_filepath.string());
    }
    Image::~Image()
    {
        stbi_image_free(m_data);
    }

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