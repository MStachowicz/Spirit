#pragma once

#include "Geometry/Ray.hpp"
#include "UI/Console.hpp"

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace Platform
{
	enum class Key : uint8_t;
	enum class MouseButton;
	enum class Action;
	enum class CursorMode;

	class Window;
	class Input;
}
namespace System
{
	class TextureSystem;
	class MeshSystem;
	class SceneSystem;
	class CollisionSystem;
}
namespace OpenGL
{
	class OpenGLRenderer;
}
namespace ECS
{
	class Entity;
}

namespace UI
{
	// Editor is a Debug-build only overlay for Zephyr that provides a UI for interaction using ImGui.
	// The individual Component clases define their own draw_UI functions which the Editor is in charge of calling.
	// Editor also handles the rendering of the FPS counter and the Console.
	class Editor
	{
		struct Windows
		{
			bool Entity                  = false;
			bool FPSTimer                = true;
			bool Debug                   = false;
			bool ImGuiDemo               = false;
			bool ImGuiMetrics            = false;
			bool ImGuiStack              = false;
			bool ImGuiAbout              = false;
			bool ImGuiStyleEditor        = false;
			bool Console                 = true;
		};

		Platform::Input&         m_input;
		Platform::Window&        m_window;
		System::TextureSystem&   m_texture_system;
		System::MeshSystem&      m_mesh_system;
		System::SceneSystem&     m_scene_system;
		System::CollisionSystem& m_collision_system;
		OpenGL::OpenGLRenderer&  m_openGL_renderer;

		std::vector<Geometry::Ray> m_click_rays; // On mouse click save the Ray to render.
		std::vector<ECS::Entity> m_selected_entities;
		Console m_console;
		Windows m_windows_to_display; // All the windows currently being displayed

	public:
		using DeltaTime = std::chrono::duration<float, std::ratio<1>>; // Represents a float precision duration in seconds.

		int m_draw_count;
		DeltaTime m_time_to_average_over; // The time over which to average out the fps.
		std::vector<DeltaTime> m_duration_between_draws;

		// Returns the fps averaged over the last p_time_period duration of p_times.
		template <typename Rep, typename Period>
		Rep get_fps(const std::vector<std::chrono::duration<Rep, Period>>& p_times, const std::chrono::duration<Rep, Period>& p_time_period)
		{
			// We iterate through p_times backwards, accumulate the total duration and num_frames, and break out of the loop if the accumulated duration exceeds the p_time_period.
			// Finally, we calculate the time per frame, convert it to frames per second, and return the FPS value.
			if (p_times.empty())
				return 0.0;

			std::chrono::duration<Rep, Period> total_duration = std::chrono::duration<Rep, Period>::zero();
			size_t num_frames = 0;

			for (auto it = p_times.rbegin(); it != p_times.rend(); ++it)
			{
				if (total_duration + *it > p_time_period)
					break;

				total_duration += *it;
				num_frames++;
			}

			auto avg_time_per_frame = total_duration / num_frames;
			return 1.f / std::chrono::duration_cast<std::chrono::duration<Rep>>(avg_time_per_frame).count();
		}

		Editor(Platform::Input& p_input, Platform::Window& p_window, System::TextureSystem& p_texture_system, System::MeshSystem& p_mesh_system, System::SceneSystem& p_scene_system, System::CollisionSystem& p_collision_system, OpenGL::OpenGLRenderer& p_openGL_renderer);

		void draw(const DeltaTime& p_duration_since_last_draw);

		void log(const std::string& p_message);
		void log_warning(const std::string& p_message);
		void log_error(const std::string& p_message);

	private:
		void on_mouse_event(Platform::MouseButton p_button, Platform::Action p_action);
		void on_key_event(Platform::Key p_key, Platform::Action p_action);

		void draw_entity_tree_window();
		void draw_debug_window();
		void draw_console_window();

		void initialiseStyling();
	};
} // namespace UI