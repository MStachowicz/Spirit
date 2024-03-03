#pragma once

#include "System/CollisionSystem.hpp"
#include "System/InputSystem.hpp"
#include "System/MeshSystem.hpp"
#include "System/PhysicsSystem.hpp"
#include "System/SceneSystem.hpp"
#include "System/TextureSystem.hpp"

#include "UI/Editor.hpp"

#include "Platform/Core.hpp"
#include "Platform/Window.hpp"
#include "Platform/Input.hpp"

#include "OpenGL/OpenGLRenderer.hpp"
#include "OpenGL/DebugRenderer.hpp"
#include "OpenGL/GridRenderer.hpp"

#include "Utility/File.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Stopwatch.hpp"

#include <chrono>

// Application manages the ownership and calling of all the Systems.
// Taking an OS window it renders and updates the state of an ECS.
class Application
{
public:
	Application(Platform::Input& p_input, Platform::Window& p_window) noexcept;
	~Application() noexcept;
	void simulation_loop();

private:
	Platform::Input& m_input;
	Platform::Window& m_window; // Main window all application business takes place in. When this window is closed, the application ends and vice-versa.

	System::TextureSystem m_texture_system;
	System::MeshSystem m_mesh_system;
	System::SceneSystem m_scene_system;

	OpenGL::OpenGLRenderer m_openGL_renderer;
	OpenGL::GridRenderer m_grid_renderer;

	System::CollisionSystem m_collision_system;
	System::PhysicsSystem m_physics_system;
	System::InputSystem m_input_system;

	UI::Editor m_editor;

	bool m_simulation_loop_params_changed;       // True when the template params of simulation_loop change, causes an exit from the loop and re-run
	int m_physics_ticks_per_second;              // The number of physics updates to perform per second. This is equivalent to the pPhysicsTicksPerSecond template param of simulation_loop.
	int m_render_ticks_per_second;               // The number of renders to perform per second. This is equivalent to the pRenderTicksPerSecond template param of simulation_loop.
	int m_input_ticks_per_second;            // The number of input system updates to perform every second;
	std::chrono::milliseconds maxFrameDelta; // If the time between loops is beyond this, cap at this duration

	// This simulation loop uses a physics timestep based on integer type giving no truncation or round-off error.
	// It's required to be templated to allow physicsTimestep to be set using std::ratio as the chrono::duration period.
	// pPhysicsTicksPerSecond: The target number of physics ticks per second. This template parameter is always equivalent to m_physics_ticks_per_second.
	// pRenderTicksPerSecond: The target number of renders per second. This template parameter is always equivalent to m_render_ticks_per_second.
	template <int pPhysicsTicksPerSecond, int pRenderTicksPerSecond, int p_input_ticks_per_second>
	void simulation_loop()
	{
		using Clock = std::chrono::steady_clock;

		constexpr auto physicsTimestep = std::chrono::duration<Clock::rep, std::ratio<1, pPhysicsTicksPerSecond>>{1};
		constexpr auto renderTimestep  = std::chrono::duration<Clock::rep, std::ratio<1, pRenderTicksPerSecond>>{1};
		constexpr auto input_timestep  = std::chrono::duration<Clock::rep, std::ratio<1, p_input_ticks_per_second>>{1};

		// The resultant sum of a Clock::duration and physicsTimestep. This will be the coarsest precision that can exactly represent both a Clock::duration and 1/60 of a second.
		// Time-based arithmetic will have no truncation error, or any round-off error if Clock::duration is integral-based (std::chrono::nanoseconds for steady_clock is integral based).
		using Duration  = decltype(Clock::duration{} + physicsTimestep);
		using TimePoint = std::chrono::time_point<Clock, Duration>;

		LOG("Target physics ticks per second: {} (timestep: {}ms = {})", pPhysicsTicksPerSecond, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(physicsTimestep).count(), physicsTimestep);
		LOG("Target render ticks per second:  {} (timestep: {}ms = {})", pRenderTicksPerSecond, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(renderTimestep).count(), renderTimestep);
		LOG("Target input ticks per second:   {} (timestep: {}ms = {})", p_input_ticks_per_second, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(input_timestep).count(), input_timestep);

		Duration duration_since_last_physics_tick   = Duration::zero();     // Accumulated time since the last physics update.
		Duration duration_since_last_render_tick    = Duration::zero();     // Accumulated time since the last render.
		Duration duration_since_last_input_tick = Duration::zero();     // Accumulated time since the last input update.
		Duration duration_since_last_frame         = Duration::zero();     // Time between this frame and last frame.
		Duration duration_application_running     = Duration::zero();     // Total time the application has been running.

		TimePoint physics_time{};      // The time point the physics is advanced to currently.
		TimePoint time_frame_started{}; // The time point at the start of a new frame.
		TimePoint time_last_frame_started = Clock::now();

		// Continuous loop until the main window is marked for closing or Input requests close.
		// While looping the physics updates by fixed timestep physicsTimestep
		// The renderer produces time and the simulation consumes it in discrete physicsTimestep sized steps
		while (true)
		{
			OpenGL::DebugRenderer::clear();

			if (duration_since_last_input_tick >= input_timestep)
			{
				m_input.update(); // Poll events then check close_requested.
				if (m_window.close_requested() || m_simulation_loop_params_changed)
					break;

				m_input_system.update(input_timestep);
				duration_since_last_input_tick = Duration::zero();
			}

			time_frame_started = Clock::now();
			duration_since_last_frame = time_frame_started - time_last_frame_started;
			if (duration_since_last_frame > maxFrameDelta)
				duration_since_last_frame = maxFrameDelta;

			time_last_frame_started            = time_frame_started;
			duration_application_running      += duration_since_last_frame;
			duration_since_last_physics_tick  += duration_since_last_frame;
			duration_since_last_render_tick   += duration_since_last_frame;
			duration_since_last_input_tick    += duration_since_last_frame;

			// Apply physics updates until accumulated time is below physicsTimestep step
			while (duration_since_last_physics_tick >= physicsTimestep)
			{
				duration_since_last_physics_tick -= physicsTimestep;
				physics_time                     += physicsTimestep;
				m_physics_system.integrate(physicsTimestep); // PhysicsSystem::Integrate takes a floating point rep duration, conversion here is troublesome.
			}

			if (duration_since_last_render_tick >= renderTimestep)
			{
				m_scene_system.get_current_scene().update(m_window.aspect_ratio(), nullptr);

				// Rendering is only relevant if the data changed in the physics update.
				// Not neccessary to decrement duration_since_last_render_tick as in physics update above as repeated draws will be identical with no data changes.
				m_window.start_ImGui_frame();
				m_openGL_renderer.start_frame();

				m_grid_renderer.draw();
				m_openGL_renderer.draw(duration_since_last_render_tick);
				m_editor.draw(duration_since_last_render_tick);
				OpenGL::DebugRenderer::render(m_scene_system);

				m_openGL_renderer.end_frame();
				m_window.end_ImGui_frame();
				m_window.swap_buffers();

				duration_since_last_render_tick = Duration::zero();
			}
		}

		#ifndef Z_RELEASE
		const auto total_time_seconds = std::chrono::duration_cast<std::chrono::seconds>(duration_application_running);
		const float render_FPS  = static_cast<float>(m_editor.m_draw_count)           / total_time_seconds.count();
		const float physics_FPS = static_cast<float>(m_physics_system.m_update_count) / total_time_seconds.count();
		const float input_FPS   = static_cast<float>(m_input_system.m_update_count)   / total_time_seconds.count();

		LOG("------------------------------------------------------------------------");
		LOG("Total simulation time: {}", total_time_seconds);
		LOG("Total physics updates: {}", m_physics_system.m_update_count);
		LOG("Averaged physics updates per second: {}/s (target: {}/s)", physics_FPS, pPhysicsTicksPerSecond);
		LOG("Total rendered frames: {}", m_editor.m_draw_count);
		LOG("Averaged render frames per second: {}/s (target: {}/s)", render_FPS, pRenderTicksPerSecond);
		LOG("Total input updates: {}", m_input_system.m_update_count);
		LOG("Averaged input updates per second: {}/s (target: {}/s)", input_FPS, p_input_ticks_per_second);
		#endif
	}
};

int main(int argc, char* argv[])
{ (void)argv;
	{
		Utility::Stopwatch stopwatch;

		// Library init order is important here
		// GLFW <- Window/GL context <- OpenGL functions <- ImGui <- App
		Platform::Core::initialise_directories();
		Platform::Core::initialise_GLFW();
		Platform::Input input   = Platform::Input();
		Platform::Window window = Platform::Window(1920, 1080, input);
		Platform::Core::initialise_OpenGL();
		OpenGL::DebugRenderer::init();
		Platform::Core::initialise_ImGui(window);

		LOG("[INIT] Number of arguments passed on launch: {}", argc);
		for (int index{}; index != argc; ++index)
			LOG("Argument {}: {}", index + 1, argv[index]);

		auto app = Application(input, window);
		LOG("[INIT] initialisation took {}", stopwatch.duration_since_start<int, std::milli>());

		app.simulation_loop();
	} // Window and input must go out of scope and destroy their resources before Core::cleanup

	Platform::Core::cleanup();
	OpenGL::DebugRenderer::deinit();
	return EXIT_SUCCESS;
}