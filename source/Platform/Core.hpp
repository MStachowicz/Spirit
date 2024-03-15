#pragma once

#include "glm/vec4.hpp"

namespace Platform
{
	class Window;

	class Core // Working name
	{
	public:
		// INIT
		static void initialise_directories();
		static void initialise_GLFW();
		static void initialise_OpenGL();
		static void initialise_ImGui(const Window& p_window);

		// DEINIT
		// Uninitialise ImGui and GLFW.
		// Make sure no further calls to these happens after cleanup.
		static void cleanup();
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