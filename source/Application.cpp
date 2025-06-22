#include "Application.hpp"

Application::Application(Platform::Input& p_input, Platform::Window& p_window) noexcept
	: m_input{p_input}
	, m_window{p_window}
	, m_asset_manager{}
	, m_scene_system{m_asset_manager}
	, m_openGL_renderer{m_window, m_asset_manager, m_scene_system}
	, m_collision_system{m_scene_system}
	, m_physics_system{m_scene_system, m_collision_system}
	, m_input_system{m_input, m_window, m_scene_system}
	, m_terrain_system{}
	, m_editor{m_input, m_window, m_asset_manager, m_scene_system, m_collision_system, m_physics_system, m_openGL_renderer}
	, maxFrameDelta{std::chrono::milliseconds(250)}
{
	Logger::s_editor_sink = &m_editor;
}
Application::~Application() noexcept
{
	Logger::s_editor_sink = nullptr;
}

void Application::simulation_loop(uint16_t physics_ticks_per_second, uint16_t render_ticks_per_second, uint16_t input_ticks_per_second)
{
	using namespace std::chrono_literals;
	Duration physics_timestep  = std::chrono::microseconds{1s} / physics_ticks_per_second; // Duration in seconds for the physics update.
	Duration input_timestep    = std::chrono::microseconds{1s} / input_ticks_per_second;   // Duration in seconds for the input update.
	bool render_rate_unlimited = render_ticks_per_second == 0;
	Duration render_timestep   = render_rate_unlimited ? Duration::zero() : std::chrono::microseconds{1s} / render_ticks_per_second; // Duration in seconds for the render update.

	LOG("Target physics ticks per second: 1/{} ({}ms)", physics_ticks_per_second, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(physics_timestep).count());
	LOG("Target input ticks per second:   1/{} ({}ms)", input_ticks_per_second,   std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(input_timestep).count());
	if (render_rate_unlimited) LOG("Target render ticks per second:  No limit (unlimited framerate)")
	else                       LOG("Target render ticks per second:  1/{} ({}ms)", render_ticks_per_second, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(render_timestep).count())

	Duration duration_since_last_physics_tick = Duration::zero(); // Accumulated time since the last physics update.
	Duration duration_since_last_render_tick  = Duration::zero(); // Accumulated time since the last render.
	Duration duration_since_last_input_tick   = Duration::zero(); // Accumulated time since the last input update.
	Duration duration_since_last_frame        = Duration::zero(); // Time between this frame and last frame.
	Duration duration_application_running     = Duration::zero(); // Total time the application has been running.
	TimePoint time_last_frame_started         = Clock::now();
	TimePoint time_frame_started;

	while (!m_window.close_requested())
	{
		time_frame_started = Clock::now();
		duration_since_last_frame = time_frame_started - time_last_frame_started;
		if (duration_since_last_frame > maxFrameDelta)
			duration_since_last_frame = maxFrameDelta;

		time_last_frame_started           = time_frame_started;
		duration_application_running     += duration_since_last_frame;
		duration_since_last_physics_tick += duration_since_last_frame;
		duration_since_last_render_tick  += duration_since_last_frame;
		duration_since_last_input_tick   += duration_since_last_frame;

		if (render_rate_unlimited || duration_since_last_render_tick >= render_timestep)
			m_window.start_ImGui_frame();

		// Apply physics updates until accumulated time is below physics_timestep step
		while (duration_since_last_physics_tick >= physics_timestep)
		{
			duration_since_last_physics_tick -= physics_timestep;
			m_physics_system.integrate(physics_timestep); // PhysicsSystem::Integrate takes a floating point rep duration, conversion here is troublesome.
			m_collision_system.update();
		}

		if (duration_since_last_input_tick >= input_timestep)
		{
			m_input.update(); // Poll events then check close_requested.
			m_input_system.update(input_timestep);
			duration_since_last_input_tick = Duration::zero();
		}

		if (render_rate_unlimited || duration_since_last_render_tick >= render_timestep)
		{
			m_scene_system.get_current_scene().update(m_window.aspect_ratio(), m_editor.get_editor_view_info());
			m_terrain_system.update(m_scene_system.get_current_scene(), m_window.aspect_ratio());

			m_openGL_renderer.start_frame();
			m_openGL_renderer.draw(duration_since_last_render_tick);
			m_editor.draw(duration_since_last_render_tick);

			m_openGL_renderer.end_frame();
			m_window.end_ImGui_frame();
			m_window.swap_buffers();

			duration_since_last_render_tick = Duration::zero();
		}

		Utility::ScopedPerformanceBench::s_performance_benchmarks.end_frame();
		OpenGL::DebugRenderer::clear();
		PERF_FRAME_END
	}

#ifndef Z_RELEASE
		const auto total_time_seconds = std::chrono::duration_cast<std::chrono::seconds>(duration_application_running);
		const float render_FPS  = static_cast<float>(m_editor.m_draw_count)           / total_time_seconds.count();
		const float physics_FPS = static_cast<float>(m_physics_system.m_update_count) / total_time_seconds.count();
		const float input_FPS   = static_cast<float>(m_input_system.m_update_count)   / total_time_seconds.count();

		LOG("------------------------------------------------------------------------");
		LOG("Total simulation time: {}", total_time_seconds);
		LOG("Total physics updates: {}", m_physics_system.m_update_count);
		LOG("Averaged physics updates per second: {}/s (target: {}/s)", physics_FPS, physics_ticks_per_second);
		LOG("Total rendered frames: {}", m_editor.m_draw_count);
		if (render_rate_unlimited) LOG("Averaged render frames per second: {}/s (No limit)", render_FPS)
		else                       LOG("Averaged render frames per second: {}/s (target: {}/s)", render_FPS, render_ticks_per_second);

		LOG("Total input updates: {}", m_input_system.m_update_count);
		LOG("Averaged input updates per second: {}/s (target: {}/s)", input_FPS, input_ticks_per_second);
#endif
	}