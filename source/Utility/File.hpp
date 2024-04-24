#pragma once

#include "ResourceManager.hpp"

#include <glm/vec2.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <type_traits>
#include <fstream>

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
		uint8_t m_number_of_channels; // Number of channels in the image. 4 = RGBA, 3 = RGB, 2 = RG, 1 = R

		Image(const std::filesystem::path& p_path) noexcept;
		~Image() noexcept;
		Image(Image&& p_other) noexcept;
		Image& operator=(Image&& p_other) noexcept;
		Image(const Image& p_other)            = delete;
		Image& operator=(const Image& p_other) = delete;

		// Raw access to the image pixel data. Access for read only.
		std::byte* get_data() const { return m_data; }
		// Return a display-friendly name for this image.
		std::string name() const { return m_filepath.stem().string(); };

		glm::uvec2 resolution() const { return {m_width, m_height}; }
	};

	// File access helper.
	class File
	{
	public:
		static bool exists(const std::filesystem::path& p_path) { return std::filesystem::exists(p_path); }
		static void foreach_file(const std::filesystem::path& p_directory, const std::function<void(const std::filesystem::directory_entry& p_entry)>& p_function);
		static void foreach_file_recursive(const std::filesystem::path& p_directory, const std::function<void(const std::filesystem::directory_entry& p_entry)>& p_function);

		static std::string read_from_file(const std::filesystem::path& p_path);

		static inline Utility::ResourceManager<Utility::Image> s_image_files;
		using ImageRef = Utility::ResourceRef<Utility::Image>;
	};
} // namespace Utility