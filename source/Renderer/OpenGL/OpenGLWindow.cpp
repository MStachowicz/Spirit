#include "OpenGLWindow.hpp"

#include "GLFW/glfw3.h"
// The imgui headers must be included after GLFW
#include "Logger.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imgui_draw.cpp"

namespace OpenGL
{
    OpenGLWindow::OpenGLWindow(const int& pMajorVersion, const int& pMinorVersion, const int& pWidth /* = 1920*/, const int& pHeight /* = 1080*/, const bool& pResizable /* = true*/)
        : mHandle(nullptr)
        , mWidth(pWidth)
        , mHeight(pHeight)
        , mAspectRatio(static_cast<float>(mWidth) / static_cast<float>(mHeight))
        , mOpenGLMajorVersion(pMajorVersion)
        , mOpenGLMinorVersion(pMinorVersion)
    {
        ZEPHYR_ASSERT(activeGLFWWindows == 0, "Creating a new window, Zephyr only supports one window.");

        if (activeGLFWWindows == 0) // First time window creation, setup GLFW and window hints
        {
            const auto initalisedGLFW = glfwInit();
            ZEPHYR_ASSERT(initalisedGLFW == GLFW_TRUE, "GLFW initialisation failed");

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, mOpenGLMajorVersion);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, mOpenGLMinorVersion);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        glfwWindowHint(GLFW_RESIZABLE, pResizable ? GL_TRUE : GL_FALSE);
        mHandle = glfwCreateWindow(mWidth, mHeight, "Zephyr", NULL, NULL);
        ZEPHYR_ASSERT(mHandle != nullptr, "GLFW window creation failed");
        LOG_INFO("OpenGL {}.{} window created with resolution {}x{} ", mOpenGLMajorVersion, mOpenGLMinorVersion, mWidth, mHeight);

        activeGLFWWindows++;
        currentWindow = this;
        glfwSetWindowUserPointer(mHandle, this);
        glfwMakeContextCurrent(mHandle);

        { // Initialise Dear ImGui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
            io.DisplaySize = ImVec2(static_cast<float>(mWidth), static_cast<float>(mHeight));
            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(mHandle, true);

            const std::string openGLVersionString = "#version " + std::to_string(mOpenGLMajorVersion) + std::to_string(mOpenGLMinorVersion) + "0";
            ImGui_ImplOpenGL3_Init(openGLVersionString.c_str());
        }
    }

    OpenGLWindow::~OpenGLWindow()
    {
        glfwSetWindowShouldClose(mHandle, GL_TRUE); // Ask GLFW to close this window
        LOG_INFO("Closing GLFW OpenGLWindow.");

        if (--activeGLFWWindows <= 0)
        {
            LOG_INFO("Final GLFW window closing. Terminating GLFW.");
            glfwTerminate(); // Terminate GLFW if this is the last window destroyed.

            { // Shutdown Dear ImGui
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();
            }
        }
    }

    GLFWwindow* const OpenGLWindow::getActiveWindowHandle()
    {
        ZEPHYR_ASSERT(currentWindow != nullptr, "No active window set for application to use.");
        ZEPHYR_ASSERT(currentWindow->mHandle != nullptr, "Active window has no active GLFW window handle.");
        return currentWindow->mHandle;
    }

    void OpenGLWindow::startImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        { // At the start of an ImGui frame, push a window the size of viewport to allow docking other ImGui windows to.
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(mWidth), static_cast<float>(mHeight)));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("Dockspace window", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus); // ImGuiWindowFlags_MenuBar
            ImGui::DockSpace(ImGui::GetID("Dockspace window"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);
            ImGui::End();

            ImGui::PopStyleVar(3);
        }
    }

    void OpenGLWindow::renderImGui()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void OpenGLWindow::swapBuffers()
    {
        glfwSwapBuffers(mHandle);
    }
} // namespace OpenGL