#pragma once

#include "ResourceManager.hpp"

#include <filesystem>
#include <functional>
#include <string>

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
	};
} // namespace Utility