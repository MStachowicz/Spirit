#pragma once

#include "System/AssetManager.hpp"
#include "System/CollisionSystem.hpp"
#include "System/InputSystem.hpp"
#include "System/PhysicsSystem.hpp"
#include "System/SceneSystem.hpp"
#include "System/TerrainSystem.hpp"

#include "UI/Editor.hpp"

#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"

#include "OpenGL/DebugRenderer.hpp"
#include "OpenGL/OpenGLRenderer.hpp"

#include "Utility/File.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Performance.hpp"
#include "Utility/Stopwatch.hpp"

#include "Component/Collider.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/Input.hpp"
#include "Component/Label.hpp"
#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/ParticleEmitter.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Terrain.hpp"
#include "Component/Texture.hpp"
#include "Component/Transform.hpp"

#include <chrono>

using Clock     = std::chrono::steady_clock;
using Duration  = Clock::duration;
using TimePoint = std::chrono::time_point<Clock>;

// Application manages the ownership and calling of all the Systems.
// Taking an OS window it renders and updates the state of an ECS.
class Application
{
public:
	Application(Platform::Input& p_input, Platform::Window& p_window) noexcept;
	~Application() noexcept;
	void simulation_loop(uint16_t physics_ticks_per_second, uint16_t render_ticks_per_second, uint16_t input_ticks_per_second);

private:
	Platform::Input& m_input;
	Platform::Window& m_window; // Main window all application business takes place in. When this window is closed, the application ends and vice-versa.

	System::AssetManager m_asset_manager;
	System::SceneSystem m_scene_system;

	OpenGL::OpenGLRenderer m_openGL_renderer;

	System::CollisionSystem m_collision_system;
	System::PhysicsSystem m_physics_system;
	System::InputSystem m_input_system;
	System::TerrainSystem m_terrain_system;

	UI::Editor m_editor;
	std::chrono::milliseconds maxFrameDelta; // If the time between loops is beyond this, cap at this duration
};

int main(int argc, char* argv[])
{ (void)argv;
	{
		Utility::Stopwatch stopwatch;

		ECS::Component::set_info<Component::Collider>();
		ECS::Component::set_info<Component::FirstPersonCamera>();
		ECS::Component::set_info<Component::Input>();
		ECS::Component::set_info<Component::Label>();
		ECS::Component::set_info<Component::PointLight>();
		ECS::Component::set_info<Component::DirectionalLight>();
		ECS::Component::set_info<Component::SpotLight>();
		ECS::Component::set_info<Component::Mesh>();
		ECS::Component::set_info<Component::ParticleEmitter>();
		ECS::Component::set_info<Component::RigidBody>();
		ECS::Component::set_info<Component::Terrain>();
		ECS::Component::set_info<Component::Texture>();
		ECS::Component::set_info<Component::Transform>();

		// Library init order is important here
		// GLFW <- Window/GL context <- OpenGL functions <- ImGui <- App
		Platform::Core::initialise_directories();
		Platform::Core::initialise_GLFW();
		Platform::Input input   = Platform::Input();
		Platform::Window window = Platform::Window(glm::vec2{0.75f, 0.75f}, input);
		Platform::Core::initialise_OpenGL();
		OpenGL::DebugRenderer::init();
		Platform::Core::initialise_ImGui(window);

		LOG("[INIT] Number of arguments passed on launch: {}", argc);
		for (int index{}; index != argc; ++index)
			LOG("Argument {}: {}", index + 1, argv[index]);

		auto app = Application(input, window);
		LOG("[INIT] initialisation took {}", stopwatch.duration_since_start<int, std::milli>());

		app.simulation_loop(60, 0, 60);
	} // Window and input must go out of scope and destroy their resources before Core::deinitialise

	OpenGL::DebugRenderer::deinit();
	Platform::Core::deinitialise_ImGui();
	Platform::Core::deinitialise_GLFW();
	return EXIT_SUCCESS;
}