#include "Core.hpp"

#include "InputDefinitions.hpp"

// Utility
#include "Logger.hpp"
#include "File.hpp"

// External
#include "glad/gl.h"
#include "GLFW/glfw3.h"

// ImGui.h (must be included after GLFW)
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

namespace Platform
{
    Window::Window(const int& pWidth, const int& pHeight)
        : m_size_fullscreen{pWidth, pHeight}
        , m_position_fullscreen{0,0}
        , m_size_windowed{pWidth, pHeight}
        , m_position_windowed{0,0}
        , m_fullscreen{false}
        , m_aspect_ratio{m_fullscreen ? static_cast<float>(m_size_fullscreen.first) / static_cast<float>(m_size_fullscreen.second) : static_cast<float>(m_size_windowed.first) / static_cast<float>(m_size_windowed.second)}
        , m_VSync{false}
        , mCapturingMouse{false}
        , mCapturedChangedThisFrame{false}
        , mHandle{nullptr}
    {
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        mHandle = glfwCreateWindow(size().first, size().second, "Zephyr", m_fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
        ASSERT(mHandle != nullptr, "Failed to create a GLFW window");

        glfwSetWindowUserPointer(mHandle, this);
        glfwMakeContextCurrent(mHandle); // Set this window as the context for GL render calls.
        glfwSetInputMode(mHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetWindowPos(mHandle, position().first, position().second);
        set_VSync(m_VSync);

        // Set the taskbar icon for the window
        const auto icon_path = Utility::File::textureDirectory / "Icons" / "Icon.png";
        auto icon_image = Utility::File::s_image_files.getOrCreate([&icon_path](const Utility::Image& p_image){ return p_image.m_filepath == icon_path; }, icon_path);
        GLFWimage icon;
        icon.pixels = (unsigned char*)(icon_image->get_data());
        icon.width = icon_image->m_width;
        icon.height = icon_image->m_height;
        glfwSetWindowIcon(mHandle, 1, &icon);

        LOG("Created GLFW Window with resolution {}x{}", size().first, size().second);
    }
    Window::~Window()
    {
        glfwDestroyWindow(mHandle);
        LOG("Destroyed GLFW Window");
    }

    void Window::set_size(std::pair<int, int> p_new_size)
    {
        if (m_fullscreen)
        {
            m_size_fullscreen = p_new_size;
            m_aspect_ratio    = static_cast<float>(m_size_fullscreen.first) / static_cast<float>(m_size_fullscreen.second);
        }
        else
        {
            m_size_windowed = p_new_size;
            m_aspect_ratio  = static_cast<float>(m_size_windowed.first) / static_cast<float>(m_size_windowed.second);
        }

        LOG("[WINDOW] Resized to {}x{} aspect: {}", size().first, size().second, get_aspect_ratio());
    }
    void Window::set_position(std::pair<int, int> p_new_position)
    {
        if (m_fullscreen)
            m_position_fullscreen = p_new_position;
        else
            m_position_windowed = p_new_position;

        LOG("[WINDOW] Moved to {}, {}", position().first, position().second, get_aspect_ratio());
    }

    void Window::requestClose()
    {
        glfwSetWindowShouldClose(mHandle, GL_TRUE); // Ask GLFW to close this window
    }
    void Window::set_VSync(bool p_enabled)
    {
        glfwMakeContextCurrent(mHandle);
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
            const auto& [res_x, res_y] = Platform::Core::getPrimaryMonitorResolution();
            glfwSetWindowMonitor(mHandle, glfwGetPrimaryMonitor(), 0, 0, res_x, res_y, GLFW_DONT_CARE);
            set_position({0, 0});
            set_size({res_x, res_y});
            LOG("[WINDOW] Set to fullscreen. Position: {},{} Resolution: {}x{} Aspect ratio: {}", 0, 0, m_size_fullscreen.first, m_size_fullscreen.second, m_aspect_ratio);
        }
        else // Windowed mode. Reuse the old size and position
        {
            glfwSetWindowMonitor(mHandle, NULL, m_position_windowed.first, m_position_windowed.second, m_size_windowed.first, m_size_windowed.second, GLFW_DONT_CARE);
            LOG("[WINDOW] Set to windowed mode. Position: {},{} Resolution: {}x{} Aspect ratio: {}", 0, 0, m_size_windowed.first, m_size_windowed.second, m_aspect_ratio);
        }
    }

    void Window::setInputMode(const CursorMode& pCursorMode)
    {
        switch (pCursorMode)
        {
            case CursorMode::NORMAL:
                glfwSetInputMode(mHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
                mCapturedChangedThisFrame = true;
                mCapturingMouse = false;
                break;
            case CursorMode::HIDDEN:
                glfwSetInputMode(mHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                mCapturedChangedThisFrame = true;
                mCapturingMouse = false;
                break;
            case CursorMode::CAPTURED:
                ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
                glfwSetInputMode(mHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                mCapturedChangedThisFrame = true;
                mCapturingMouse = true;
                break;
            case CursorMode::UNKNOWN:
            default:
                LOG_ERROR("UNKNOWN cursor mode requested for input mode");
                break;
        }
    }

    void Core::initialise()
    {
        constexpr int OpenGLMajorVersion = 4;
        constexpr int OpenGLMinorVersion = 3;
        constexpr const char* const OpenGLVersion = "#version 430";

        { // Initialise GLFW
            glfwSetErrorCallback(GLFW_errorCallback);
            const auto initalisedGLFW = glfwInit();
            ASSERT(initalisedGLFW == GLFW_TRUE, "GLFW initialisation failed");

            // Set Graphics context version
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OpenGLMajorVersion);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OpenGLMinorVersion);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        const auto fullResolution = getPrimaryMonitorResolution();
        mPrimaryWindow = new Window(fullResolution.first, fullResolution.second);

        glfwSetKeyCallback(mPrimaryWindow->mHandle,         GLFW_keyPressCallback);
        glfwSetWindowCloseCallback(mPrimaryWindow->mHandle, GLFW_windowCloseCallback);
        glfwSetCursorPosCallback(mPrimaryWindow->mHandle,   GLFW_mouseMoveCallback);
        glfwSetMouseButtonCallback(mPrimaryWindow->mHandle, GLFW_mouseButtonCallback);
        glfwSetWindowSizeCallback(mPrimaryWindow->mHandle,  GLFW_windowResizeCallback);
        glfwSetWindowPosCallback(mPrimaryWindow->mHandle,   GLFW_window_move_callback);

        { // Initialise ImGui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
            io.DisplaySize = ImVec2(static_cast<float>(mPrimaryWindow->size().first), static_cast<float>(mPrimaryWindow->size().second));
            ImGui::StyleColorsDark();

            ImGui_ImplGlfw_InitForOpenGL(mPrimaryWindow->mHandle, true);
            ImGui_ImplOpenGL3_Init(OpenGLVersion);
        }

        {
            mOpenGLContext = (GladGLContext*)malloc(sizeof(GladGLContext));
            int version    = gladLoadGLContext(mOpenGLContext, glfwGetProcAddress);
            ASSERT(mOpenGLContext && version != 0, "Failed to initialise GLAD OpenGL");
            LOG("Initialised GLAD using OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
        }

        LOG("OpenGL {}.{} context created", OpenGLMajorVersion, OpenGLMinorVersion);
        LOG("Core initialised");
    }
    void Core::cleanup()
    {
        if (mPrimaryWindow)
            delete mPrimaryWindow;

        glfwTerminate();

        { // Shutdown Dear ImGui
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        free(mOpenGLContext);

        LOG("Core cleanup");
    }

    void Core::pollEvents()
    {
        // Calling glfwSetWindowShouldClose or Window::request close doesnt destroy the window,
        // we have to handle this ourselves by reading the flag.
        if (hasWindow() && glfwWindowShouldClose(mPrimaryWindow->mHandle))
        {
            delete mPrimaryWindow;
            mPrimaryWindow = nullptr;
        }
        else
        {
            if (mPrimaryWindow->mCapturedChangedThisFrame)
                mPrimaryWindow->mCapturedChangedThisFrame = false;
        }

        glfwPollEvents();
    }

    bool Core::UICapturingMouse()
    {
        return ImGui::GetIO().WantCaptureMouse;
    }
    bool Core::UICapturingKeyboard()
    {
        return ImGui::GetIO().WantCaptureKeyboard;
    }

    bool Core::is_key_down(const Key& p_key)
    { // Updated in GLFW_keyPressCallback
        return keys_pressed[static_cast<size_t>(p_key)];
    }

    std::pair<float, float> Core::getCursorPosition()
    {
        double x, y;
        glfwGetCursorPos(mPrimaryWindow->mHandle, &x, &y);
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    void Core::swapBuffers()
    {
        glfwSwapBuffers(mPrimaryWindow->mHandle);
    }

    void Core::startImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        { // At the start of an ImGui frame, push a window the size of viewport to allow docking other ImGui windows to.
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(mPrimaryWindow->size().first), static_cast<float>(mPrimaryWindow->size().second)));
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
    void Core::endImGuiFrame()
    {
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    std::pair<int,int> Core::getPrimaryMonitorResolution()
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return {mode->width, mode->height};
    }

    void Core::GLFW_errorCallback(int pError, const char* pDescription)
    {
        LOG_ERROR("GLFW error {}: {}", pError, pDescription);
    }
    void Core::GLFW_windowCloseCallback(GLFWwindow* pWindow)
    {
        if (mPrimaryWindow->mHandle == pWindow)
        {
            delete mPrimaryWindow;
            mPrimaryWindow = nullptr;
        }
    }
    void Core::GLFW_keyPressCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode)
    {
        if (pAction == GLFW_PRESS)
        {
            mKeyPressEvent.dispatch(GLFW_getKey(pKey));
            keys_pressed[static_cast<size_t>(GLFW_getKey(pKey))] = true;
        }
        else if (pAction == GLFW_RELEASE)
        {
            keys_pressed[static_cast<size_t>(GLFW_getKey(pKey))] = false;
        }
    }
    void Core::GLFW_mouseMoveCallback(GLFWwindow* pWindow, double pNewXPosition, double pNewYPosition)
    {
        if (mLastXPosition == -1.0 || mLastYPosition == -1.0) // First mouse input
        {
            mLastXPosition = pNewXPosition;
            mLastYPosition = pNewYPosition;
        }

        const float xOffset = static_cast<float>(pNewXPosition - mLastXPosition);
        const float yOffset = static_cast<float>(mLastYPosition - pNewYPosition); // reversed since y-coordinates go from bottom to top

        mLastXPosition = pNewXPosition;
        mLastYPosition = pNewYPosition;
        mMouseMoveEvent.dispatch(xOffset, yOffset);
    }
    void Core::GLFW_mouseButtonCallback(GLFWwindow* pWindow, int pButton, int pAction, int pModifiers)
    {
        mMouseButtonEvent.dispatch(GLFW_getMouseButton(pButton), GLFW_getAction(pAction));
    }
    void Core::GLFW_windowResizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
    {
        ImGuiIO& io        = ImGui::GetIO();
        io.DisplaySize     = ImVec2(static_cast<float>(pWidth), static_cast<float>(pHeight));
        io.FontGlobalScale = std::round(ImGui::GetMainViewport()->DpiScale);

        mPrimaryWindow->set_size({pWidth, pHeight});
        auto new_size = mPrimaryWindow->size();
        mWindowResizeEvent.dispatch(new_size.first, new_size.second);
    }

    void Core::GLFW_window_move_callback(GLFWwindow* p_window, int p_x_position, int p_y_position)
    {
        mPrimaryWindow->set_position({p_x_position, p_y_position});
    }

    Key Core::GLFW_getKey(const int& pKeyInput)
    {
        switch (pKeyInput)
        {
            case GLFW_KEY_0:      return Key::Num_0;
            case GLFW_KEY_1:      return Key::Num_1;
            case GLFW_KEY_2:      return Key::Num_2;
            case GLFW_KEY_3:      return Key::Num_3;
            case GLFW_KEY_4:      return Key::Num_4;
            case GLFW_KEY_5:      return Key::Num_5;
            case GLFW_KEY_6:      return Key::Num_6;
            case GLFW_KEY_7:      return Key::Num_7;
            case GLFW_KEY_8:      return Key::Num_8;
            case GLFW_KEY_9:      return Key::Num_9;
            case GLFW_KEY_A:      return Key::A;
            case GLFW_KEY_B:      return Key::B;
            case GLFW_KEY_C:      return Key::C;
            case GLFW_KEY_D:      return Key::D;
            case GLFW_KEY_E:      return Key::E;
            case GLFW_KEY_F:      return Key::F;
            case GLFW_KEY_G:      return Key::G;
            case GLFW_KEY_H:      return Key::H;
            case GLFW_KEY_I:      return Key::I;
            case GLFW_KEY_J:      return Key::J;
            case GLFW_KEY_K:      return Key::K;
            case GLFW_KEY_L:      return Key::L;
            case GLFW_KEY_M:      return Key::M;
            case GLFW_KEY_N:      return Key::N;
            case GLFW_KEY_O:      return Key::O;
            case GLFW_KEY_P:      return Key::P;
            case GLFW_KEY_Q:      return Key::Q;
            case GLFW_KEY_R:      return Key::R;
            case GLFW_KEY_S:      return Key::S;
            case GLFW_KEY_T:      return Key::T;
            case GLFW_KEY_U:      return Key::U;
            case GLFW_KEY_V:      return Key::V;
            case GLFW_KEY_W:      return Key::W;
            case GLFW_KEY_X:      return Key::X;
            case GLFW_KEY_Y:      return Key::Y;
            case GLFW_KEY_Z:      return Key::Z;
            case GLFW_KEY_F1:     return Key::F1;
            case GLFW_KEY_F2:     return Key::F2;
            case GLFW_KEY_F3:     return Key::F3;
            case GLFW_KEY_F4:     return Key::F4;
            case GLFW_KEY_F5:     return Key::F5;
            case GLFW_KEY_F6:     return Key::F6;
            case GLFW_KEY_F7:     return Key::F7;
            case GLFW_KEY_F8:     return Key::F8;
            case GLFW_KEY_F9:     return Key::F9;
            case GLFW_KEY_F10:    return Key::F10;
            case GLFW_KEY_F11:    return Key::F11;
            case GLFW_KEY_F12:    return Key::F12;
            case GLFW_KEY_SPACE:  return Key::Space;
            case GLFW_KEY_ESCAPE: return Key::Escape;
            case GLFW_KEY_ENTER:  return Key::Enter;
            case GLFW_KEY_TAB:    return Key::Tab;

            case GLFW_KEY_UNKNOWN:
            default:
                LOG_ERROR("Could not convert GLFW key '{}' to Key", pKeyInput);
                return Key::Unknown;
        }
    }
    MouseButton Core::GLFW_getMouseButton(const int& pMouseButton)
    {
        switch (pMouseButton)
        {
            case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::MOUSE_LEFT;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::MOUSE_MIDDLE;
            case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::MOUSE_RIGHT;
            case GLFW_MOUSE_BUTTON_4:      return MouseButton::MOUSE_BUTTON_1;
            case GLFW_MOUSE_BUTTON_5:      return MouseButton::MOUSE_BUTTON_2;
            case GLFW_MOUSE_BUTTON_6:      return MouseButton::MOUSE_BUTTON_3;
            case GLFW_MOUSE_BUTTON_7:      return MouseButton::MOUSE_BUTTON_4;
            case GLFW_MOUSE_BUTTON_8:      return MouseButton::MOUSE_BUTTON_5;
            default:
                LOG_ERROR("Could not convert GLFW mouse button '{}' to MouseButton", pMouseButton);
                return MouseButton::MOUSE_UNKNOWN;
        }
    }
    Action Core::GLFW_getAction(const int& pAction)
    {
        switch (pAction)
        {
            case GLFW_PRESS:   return Action::PRESS;
            case GLFW_RELEASE: return Action::RELEASE;
            case GLFW_REPEAT:  return Action::REPEAT;
            default:
                LOG_ERROR("Could not convert GLFW action '{}' to Action", pAction);
                return Action::UNKNOWN;
        }
    }
}