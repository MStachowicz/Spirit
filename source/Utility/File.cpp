#include "File.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace Utility
{
	Image::Image(const std::filesystem::path& p_path) noexcept
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
	Image::~Image() noexcept
	{
		if (m_data)
			stbi_image_free(m_data);
	}

	Image::Image(Image&& p_other) noexcept
		: m_data{std::exchange(p_other.m_data, nullptr)}
		, m_filepath{std::exchange(p_other.m_filepath, {})}
		, m_width{std::exchange(p_other.m_width, 0)}
		, m_height{std::exchange(p_other.m_height, 0)}
		, m_number_of_channels{std::exchange(p_other.m_number_of_channels, 0)}
	{}

	Image& Image::operator=(Image&& p_other) noexcept
	{
		if (this != &p_other)
		{
			m_data               = std::exchange(p_other.m_data, nullptr);
			m_filepath           = std::exchange(p_other.m_filepath, {});
			m_width              = std::exchange(p_other.m_width, 0);
			m_height             = std::exchange(p_other.m_height, 0);
			m_number_of_channels = std::exchange(p_other.m_number_of_channels, 0);
		}
		return *this;
	}

	std::string File::read_from_file(const std::filesystem::path& p_path)
	{
		if (!exists(p_path))
		{
			LOG_ERROR("File with path {} doesnt exist", p_path.string());
			return "";
		}

		std::ifstream file;
		// ensure ifstream objects can throw exceptions
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			file.open(p_path);
			std::stringstream stream;
			stream << file.rdbuf();
			file.close();

			return stream.str();
		}
		catch (std::ifstream::failure& e)
		{
			ASSERT_THROW(false, "File not successfully read, exception thrown: {}", e.what());
		}

		return "";
	}

	void File::foreach_file(const std::filesystem::path& p_directory, const std::function<void(const std::filesystem::directory_entry& p_entry)>& p_function)
	{
		ASSERT(Utility::File::exists(p_directory), "Directory {} doesn't exist, cannot iterate over its contents.", p_directory.string());

		for (const auto& entry : std::filesystem::directory_iterator(p_directory))
			p_function(entry);
	}
	void File::foreach_file_recursive(const std::filesystem::path& p_directory, const std::function<void(const std::filesystem::directory_entry& p_entry)>& p_function)
	{
		ASSERT(Utility::File::exists(p_directory), "Directory {} doesn't exist, cannot iterate over its contents.", p_directory.string());

		for (const auto& entry : std::filesystem::recursive_directory_iterator(p_directory))
			p_function(entry);
	}
} // namespace Utility