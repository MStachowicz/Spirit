#include "Core.hpp"
#include "Input.hpp"
#include "Window.hpp"

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
    void Core::initialise_GLFW()
    {
        glfwSetErrorCallback([](int p_error, const char* p_description){ LOG_ERROR("[GLFW] Error: {}: {}", p_error, p_description); });
        const auto initalisedGLFW = glfwInit();
        ASSERT(initalisedGLFW == GLFW_TRUE, "[INIT] GLFW initialisation failed");

        // Set Graphics context version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OpenGLMajorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OpenGLMinorVersion);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        LOG("[INIT] Initialised GLFW");
    }

    void Core::initialise_OpenGL()
    { // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
        int version = gladLoadGL(glfwGetProcAddress);
        ASSERT(version != 0, "[INIT] Failed to initialise GLAD OpenGL"); // ALWAYS ASSERT
        LOG("[INIT] Initialised GLAD OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    }

    void Core::initialise_ImGui(const Window& p_window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
        ImGui_ImplGlfw_InitForOpenGL(p_window.m_handle, true);
        ImGui_ImplOpenGL3_Init(OpenGLVersion);
        io.DisplaySize = p_window.size();

        LOG("[INIT] Initialised ImGui");
    }

    void Core::cleanup()
    {
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            LOG("[DEINIT] Uninitialised ImGui");
        }
        {
            glfwTerminate();
            LOG("[DEINIT] Uninitialised GLFW");
        }
    }
}