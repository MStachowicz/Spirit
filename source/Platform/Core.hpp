#pragma once

#include "glm/vec4.hpp"

#include <filesystem>

namespace Platform
{
	class Window;

	enum class FileDialogType : uint8_t
	{
		Open,
		Save
	};
	enum class FileDialogFilter : uint8_t
	{
		All,
		Scene
	};
	// Open a native file dialog for this platform.
	//@param p_filter The filter to apply to the dialog. Usage examples:
	// All files = "All Files\0*.*\0"
	// Text files = "Text Files\0*.txt\0"
	// Multiple filters = "Text Files\0*.txt\0Image Files\0*.png;*.jpg\0"
	//@return The path to the file selected by the user. If the user cancels the dialog, an empty path is returned.
	std::filesystem::path file_dialog(
		FileDialogType p_type,
		FileDialogFilter p_filter,
		const char* p_title,
		const std::filesystem::path& p_start_path = {});

	class Core // Working name
	{
	public:
		// INIT
		static void initialise_directories();
		static void initialise_GLFW();
		static void initialise_OpenGL();
		static void initialise_ImGui(const Window& p_window);

		// DEINIT
		static void deinitialise_ImGui();
		static void deinitialise_GLFW();

		static bool is_dark_mode(); // Is the OS running in dark mode

		struct Theme
		{
			Theme();

			void draw_theme_editor_UI();

			glm::vec4 background;
			glm::vec4 accent;
			glm::vec4 general_text;
			glm::vec4 warning_text;
			glm::vec4 error_text;
			glm::vec4 success_text;
			bool dark_mode;
		};
		static inline Theme s_theme = {};
	};
}