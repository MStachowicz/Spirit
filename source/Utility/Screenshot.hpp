#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace Utility
{
	// Shared screenshot save directory.
	// On first use, opens a folder dialog and caches the selected path for subsequent saves.
	// If the user cancels the dialog, returns an empty path and will prompt again on the next call.
	// Callers are expected to check `.empty()` before attempting to use the directory.
	std::filesystem::path& screenshot_directory();

	// Save raw pixel data to a timestamped PNG file.
	//@param p_directory The directory to save the image to.
	//@param p_pixels Raw pixel data in row order provided by the caller.
	//@param p_width Image width in pixels.
	//@param p_height Image height in pixels.
	//@param p_channels Number of channels (e.g. 4 for RGBA).
	//@param p_flip_y Whether to vertically flip the image while encoding the PNG.
	//@param p_prefix Filename prefix before the timestamp (e.g. "screenshot", "pick_debug").
	void save_pixels_to_file(const std::filesystem::path& p_directory, const std::vector<std::byte>& p_pixels, int p_width, int p_height, int p_channels, bool p_flip_y = false, const char* p_prefix = "screenshot");
} // namespace Utility