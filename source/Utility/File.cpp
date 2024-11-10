#include "File.hpp"
#include "Logger.hpp"

#include <fstream>
#include <sstream>

namespace Utility
{
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