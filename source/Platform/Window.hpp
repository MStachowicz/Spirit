#pragma once

#include "glm/vec2.hpp"

typedef struct GLFWwindow GLFWwindow;

namespace Platform
{
	class Input;

	class Window
	{
		friend class Core; // Core::initialise_imgui requires access to m_handle.
		glm::uvec2 m_last_position_windowed; // the most up-to-date position of the window in windowed.
		glm::uvec2 m_last_size_windowed;     // the most up-to-date size of the window in windowed.

		bool m_fullscreen;
		bool m_VSync; // Whether VSync is on for the window.
		bool m_close_requested;
		GLFWwindow* m_handle;
		Input& m_input; // Window requires access to Input to use it in GLFW callbacks from glfwGetWindowUserPointer.

	public:
		uint16_t m_framerate_cap; // Target framerate cap (0 = unlimited).

		// Creates a OS window of p_width and p_height.
		// Takes an Input and sets its GLFW callback functions. Input depends on a Window.
		// Window construction requires GLFW and ImGui to be initialised before.
		Window(const glm::vec2& p_screen_factor, Input& p_input_state) noexcept;
		~Window() noexcept;

		void set_VSync(bool p_enabled);
		bool get_VSync() const { return m_VSync; };

		glm::uvec2 size() const;
		void on_size_callback(const glm::uvec2& p_new_size);
		void set_size(const glm::uvec2& p_new_size);

		glm::uvec2 position() const;
		void on_position_callback(const glm::uvec2& p_new_position);
		void set_position(const glm::uvec2& p_new_position);

		void toggle_fullscreen();
		void swap_buffers();
		void start_ImGui_frame();
		void end_ImGui_frame();
		void request_close();
		bool close_requested() const { return m_close_requested; };
		float aspect_ratio() const;

		void on_content_scale_callback(float new_scale);
		float content_scale() const;

		// Get the max resolution of the primary monitor.
		static glm::uvec2 get_max_resolution();
		static uint16_t get_primary_monitor_refresh_rate();

		bool m_show_menu_bar;
	};
}