#include "Core.hpp"
#include "Input.hpp"
#include "Window.hpp"

#include "Utility/Logger.hpp"
#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#ifdef _WIN32
	#include <Windows.h> // MSVC requires Windows.h to be included before glfw headers
#endif
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
		LOG("[INIT] Initialised OpenGL\nVersion:  {}\nVendor:   {}\nRenderer: {}", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER));

		{// Set OpenGL debug callback
			static const std::unordered_map<GLenum, const char*> error_source_map {
				{GL_DEBUG_SOURCE_API,              "SOURCE_API"},
				{GL_DEBUG_SOURCE_WINDOW_SYSTEM,    "WINDOW_SYSTEM"},
				{GL_DEBUG_SOURCE_SHADER_COMPILER,  "SHADER_COMPILER"},
				{GL_DEBUG_SOURCE_THIRD_PARTY,      "THIRD_PARTY"},
				{GL_DEBUG_SOURCE_APPLICATION,      "APPLICATION"},
				{GL_DEBUG_SOURCE_OTHER,            "OTHER"}};
			static const std::unordered_map<GLenum, const char*> severity_map {
				{GL_DEBUG_SEVERITY_HIGH,           "HIGH"},
				{GL_DEBUG_SEVERITY_MEDIUM,         "MEDIUM"},
				{GL_DEBUG_SEVERITY_LOW,            "LOW"},
				{GL_DEBUG_SEVERITY_NOTIFICATION,   "NOTIFICATION"}};
			static const std::unordered_map<GLenum, const char*> error_type_map {
				{GL_DEBUG_TYPE_ERROR,              "ERROR"},
				{GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,"DEPRECATED_BEHAVIOR"},
				{GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "UNDEFINED_BEHAVIOR"},
				{GL_DEBUG_TYPE_PORTABILITY,        "PORTABILITY"},
				{GL_DEBUG_TYPE_PERFORMANCE,        "PERFORMANCE"},
				{GL_DEBUG_TYPE_OTHER,              "OTHER"},
				{GL_DEBUG_TYPE_MARKER,             "MARKER"}};

			auto GL_error_callback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
			{ (void)id; (void)length; (void)user_param;
				auto src = error_source_map.at(source);
				auto sv  = severity_map.at(severity);
				auto tp  = error_type_map.at(type);

				if (type == GL_DEBUG_TYPE_PERFORMANCE ||
				    severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW)
				{
					LOG_WARN_NO_LOCATION("[OpenGL][{}][{}][{}]: {}", sv, src, tp, message);
				}
				else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
				{
					ASSERT_FAIL("[OpenGL][{}][{}][{}]: {}", sv, src, tp, message);
				}
				else if (severity == GL_DEBUG_SEVERITY_HIGH)
				{
					ASSERT_FAIL("[OpenGL][{}][{}][{}]: {}", sv, src, tp, message);
				}
			};

			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(GL_error_callback, 0);
			glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, false);
		}
	}

	void Core::initialise_ImGui(const Window& p_window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(p_window.m_handle, true);
		ImGui_ImplOpenGL3_Init(Config::GLSL_Version_String);

		ImGuiIO& io        = ImGui::GetIO();
		io.FontGlobalScale = p_window.content_scale();
		glm::uvec2 size    = p_window.size();
		io.DisplaySize     = ImVec2(static_cast<float>(size.x), static_cast<float>(size.y));
		io.ConfigFlags    |= ImGuiConfigFlags_DockingEnable;

		LOG("[INIT] Initialised ImGui");
	}

	void Core::deinitialise_ImGui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		LOG("[DEINIT] Uninitialised ImGui");
	}
	void Core::deinitialise_GLFW()
	{
		glfwTerminate();
		LOG("[DEINIT] Uninitialised GLFW");
	}


#ifdef _WIN32
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

	std::filesystem::path file_dialog(FileDialogType p_type, FileDialogFilter p_filter, const char* p_title, const std::filesystem::path& p_start_path)
	{
		#ifdef _WIN32
			OPENFILENAMEA ofn;
			CHAR szFile[260]    = { 0 };
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lpstrTitle      = p_title;
			ofn.lStructSize     = sizeof(ofn);
			ofn.hwndOwner       = NULL;
			ofn.lpstrFile       = szFile;
			ofn.lpstrFile[0]    = '\0';
			ofn.nMaxFile        = sizeof(szFile);
			ofn.nFilterIndex    = 1;
			ofn.lpstrFileTitle  = NULL;
			ofn.nMaxFileTitle   = 0;

			// If the start path is empty isnt empty and doesnt exist, create it
			if (!p_start_path.empty() && !std::filesystem::exists(p_start_path))
				std::filesystem::create_directories(p_start_path);
			ofn.lpstrInitialDir = p_start_path.string().c_str();

			// ("Scene Files (*.{})\0*.{}\0", Config::Save_Extension, Config::Save_Extension)
			static std::string scene_filter = "Scene File\0*.SS\0";
			switch (p_filter)
			{
				case FileDialogFilter::All:   ofn.lpstrFilter = "All Files\0*.*\0"; break;
				case FileDialogFilter::Scene: ofn.lpstrFilter = scene_filter.c_str(); break;
				default: ASSERT_FAIL("Invalid FileDialogFilter");
			}

			std::filesystem::path selected_path;
			switch (p_type)
			{
				case FileDialogType::Save:
				{
					ofn.Flags = OFN_OVERWRITEPROMPT;
					if (GetSaveFileNameA(&ofn) == TRUE) selected_path = std::filesystem::path(ofn.lpstrFile);
					break;
				}
				case FileDialogType::Open:
				{
					if (GetOpenFileNameA(&ofn) == TRUE) selected_path = std::filesystem::path(ofn.lpstrFile);
					break;
				}
				default: ASSERT_FAIL("Invalid FileDialogType");
			}

			// Append the .ss extension if not present
			if (p_type == FileDialogType::Save && !selected_path.empty() && selected_path.extension() != ".ss")
				selected_path.replace_extension(".ss");

			return selected_path;
		#else
			(void)p_type; (void)p_filter; (void)p_title; (void)p_start_path;
			ASSERT_FAIL("file_dialog not implemented for this platform");
		#endif
	}
} // namespace Platform