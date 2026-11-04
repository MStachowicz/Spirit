#include "File.hpp"
#include "Logger.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace Utility
{
	std::vector<std::byte> File::read_bytes(const std::filesystem::path& p_path)
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(p_path, std::ios::binary | std::ios::ate);
		const auto size = static_cast<std::streamoff>(file.tellg());
		if (!std::in_range<std::size_t>(size) || !std::in_range<std::streamsize>(size))
			throw std::length_error("Binary file size is not representable.");

		std::vector<std::byte> bytes(static_cast<std::size_t>(size));
		file.seekg(0, std::ios::beg);
		if (!bytes.empty())
			file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
		file.close();
		return bytes;
	}

	void File::write_bytes(const std::filesystem::path& p_path, std::span<const std::byte> p_bytes)
	{
		if (!std::in_range<std::streamsize>(p_bytes.size()))
			throw std::length_error("Binary buffer size is not representable.");

		std::ofstream file;
		file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		file.open(p_path, std::ios::binary | std::ios::trunc);
		if (!p_bytes.empty())
			file.write(reinterpret_cast<const char*>(p_bytes.data()), static_cast<std::streamsize>(p_bytes.size()));
		file.close();
	}

	std::string File::read_from_file(const std::filesystem::path& p_path)
	{
		if (!exists(p_path))
		{
			LOG_ERROR(false, "File with path {} doesnt exist", p_path.string());
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
			ASSERT_FAIL("File not successfully read, exception thrown: {}", e.what());
		}
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