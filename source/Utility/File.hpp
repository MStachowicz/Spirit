#pragma once

#include "ResourceManager.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace Utility
{
	// File access helper.
	class File
	{
	public:
		static bool exists(const std::filesystem::path& p_path) { return std::filesystem::exists(p_path); }
		static void foreach_file(const std::filesystem::path& p_directory, const std::function<void(const std::filesystem::directory_entry& p_entry)>& p_function);
		static void foreach_file_recursive(const std::filesystem::path& p_directory, const std::function<void(const std::filesystem::directory_entry& p_entry)>& p_function);
		static std::string read_from_file(const std::filesystem::path& p_path);

		// Binary file operations propagate I/O errors to the caller.
		static std::vector<std::byte> read_bytes(const std::filesystem::path& p_path);
		static void write_bytes(const std::filesystem::path& p_path, std::span<const std::byte> p_bytes);
	};
} // namespace Utility