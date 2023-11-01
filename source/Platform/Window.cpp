#include "Window.hpp"
#include "Input.hpp"

#include "Logger.hpp"
#include "Utility.hpp"
#include "File.hpp"
#include "Config.hpp"

#include <iostream>

#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Platform
{
    Window::Window(const int& p_width, const int& p_height, Input& p_input_state) noexcept
        : m_size_fullscreen{get_max_resolution()}
        , m_position_fullscreen{0, 0}
        , m_size_windowed{p_width, p_height}
        , m_position_windowed{0, get_window_title_bar_height()} // Offset starting position by height of title bar.
        , m_fullscreen{false}
        , m_aspect_ratio{m_fullscreen ? static_cast<float>(m_size_fullscreen.x) / static_cast<float>(m_size_fullscreen.y) : static_cast<float>(m_size_windowed.x) / static_cast<float>(m_size_windowed.y)}
        , m_VSync{false}
        , m_close_requested{false}
        , m_handle{nullptr}
        , m_input{p_input_state}
    {
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        const auto window_size = size();
        const auto window_position = position();
        m_handle = glfwCreateWindow(window_size.x, window_size.y, "Zephyr", m_fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
        ASSERT(m_handle != nullptr, "[WINDOW] Failed to construct Window");

        glfwMakeContextCurrent(m_handle); // Set this window as the context for GL render calls.
        glfwSetWindowPos(m_handle, window_position.x, window_position.y);
        glfwSetWindowSize(m_handle, window_size.x, window_size.y); // glfwCreateWindow size requests are not hard constraints. Have to ensure size was set by calling glfwSetWindowSize.
        set_VSync(m_VSync);

        { // Set the GLFW callbacks.
            // GLFW is a C library requiring static || global functions for callbacks.
            // We use the glfwSetWindowUserPointer and glfwGetWindowUserPointer to retrieve this instance of Window from the library and call the member functions.
            glfwSetWindowUserPointer(m_handle, this);
            glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* p_handle){((Window*)glfwGetWindowUserPointer(p_handle))->request_close();});
            glfwSetWindowSizeCallback(m_handle,  [](GLFWwindow* p_handle, int p_size_x, int p_size_y){((Window*)glfwGetWindowUserPointer(p_handle))->set_size({p_size_x, p_size_y});});
            glfwSetWindowPosCallback(m_handle,   [](GLFWwindow* p_handle, int p_pos_x, int p_pos_y){((Window*)glfwGetWindowUserPointer(p_handle))->set_position({p_pos_x, p_pos_y});});

            { // INPUT CALLBACKS - Because we only have access to the WindowUserPointer via GLFW, we have to access the Input and set it from the window.
                glfwSetKeyCallback(m_handle,         [](GLFWwindow* p_handle, int p_key, int p_scancode, int p_action, int p_mode){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_key_press(p_key, p_scancode, p_action, p_mode);});
                glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* p_handle, int p_button, int p_action, int p_modifiers){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_mouse_press(p_button, p_action, p_modifiers);});
                glfwSetCursorPosCallback(m_handle,   [](GLFWwindow* p_handle, double p_mouse_new_x_position, double p_mouse_new_y_position){((Window*)glfwGetWindowUserPointer(p_handle))->m_input.glfw_mouse_move(p_mouse_new_x_position, p_mouse_new_y_position);});

                // Because Input is constructed before we have access to the Window, its necessary to set the defaults from Input here after glfwCreateWindow.
                double x, y;
                glfwGetCursorPos(m_handle, &x, &y);
                m_input.m_handle = m_handle;
                m_input.m_cursor_position = {static_cast<float>(x), static_cast<float>(y)};
                m_input.set_cursor_mode(m_input.m_cursor_mode);
            }
        }
        { // Set the taskbar icon for the window
            const auto icon_path = Config::Texture_Directory / "Icons" / "Icon.png";
            auto icon_image      = Utility::File::s_image_files.get_or_create([&icon_path](const Utility::Image& p_image) { return p_image.m_filepath == icon_path; }, icon_path);
            GLFWimage icon;
            icon.pixels = (unsigned char*)(icon_image->get_data());
            icon.width  = icon_image->m_width;
            icon.height = icon_image->m_height;
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

    void Window::set_size(glm::ivec2 p_new_size)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = std::round(ImGui::GetMainViewport()->DpiScale);

        if (m_fullscreen)
        {
            m_size_fullscreen = p_new_size;
            m_aspect_ratio    = static_cast<float>(m_size_fullscreen.x) / static_cast<float>(m_size_fullscreen.y);
            io.DisplaySize    = ImVec2(static_cast<float>(m_size_fullscreen.x), static_cast<float>(m_size_fullscreen.y));
        }
        else
        {
            m_size_windowed = p_new_size;
            m_aspect_ratio  = static_cast<float>(m_size_windowed.x) / static_cast<float>(m_size_windowed.y);
            io.DisplaySize    = ImVec2(static_cast<float>(m_size_windowed.x), static_cast<float>(m_size_windowed.y));
        }

        //mWindowResizeEvent.dispatch(new_size.x, new_size.y);

        LOG("[WINDOW] Resized to {}x{} aspect: {}", size().x, size().y, aspect_ratio());
    }
    void Window::set_position(glm::ivec2 p_new_position)
    {
        if (m_fullscreen)
            m_position_fullscreen = p_new_position;
        else
            m_position_windowed = p_new_position;

        LOG("[WINDOW] Moved to {}, {}", position().x, position().y, aspect_ratio());
    }

    void Window::request_close()
    {
        glfwSetWindowShouldClose(m_handle, GL_TRUE); // Ask GLFW to close this window
        m_close_requested = true;
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
            set_position({0, 0});
            set_size(max_res);
            LOG("[WINDOW] Set to fullscreen. Position: {},{} Resolution: {}x{} Aspect ratio: {}", 0, 0, m_size_fullscreen.x, m_size_fullscreen.y, m_aspect_ratio);
        }
        else // Windowed mode. Reuse the old size and position
        {
            glfwSetWindowMonitor(m_handle, NULL, m_position_windowed.x, m_position_windowed.y, m_size_windowed.x, m_size_windowed.y, GLFW_DONT_CARE);
            LOG("[WINDOW] Set to windowed mode. Position: {},{} Resolution: {}x{} Aspect ratio: {}", 0, 0, m_size_windowed.x, m_size_windowed.y, m_aspect_ratio);
        }
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

        { // At the start of an ImGui frame, push a window the size of viewport to allow docking other ImGui windows to.
            ImGui::SetNextWindowSize(size());
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("Dockspace window", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::DockSpace(ImGui::GetID("Dockspace window"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None
            | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);

            ImGui::PopStyleVar(3);
        }
    }
    void Window::end_ImGui_frame()
    {
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    int Window::get_window_title_bar_height()
    {
#ifdef _WIN32
        return (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER));
#else
        // Not implemented get_taskbar_height for this platform
        return 0;
#endif
    }

    glm::ivec2 Window::get_max_resolution()
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return {mode->width, mode->height};
    }
} // namespace Platform