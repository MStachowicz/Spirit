#include "Window.hpp"
#include "Input.hpp"

#include "Data/Image.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"
#include "Utility/Config.hpp"

#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"

namespace Platform
{
	Window::Window(const glm::vec2& p_screen_factor, Input& p_input_state) noexcept
		: m_last_position_windowed{0, 0} // init in body via set_position
		, m_last_size_windowed{0, 0}     // init in body
		, m_fullscreen{false}
		, m_VSync{true}
		, m_close_requested{false}
		, m_handle{nullptr}
		, m_input{p_input_state}
		, m_show_menu_bar{false}
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

		auto monitor = glfwGetPrimaryMonitor();
		int wa_xpos, wa_ypos, wa_width, wa_height;
		glfwGetMonitorWorkarea(monitor, &wa_xpos, &wa_ypos, &wa_width, &wa_height);
		const glm::ivec2 desired_size_windowed = {wa_width * p_screen_factor.x, wa_height * p_screen_factor.y};

		ASSERT(!m_fullscreen, "[WINDOW] Fullscreen construction not implemented. Add glfwCreateWindow call for fullscreen.");
		m_handle = glfwCreateWindow(desired_size_windowed.x, desired_size_windowed.y, Config::Is_Debug ? "Spirit - Debug" : "Spirit", NULL, NULL);
		ASSERT(m_handle != nullptr, "[WINDOW] Failed to construct Window");

		// Check the actual size of the window after creation. Per GLFW docs, this may not be the same as the requested size.
		if (size() != glm::uvec2(desired_size_windowed))
			set_size(desired_size_windowed);

		const glm::ivec2 desired_position_windowed =
		{
			wa_xpos + (wa_width  - m_last_size_windowed.x) * 0.5f,
			wa_ypos + (wa_height - m_last_size_windowed.y) * 0.5f
		};
		set_position(desired_position_windowed);

		set_VSync(m_VSync);
		glfwMakeContextCurrent(m_handle); // Set this window as the context for GL render calls.

		{ // Set the GLFW callbacks.
			// GLFW is a C library requiring static || global functions for callbacks.
			// We use the glfwSetWindowUserPointer and glfwGetWindowUserPointer to retrieve this instance of Window from the library and call the member functions.
			glfwSetWindowUserPointer(m_handle, this);
			glfwSetWindowCloseCallback(m_handle,        [](GLFWwindow* p_handle){((Window*)glfwGetWindowUserPointer(p_handle))->request_close();});
			glfwSetWindowSizeCallback(m_handle,         [](GLFWwindow* p_handle, int p_size_x, int p_size_y){((Window*)glfwGetWindowUserPointer(p_handle))->on_size_callback({p_size_x, p_size_y});});
			glfwSetWindowPosCallback(m_handle,          [](GLFWwindow* p_handle, int p_pos_x, int p_pos_y){((Window*)glfwGetWindowUserPointer(p_handle))->on_position_callback({p_pos_x, p_pos_y});});
			glfwSetWindowContentScaleCallback(m_handle, [](GLFWwindow* p_handle, float p_scale_x, float p_scale_y){((Window*)glfwGetWindowUserPointer(p_handle))->on_content_scale_callback((p_scale_x + p_scale_y) * 0.5f);});

			{ // INPUT CALLBACKS - Because we only have access to the WindowUserPointer via GLFW, we have to access the Input and set it from the window.
				glfwSetKeyCallback(m_handle,         [](GLFWwindow* p_handle, int p_key, int p_scancode, int p_action, int p_mode){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_key_press(p_key, p_scancode, p_action, p_mode);});
				glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* p_handle, int p_button, int p_action, int p_modifiers){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_mouse_press(p_button, p_action, p_modifiers);});
				glfwSetCursorPosCallback(m_handle,   [](GLFWwindow* p_handle, double p_mouse_new_x_position, double p_mouse_new_y_position){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_mouse_move(p_mouse_new_x_position, p_mouse_new_y_position);});
				glfwSetScrollCallback(m_handle,      [](GLFWwindow* p_handle, double p_x_offset, double p_y_offset){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_mouse_scroll(p_x_offset, p_y_offset);});

				// Because Input is constructed before we have access to the Window, its necessary to set the defaults from Input here after glfwCreateWindow.
				double x, y;
				glfwGetCursorPos(m_handle, &x, &y);
				m_input.m_handle = m_handle;
				m_input.m_cursor_position = {static_cast<float>(x), static_cast<float>(y)};
				m_input.set_cursor_mode(m_input.m_cursor_mode);
			}
		}
		{ // Set the taskbar icon for the window
			auto icon_image = Data::Image(Config::Texture_Directory / "Icons" / "Icon.png");
			GLFWimage icon;
			icon.pixels = (unsigned char*)(icon_image.data);
			icon.width  = icon_image.width;
			icon.height = icon_image.height;
			glfwSetWindowIcon(m_handle, 1, &icon);
		}

		LOG("[WINDOW] Created Window with resolution {}x{}", size().x, size().y);
	}
	Window::~Window() noexcept
	{
		glfwDestroyWindow(m_handle);
		m_handle = nullptr;
		LOG("[WINDOW] Closed window");
	}

	glm::uvec2 Window::size() const
	{
		int size_x, size_y;
		glfwGetWindowSize(m_handle, &size_x, &size_y);

		if (size_x == 0 || size_y == 0) // When the window is minimised the size can be 0.
			return {1, 1};
		else
			return {static_cast<unsigned int>(size_x), static_cast<unsigned int>(size_y)};
	}

	void Window::on_size_callback(const glm::uvec2& p_new_size)
	{
		if (p_new_size.x == 0 || p_new_size.y == 0)
			return;

		if (!m_fullscreen)
			m_last_size_windowed = p_new_size;

		if (ImGui::IsInitialised())
			ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(p_new_size.x), static_cast<float>(p_new_size.y));

		LOG("[WINDOW] Resized to {}x{} aspect: {}", size().x, size().y, aspect_ratio());
	}
	void Window::set_size(const glm::uvec2& p_new_size)
	{
		glfwSetWindowSize(m_handle, p_new_size.x, p_new_size.y);
		on_size_callback(size()); // Actual size may not be the same as requested call size to get the actual size.
	}
	glm::uvec2 Window::position() const
	{
		int pos_x, pos_y;
		glfwGetWindowPos(m_handle, &pos_x, &pos_y);
		return {static_cast<unsigned int>(pos_x), static_cast<unsigned int>(pos_y)};
	}
	void Window::on_position_callback(const glm::uvec2& new_position)
	{
		if (!m_fullscreen)
			m_last_position_windowed = new_position;
		LOG("[WINDOW] Moved to {}, {}", new_position.x, new_position.y, aspect_ratio());
	}
	void Window::set_position(const glm::uvec2& p_new_position)
	{
		glfwSetWindowPos(m_handle, p_new_position.x, p_new_position.y);
		on_position_callback(position());// Actual position may not be the same as requested call position to get the actual position.
	}

	void Window::request_close()
	{
		glfwSetWindowShouldClose(m_handle, GL_TRUE); // Ask GLFW to close this window
		m_close_requested = true;
	}
	float Window::aspect_ratio() const
	{
		const auto curr_size = size();
		return static_cast<float>(curr_size.x) / static_cast<float>(curr_size.y);
	}
	void Window::set_VSync(bool p_enabled)
	{
		glfwMakeContextCurrent(m_handle);
		// This function sets the swap interval for the current OpenGL context,
		// i.e. the number of screen updates to wait from the time glfwSwapBuffers was called before swapping the buffers and returning.
		glfwSwapInterval(p_enabled ? 1 : 0);
		m_VSync = p_enabled;
	}

	void Window::toggle_fullscreen()
	{
		m_fullscreen = !m_fullscreen;

		if (m_fullscreen)
		{
			const auto max_res = get_max_resolution();
			glfwSetWindowMonitor(m_handle, glfwGetPrimaryMonitor(), 0, 0, max_res.x, max_res.y, GLFW_DONT_CARE);
		}
		else // Windowed mode. Reuse the old size and position
			glfwSetWindowMonitor(m_handle, NULL, m_last_position_windowed.x, m_last_position_windowed.y, m_last_size_windowed.x, m_last_size_windowed.y, GLFW_DONT_CARE);

		on_position_callback(position());
		on_size_callback(size());

		LOG("[WINDOW] Set to fullscreen. Position: {},{} Resolution: {}x{} Aspect ratio: {}", position().x, position().y, size().x, size().y, aspect_ratio());
	}

	void Window::swap_buffers()
	{
		glfwSwapBuffers(m_handle);
	}

	void Window::start_ImGui_frame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		// At the start of an ImGui frame, push a window the size of viewport to allow docking other ImGui windows to.
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		auto imgui_window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;

		if (m_show_menu_bar) imgui_window_flags |= ImGuiWindowFlags_MenuBar;

		ImGui::Begin("root_dock", nullptr, imgui_window_flags);
		ImGui::PopStyleVar(3);


		ImGuiID root_dock_ID = ImGui::GetID("root_dock");
		ImGui::DockSpace(root_dock_ID, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);

		static bool first_time = true;
		if (first_time)
		{
			first_time = false;
			ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

			// Clear the previous layout and add a root node the size of the viewport.
			ImGui::DockBuilderRemoveNode(root_dock_ID);
			ImGui::DockBuilderAddNode(root_dock_ID, dockspace_flags | ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_KeepAliveOnly);
			ImGui::DockBuilderSetNodeSize(root_dock_ID, viewport->Size);

			// Split the dockspace into 4 nodes by splitting root_dock_ID horizontally and then splitting the right node vertically.
			ImGuiID dock_id_left, dock_id_right;
			ImGui::DockBuilderSplitNode(root_dock_ID, ImGuiDir_Left, 0.2f, &dock_id_left, &dock_id_right);
			ImGuiID dock_id_down, dock_id_up;
			ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.15f, &dock_id_down, &dock_id_up);
			ImGui::DockBuilderFinish(root_dock_ID);

			ASSERT_THROW(dock_id_left == 1 && dock_id_right == 2 && dock_id_up == 3 && dock_id_down == 4,
				"Dock direction IDs are not as expected. We use these hard coded numbers for the layout of the editor with SetNextWindowDockID()! Can create a mapping if the order changes.");
		}
	}
	void Window::end_ImGui_frame()
	{
		ImGui::End(); // root_dock
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Window::on_content_scale_callback(float new_scale)
	{
		if (ImGui::IsInitialised())
			ImGui::GetIO().FontGlobalScale = new_scale;
	}
	float Window::content_scale() const
	{
		float xscale, yscale;
		glfwGetWindowContentScale(m_handle, &xscale, &yscale);
		return (xscale + yscale) * 0.5f;
	}

	glm::uvec2 Window::get_max_resolution()
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		return {static_cast<unsigned int>(mode->width), static_cast<unsigned int>(mode->height)};
	}
} // namespace Platform