#include "Application.hpp"

Application::Application(Platform::Input& p_input, Platform::Window& p_window) noexcept
	: m_input{p_input}
	, m_window{p_window}
	, m_texture_system{}
	, m_mesh_system{m_texture_system}
	, m_scene_system{m_texture_system, m_mesh_system}
	, m_openGL_renderer{m_window, m_scene_system, m_mesh_system, m_texture_system}
	, m_grid_renderer{}
	, m_collision_system{m_scene_system}
	, m_physics_system{m_scene_system, m_collision_system}
	, m_input_system{m_input, m_window, m_scene_system}
	, m_editor{m_input, m_window, m_texture_system, m_mesh_system, m_scene_system, m_collision_system, m_openGL_renderer}
	, m_simulation_loop_params_changed{false}
	, m_physics_ticks_per_second{60}
	, m_render_ticks_per_second{120}
	, m_input_ticks_per_second{120}
	, maxFrameDelta{std::chrono::milliseconds(250)}
{
	Logger::s_editor_sink = &m_editor;
}
Application::~Application() noexcept
{
	Logger::s_editor_sink = nullptr;
}

void Application::simulation_loop()
{
	while (!m_window.close_requested())
	{
		ASSERT(m_physics_ticks_per_second == 30   || m_physics_ticks_per_second == 60   || m_physics_ticks_per_second == 90   || m_physics_ticks_per_second == 120,   "[APPLICATION] Physics ticks per second requested invalid!");
		ASSERT(m_render_ticks_per_second == 30    || m_render_ticks_per_second == 60    || m_render_ticks_per_second == 90    || m_render_ticks_per_second == 120,    "[APPLICATION] Render ticks per second requested invalid!");
		ASSERT(m_input_ticks_per_second == 30 || m_input_ticks_per_second == 60 || m_input_ticks_per_second == 90 || m_input_ticks_per_second == 120, "[APPLICATION] Input ticks per second requested invalid!");

		switch (m_physics_ticks_per_second)
		{
			case 30:
			{
				switch (m_render_ticks_per_second)
				{
					case 30:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<30, 30, 30>();    break;
							case 60:  simulation_loop<30, 30, 60>();    break;
							case 90:  simulation_loop<30, 30, 90>();    break;
							case 120: simulation_loop<30, 30, 120>();   break;
						} break;
					}
					case 60:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<30, 60, 30>();    break;
							case 60:  simulation_loop<30, 60, 60>();    break;
							case 90:  simulation_loop<30, 60, 90>();    break;
							case 120: simulation_loop<30, 60, 120>();   break;
						} break;
					}
					case 90:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<30, 90, 30>();    break;
							case 60:  simulation_loop<30, 90, 60>();    break;
							case 90:  simulation_loop<30, 90, 90>();    break;
							case 120: simulation_loop<30, 90, 120>();   break;
						} break;
					}
					case 120:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<30, 120, 30>();   break;
							case 60:  simulation_loop<30, 120, 60>();   break;
							case 90:  simulation_loop<30, 120, 90>();   break;
							case 120: simulation_loop<30, 120, 120>();  break;
						} break;
					}
				} break;
			}
			case 60:
			{
				switch (m_render_ticks_per_second)
				{
					case 30:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<60, 30, 30>();    break;
							case 60:  simulation_loop<60, 30, 60>();    break;
							case 90:  simulation_loop<60, 30, 90>();    break;
							case 120: simulation_loop<60, 30, 120>();   break;
						} break;
					}
					case 60:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<60, 60, 30>();    break;
							case 60:  simulation_loop<60, 60, 60>();    break;
							case 90:  simulation_loop<60, 60, 90>();    break;
							case 120: simulation_loop<60, 60, 120>();   break;
						} break;
					}
					case 90:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<60, 90, 30>();    break;
							case 60:  simulation_loop<60, 90, 60>();    break;
							case 90:  simulation_loop<60, 90, 90>();    break;
							case 120: simulation_loop<60, 90, 120>();   break;
						} break;
					}
					case 120:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<60, 120, 30>();   break;
							case 60:  simulation_loop<60, 120, 60>();   break;
							case 90:  simulation_loop<60, 120, 90>();   break;
							case 120: simulation_loop<60, 120, 120>();  break;
						} break;
					}
				} break;
			}
			case 90:
			{
				switch (m_render_ticks_per_second)
				{
					case 30:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<90, 30, 30>();    break;
							case 60:  simulation_loop<90, 30, 60>();    break;
							case 90:  simulation_loop<90, 30, 90>();    break;
							case 120: simulation_loop<90, 30, 120>();   break;
						} break;
					}
					case 60:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<90, 60, 30>();    break;
							case 60:  simulation_loop<90, 60, 60>();    break;
							case 90:  simulation_loop<90, 60, 90>();    break;
							case 120: simulation_loop<90, 60, 120>();   break;
						} break;
					}
					case 90:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<90, 90, 30>();    break;
							case 60:  simulation_loop<90, 90, 60>();    break;
							case 90:  simulation_loop<90, 90, 90>();    break;
							case 120: simulation_loop<90, 90, 120>();   break;
						} break;
					}
					case 120:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<90, 120, 30>();   break;
							case 60:  simulation_loop<90, 120, 60>();   break;
							case 90:  simulation_loop<90, 120, 90>();   break;
							case 120: simulation_loop<90, 120, 120>();  break;
						} break;
					}
				} break;
			}
			case 120:
			{
				switch (m_render_ticks_per_second)
				{
					case 30:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<120, 30, 30>();   break;
							case 60:  simulation_loop<120, 30, 60>();   break;
							case 90:  simulation_loop<120, 30, 90>();   break;
							case 120: simulation_loop<120, 30, 120>();  break;
						} break;
					}
					case 60:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<120, 60, 30>();   break;
							case 60:  simulation_loop<120, 60, 60>();   break;
							case 90:  simulation_loop<120, 60, 90>();   break;
							case 120: simulation_loop<120, 60, 120>();  break;
						} break;
					}
					case 90:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<120, 90, 30>();   break;
							case 60:  simulation_loop<120, 90, 60>();   break;
							case 90:  simulation_loop<120, 90, 90>();   break;
							case 120: simulation_loop<120, 90, 120>();  break;
						} break;
					}
					case 120:
					{
						switch (m_input_ticks_per_second)
						{
							case 30:  simulation_loop<120, 120, 30>();  break;
							case 60:  simulation_loop<120, 120, 60>();  break;
							case 90:  simulation_loop<120, 120, 90>();  break;
							case 120: simulation_loop<120, 120, 120>(); break;
						} break;
					}
				} break;
			}
		}

		// After exiting a simulation loop we may have requested a physics timestep change.
		// Reset this flag to not exit the next simulation_loop when looping back around this While().
		m_simulation_loop_params_changed = false;
	}
}