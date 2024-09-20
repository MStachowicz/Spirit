#include "Image.hpp"

#include "Utility/Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <utility>

namespace Data
{
	Image::Image(const std::filesystem::path& p_filePath) noexcept
	{
		ASSERT(std::filesystem::exists(p_filePath), "[FILE][TEXTURE] Path '{}' does not exist.", p_filePath.string());

		// OpenGL expects 0 coordinate on y-axis to be the bottom side of the image, images usually have 0 at the top of y-axis
		// Flip textures here to account for this.
		stbi_set_flip_vertically_on_load(false);

		int components     = 0;
		data               = (std::byte*)(stbi_load(p_filePath.string().c_str(), &width, &height, &components, 0));
		number_of_channels = static_cast<uint8_t>(components);
		ASSERT(data != nullptr, "Failed to load texture at path '{}'", p_filePath.string());
	}
	Image::~Image()
	{
		if (data != nullptr)
			stbi_image_free(data);
	}
	Image::Image(Image&& p_other) noexcept
	    : data{std::exchange(p_other.data, nullptr)}
	    , width{std::exchange(p_other.width, 0)}
	    , height{std::exchange(p_other.height, 0)}
	    , number_of_channels{std::exchange(p_other.number_of_channels, 0)}
	{
	}
	Image& Image::operator=(Image&& p_other) noexcept
	{
		if (this != &p_other)
		{
			data               = std::exchange(p_other.data, nullptr);
			width              = std::exchange(p_other.width, 0);
			height             = std::exchange(p_other.height, 0);
			number_of_channels = std::exchange(p_other.number_of_channels, 0);
		}
		return *this;
	}
} // namespace Data