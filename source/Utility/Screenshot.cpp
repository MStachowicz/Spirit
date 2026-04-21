#include "Screenshot.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Config.hpp"

#include "Platform/Core.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <array>
#include <chrono>
#include <ctime>
#include <format>
#include <string>

namespace Utility
{
	std::string screenshot_timestamp()
	{
		const auto now      = std::chrono::system_clock::now();
		const auto now_time = std::chrono::system_clock::to_time_t(now);

		std::tm local_time{};

		#if defined(_WIN32) && !defined(__MINGW32__)
			if (localtime_s(&local_time, &now_time) != 0)
				return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
		#elif defined(__APPLE__) || defined(__unix__)
			if (localtime_r(&now_time, &local_time) == nullptr)
				return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
		#else
			const std::tm* current_local_time = std::localtime(&now_time);
			if (current_local_time == nullptr)
				return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());

			local_time = *current_local_time;
		#endif

		std::array<char, sizeof("00-00-00 00-00-00")> timestamp{};
		if (std::strftime(timestamp.data(), timestamp.size(), "%y-%m-%d %H-%M-%S", &local_time) == 0)
			return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());

		return timestamp.data();
	}

	std::filesystem::path& screenshot_directory()
	{
		static std::filesystem::path s_directory;

		if (s_directory.empty())
		{
			auto start_path = Config::Is_Debug ? Config::Source_Directory / "screenshots" : std::filesystem::current_path();
			auto result = Platform::folder_dialog("Select screenshot save directory", start_path);
			if (!result.empty())
				s_directory = std::move(result);
		}

		return s_directory;
	}

	void save_pixels_to_file(const std::filesystem::path& p_directory, const std::vector<std::byte>& p_pixels, int p_width, int p_height, int p_channels, bool p_flip_y, const char* p_prefix)
	{
		if (p_width <= 0 || p_height <= 0 || p_pixels.empty())
			return;

		const std::string timestamp = screenshot_timestamp();
		const int stride            = p_width * p_channels;
		auto filepath               = p_directory / std::format("{} {}.png", p_prefix, timestamp);

		// If a file with the same name already exists append _N.
		if (std::filesystem::exists(filepath))
		{
			unsigned int counter = 1;
			while (std::filesystem::exists(filepath))
				filepath = p_directory / std::format("{} {}_{}.png", p_prefix, timestamp, counter++);
		}

		// stb_image_write uses process-global flip state, so this path assumes screenshot writes stay single-threaded.
		stbi_flip_vertically_on_write(p_flip_y ? 1 : 0);
		const int write_succeeded = stbi_write_png(filepath.string().c_str(), p_width, p_height, p_channels, p_pixels.data(), stride);
		stbi_flip_vertically_on_write(0);

		if (write_succeeded)
			LOG("Screenshot saved: {}", filepath.string());
		else
			LOG("Failed to save screenshot to: {}", filepath.string());
	}
} // namespace Utility