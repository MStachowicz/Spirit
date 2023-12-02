#pragma once

#include "glm/vec2.hpp"

typedef struct GLFWwindow GLFWwindow;

namespace Platform
{
	class Input;

	class Window
	{
		friend class Core; // Core::initialise_imgui requires access to m_handle.
		glm::ivec2 m_size_fullscreen;     // the most up-to-date size of the window in fullscreen.
		glm::ivec2 m_position_fullscreen; // the most up-to-date position of the window in fullscreen.
		glm::ivec2 m_size_windowed;       // the most up-to-date size of the window in windowed.
		glm::ivec2 m_position_windowed;   // the most up-to-date position of the window in windowed.

		bool m_fullscreen;
		float m_aspect_ratio;
		bool m_VSync; // Whether VSync is on for the window.
		bool m_close_requested;
		GLFWwindow* m_handle;
		Input& m_input; // Window requires access to Input to use it in GLFW callbacks from glfwGetWindowUserPointer.

	public:
		// Creates a OS window of p_width and p_height.
		// Takes an Input and sets its GLFW callback functions. Input depends on a Window.
		// Window construction requires GLFW and ImGui to be initialised before.
		Window(const int& p_width, const int& p_height, Input& p_input_state) noexcept;
		~Window() noexcept;

		void set_VSync(bool p_enabled);
		bool get_VSync() const { return m_VSync; };

		glm::ivec2 size() const { return m_fullscreen ? m_size_fullscreen : m_size_windowed; };
		void set_size(glm::ivec2 p_new_size);

		glm::ivec2 position() const { return m_fullscreen ? m_position_fullscreen : m_position_windowed; };
		void set_position(glm::ivec2 p_new_position);

		void toggle_fullscreen();
		void swap_buffers();
		void start_ImGui_frame();
		void end_ImGui_frame();
		void request_close();
		bool close_requested() const { return m_close_requested; };
		float aspect_ratio()   const { return m_aspect_ratio; };

		// Returns the hight of the window title bar in pixels.
		static int get_window_title_bar_height();
		// Get the max resolution of the primary monitor.
		static glm::ivec2 get_max_resolution();
	};
}