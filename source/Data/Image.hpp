#pragma once

#include <filesystem>

namespace Data
{
	// Pixel data of an image file. The data is loaded into memory and can be used to create a texture on the GPU.
	struct Image
	{
		Image(const std::filesystem::path& p_filePath) noexcept;
		~Image();
		Image(Image&& p_other) noexcept;
		Image& operator=(Image&& p_other) noexcept;
		Image(const Image& p_other)            = delete;
		Image& operator=(const Image& p_other) = delete;

		std::byte* data; // Pointer to the pixel data
		int width; // Width in pixels
		int height; // Height in pixels
		uint8_t number_of_channels; // Number of channels in the image. 4 = RGBA, 3 = RGB, 2 = RG, 1 = R
	};
}