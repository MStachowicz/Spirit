#include "Core.hpp"
#include "Input.hpp"
#include "Window.hpp"

#include "Utility/Logger.hpp"
#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#include "glad/glad.h"
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
		glfwSetErrorCallback([](int p_error, const char* p_description){ LOG_ERROR(false, "[GLFW] Error: {}: {}", p_error, p_description); });
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
	{ // Load OpenGL functions
		int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ASSERT_THROW(version != 0, "[INIT] Failed to initialise GLAD OpenGL");
		LOG("[INIT] Initialised OpenGL {}", (const char*)glGetString(GL_VERSION));
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

#ifdef _WIN32
	#include <Windows.h>

	bool Core::is_dark_mode()
	{
		HKEY key;
		DWORD value;
		DWORD size = sizeof(DWORD);
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
		{
			if (RegQueryValueExW(key, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS)
				return value == 0;

			RegCloseKey(key);
		}

		return true; // Default to dark mode if unable to read the registry value
	}
#else
	bool Core::is_dark_mode()
	{
		return true; // Default to dark mode if no platform specific implementation
	}
#endif


	Core::Theme::Theme()
	{
		dark_mode    = Core::is_dark_mode();
		background   = dark_mode ? glm::vec4(0.1f, 0.1f, 0.1f, 1.f) : glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
		accent       = glm::vec4(0.2f, 0.6f, 1.0f, 1.f);
		general_text = dark_mode ? glm::vec4(0.9f, 0.9f, 0.9f, 1.f) : glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
		warning_text = glm::vec4(1.0f, 0.6f, 0.0f, 1.f);
		error_text   = glm::vec4(1.0f, 0.0f, 0.0f, 1.f);
		success_text = dark_mode ? glm::vec4(0.0f, 1.0f, 0.0f, 1.f) : glm::vec4(0.f, 0.765f, 0.133f, 1.f);
	}
	void Core::Theme::draw_theme_editor_UI()
	{
		ImGui::Begin("Theme editor");
		ImGui::Checkbox("Dark mode",    &dark_mode);
		ImGui::ColorEdit4("Background", &background.x);
		ImGui::ColorEdit4("Accent",     &accent.x);
		ImGui::ColorEdit4("General",    &general_text.x);
		ImGui::ColorEdit4("Warning",    &warning_text.x);
		ImGui::ColorEdit4("Error",      &error_text.x);
		ImGui::ColorEdit4("Success",    &success_text.x);
		ImGui::End();
	}
}