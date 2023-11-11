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
		struct DebugOptions // Options belonging to the Debug Window
		{
			// Rendering
			bool m_show_light_positions = false;
			bool m_show_mesh_normals    = false;
			// Physics
			bool m_show_orientations    = false; // Draw an arrow in the direction the meshes are facing.
			bool m_show_bounding_box    = false; // Draw the bounding boxes of the meshes. Used for broad phase collision detection.
			bool m_fill_bounding_box    = false; // Fill the bounding boxes of the meshes. Only valid if m_show_bounding_box is true.
			bool m_show_collision_shape = false; // Draw the collision shape of the meshes.
		};

		Platform::Input&         m_input;
		Platform::Window&        m_window;
		System::TextureSystem&   mTextureSystem;
		System::MeshSystem&      mMeshSystem;
		System::SceneSystem&     mSceneSystem;
		System::CollisionSystem& mCollisionSystem;
		OpenGL::OpenGLRenderer&  mOpenGLRenderer;

		std::vector<Geometry::Ray> m_click_rays; // On mouse click save the Ray to render.
		std::vector<ECS::Entity> mSelectedEntities;
		DebugOptions m_debug_options;
		Console m_console;
		Windows mWindowsToDisplay; // All the windows currently being displayed

	public:
		using DeltaTime = std::chrono::duration<float, std::ratio<1>>; // Represents a float precision duration in seconds.

		int mDrawCount;
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

		Editor(Platform::Input& p_input, Platform::Window& p_window, System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem, System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer);

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