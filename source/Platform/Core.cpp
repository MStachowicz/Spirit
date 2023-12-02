#include "Core.hpp"
#include "Input.hpp"
#include "Window.hpp"

#include "Utility/Logger.hpp"
#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
// ImGui.h (must be included after GLFW)
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

namespace Platform
{
	void Core::initialise_directories()
	{
		ASSERT(Utility::File::exists(Config::Source_Directory), "CMake configured CMAKE_CURRENT_SOURCE_DIR does not exist!",     Config::Source_Directory.string());
		ASSERT(Utility::File::exists(Config::GLSL_Shader_Directory), "Setting GLSL directory failed. Path '{}' does not exist.", Config::GLSL_Shader_Directory.string());
		ASSERT(Utility::File::exists(Config::Texture_Directory), "Setting texture directory failed. Path '{}' does not exist.",  Config::Texture_Directory.string());
		ASSERT(Utility::File::exists(Config::Model_Directory), "Setting model directory failed. Path '{}' does not exist.",      Config::Model_Directory.string());

		LOG("[INIT][FILE] CMake configured source directory: '{}'", Config::Source_Directory.string());
		LOG("[INIT][FILE] Shader directory initialised to '{}'",    Config::GLSL_Shader_Directory.string());
		LOG("[INIT][FILE] Texture directory initialised to '{}'",   Config::Texture_Directory.string());
		LOG("[INIT][FILE] Model directory initialised to '{}'",     Config::Model_Directory.string());
	}

	void Core::initialise_GLFW()
	{
		#ifdef Z_DEBUG
		glfwSetErrorCallback([](int p_error, const char* p_description){ LOG_ERROR("[GLFW] Error: {}: {}", p_error, p_description); });
		#endif

		const auto initalisedGLFW = glfwInit();
		ASSERT_THROW(initalisedGLFW == GLFW_TRUE, "[INIT] GLFW initialisation failed");

		// Set Graphics context version
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Config::OpenGL_Version_Major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Config::OpenGL_Version_Minor);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		LOG("[INIT] Initialised GLFW");
	}

	void Core::initialise_OpenGL()
	{ // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
		int version = gladLoadGL(glfwGetProcAddress);
		ASSERT_THROW(version != 0, "[INIT] Failed to initialise GLAD OpenGL");
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
		ImGui_ImplOpenGL3_Init(Config::GLSL_Version_String);
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