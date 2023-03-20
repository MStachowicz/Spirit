#include "Core.hpp"

#include "InputDefinitions.hpp"

// Utility
#include "Logger.hpp"

// External
#include "glad/gl.h"
#include "GLFW/glfw3.h"
// ImGui (must be included after GLFW)
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
//#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"
#include "imgui_draw.cpp"

namespace Platform
{
    Window::Window(const int& pWidth, const int& pHeight)
        : mWidth{pWidth}
        , mHeight{pHeight}
        , mAspectRatio{static_cast<float>(mWidth) / static_cast<float>(mHeight)}
        , mCapturingMouse{false}
        , mCapturedChangedThisFrame{false}
        , mHandle{nullptr}
    {
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        mHandle = glfwCreateWindow(mWidth, mHeight, "Zephyr", NULL, NULL);
        ASSERT(mHandle != nullptr, "Failed to create a GLFW window");
        glfwSetWindowUserPointer(mHandle, this);
        glfwMakeContextCurrent(mHandle);
        glfwSetInputMode(mHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        LOG("Created GLFW Window with resolution {}x{}",mWidth, mHeight);
    }
    void Window::requestClose()
    {
        glfwSetWindowShouldClose(mHandle, GL_TRUE); // Ask GLFW to close this window
    }
    Window::~Window()
    {
        glfwDestroyWindow(mHandle);
        LOG("Destroyed GLFW Window");
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
                LOG_ERROR("Could not convert cursor mode requested ({}) to GLFW cursor mode", pCursorMode);
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


        { // Initialise ImGui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
            io.DisplaySize = ImVec2(static_cast<float>(mPrimaryWindow->mWidth), static_cast<float>(mPrimaryWindow->mHeight));
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
        if (glfwWindowShouldClose(mPrimaryWindow->mHandle))
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
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(mPrimaryWindow->mWidth), static_cast<float>(mPrimaryWindow->mHeight)));
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
            mKeyPressEvent.dispatch(GLFW_getKey(pKey));
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

        mPrimaryWindow->mWidth = pWidth;
        mPrimaryWindow->mHeight = pHeight;
        mPrimaryWindow->mAspectRatio = static_cast<float>(pWidth) / static_cast<float>(pHeight);

        mWindowResizeEvent.dispatch(pWidth, pHeight);
    }

    Key Core::GLFW_getKey(const int& pKeyInput)
    {
        switch (pKeyInput)
        {
            case GLFW_KEY_0:      return Key::KEY_0;
            case GLFW_KEY_1:      return Key::KEY_1;
            case GLFW_KEY_2:      return Key::KEY_2;
            case GLFW_KEY_3:      return Key::KEY_3;
            case GLFW_KEY_4:      return Key::KEY_4;
            case GLFW_KEY_5:      return Key::KEY_5;
            case GLFW_KEY_6:      return Key::KEY_6;
            case GLFW_KEY_7:      return Key::KEY_7;
            case GLFW_KEY_8:      return Key::KEY_8;
            case GLFW_KEY_9:      return Key::KEY_9;
            case GLFW_KEY_A:      return Key::KEY_A;
            case GLFW_KEY_B:      return Key::KEY_B;
            case GLFW_KEY_C:      return Key::KEY_C;
            case GLFW_KEY_D:      return Key::KEY_D;
            case GLFW_KEY_E:      return Key::KEY_E;
            case GLFW_KEY_F:      return Key::KEY_F;
            case GLFW_KEY_G:      return Key::KEY_G;
            case GLFW_KEY_H:      return Key::KEY_H;
            case GLFW_KEY_I:      return Key::KEY_I;
            case GLFW_KEY_J:      return Key::KEY_J;
            case GLFW_KEY_K:      return Key::KEY_K;
            case GLFW_KEY_L:      return Key::KEY_L;
            case GLFW_KEY_M:      return Key::KEY_M;
            case GLFW_KEY_N:      return Key::KEY_N;
            case GLFW_KEY_O:      return Key::KEY_O;
            case GLFW_KEY_P:      return Key::KEY_P;
            case GLFW_KEY_Q:      return Key::KEY_Q;
            case GLFW_KEY_R:      return Key::KEY_R;
            case GLFW_KEY_S:      return Key::KEY_S;
            case GLFW_KEY_T:      return Key::KEY_T;
            case GLFW_KEY_U:      return Key::KEY_U;
            case GLFW_KEY_V:      return Key::KEY_V;
            case GLFW_KEY_W:      return Key::KEY_W;
            case GLFW_KEY_X:      return Key::KEY_X;
            case GLFW_KEY_Y:      return Key::KEY_Y;
            case GLFW_KEY_Z:      return Key::KEY_Z;
            case GLFW_KEY_F1:     return Key::KEY_F1;
            case GLFW_KEY_F2:     return Key::KEY_F2;
            case GLFW_KEY_F3:     return Key::KEY_F3;
            case GLFW_KEY_F4:     return Key::KEY_F4;
            case GLFW_KEY_F5:     return Key::KEY_F5;
            case GLFW_KEY_F6:     return Key::KEY_F6;
            case GLFW_KEY_F7:     return Key::KEY_F7;
            case GLFW_KEY_F8:     return Key::KEY_F8;
            case GLFW_KEY_F9:     return Key::KEY_F9;
            case GLFW_KEY_F10:    return Key::KEY_F10;
            case GLFW_KEY_F11:    return Key::KEY_F11;
            case GLFW_KEY_F12:    return Key::KEY_F12;
            case GLFW_KEY_SPACE:  return Key::KEY_SPACE;
            case GLFW_KEY_ESCAPE: return Key::KEY_ESCAPE;
            case GLFW_KEY_ENTER:  return Key::KEY_ENTER;
            case GLFW_KEY_TAB:    return Key::KEY_TAB;

            case GLFW_KEY_UNKNOWN:
            default:
                LOG_ERROR("Could not convert GLFW key ({}) to Key", pKeyInput);
                return Key::KEY_UNKNOWN;
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
                LOG_ERROR("Could not convert GLFW mouse button ({}) to MouseButton", pMouseButton);
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
                LOG_ERROR("Could not convert GLFW action ({}) to Action", pAction);
                return Action::UNKNOWN;
        }
    }
}