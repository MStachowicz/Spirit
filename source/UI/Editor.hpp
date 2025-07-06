#pragma once

#include "UI/Console.hpp"
#include "Component/TwoAxisCamera.hpp"
#include "ECS/Entity.hpp"

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Platform
{
	enum class Key         : uint8_t;
	enum class MouseButton : uint8_t;
	enum class Action      : uint8_t;
	enum class CursorMode  : uint8_t;

	class Window;
	class Input;
}
namespace System
{
	class AssetManager;
	class SceneSystem;
	class Scene;
	class CollisionSystem;
	class PhysicsSystem;
}
namespace OpenGL
{
	class OpenGLRenderer;
}
namespace UI
{
	// Editor is a Debug-build only overlay for Spirit that provides a UI for interaction using ImGui.
	// The individual Component clases define their own draw_UI functions which the Editor is in charge of calling.
	// Editor also handles the rendering of the FPS counter and the Console.
	class Editor
	{
		enum class State
		{
			Editing,       // The editor is active and the user is interacting with the scene.
			Playing,       // The editor is inactive and the scene is running.
			CameraTesting  // The editor is active and the cursor is captured.
		};

		struct Windows
		{
			bool Entity           = false;
			bool FPSTimer         = true;
			bool add_entity_popup = false;
			bool ent_properties   = false;
			bool Graphics_Debug   = false;
			bool Physics_Debug    = false;
			bool Performance      = false;
			bool ImGuiDemo        = false;
			bool ImGuiMetrics     = false;
			bool ImGuiStack       = false;
			bool ImGuiAbout       = false;
			bool theme_editor     = false;
			bool editor_camera    = false;
			bool ImGuiStyleEditor = false;
			bool Console          = true;
			bool asset_browser    = false;
		};

		struct PlayerInfoWindow
		{
			bool open = false;
			bool render_player_position      = false;
			bool render_player_view_distance = false;
			bool render_player_frustrum      = false;

			void draw(Editor& p_editor);
			void reset()
			{
				render_player_position      = false;
				render_player_view_distance = false;
				render_player_frustrum      = false;
			}
		};

		Platform::Input&         m_input;
		Platform::Window&        m_window;
		System::AssetManager&    m_asset_manager;
		System::SceneSystem&     m_scene_system;
		System::CollisionSystem& m_collision_system;
		System::PhysicsSystem&   m_physics_system;
		OpenGL::OpenGLRenderer&  m_openGL_renderer;

		State m_state;                          // The current state of the editor.
		System::Scene* m_scene_before_play;     // The scene being edited before play was pressed.
		Component::TwoAxisCamera m_camera;      // Camera used when m_state is Editing.
		Component::ViewInformation m_view_info; // View information for m_camera required to provide persistant memory.
		std::vector<ECS::Entity> m_selected_entities;
		std::optional<ECS::Entity> m_entity_to_draw_info_for; // The entity for which to draw the UI. When a new entity is selected, this is set to the new entity.
		// The last intersection of the cursor with the scene.
		// Sometimes we need the cursor intersection earlier in the action (e.g. add_entity_popup should interesect at the point of right click not menu selection.
		std::optional<glm::vec3> m_cursor_intersection;
		Console m_console;
		Windows m_windows_to_display; // All the windows currently being displayed
		PlayerInfoWindow m_player_info_window;
		bool m_dragging;              // Is the user currently dragging the mouse. i.e. any mouse button is down while the mouse is moving.

		bool m_draw_axes; // Whether to draw the axes in the editor.
		bool m_debug_GJK;
		std::optional<ECS::Entity> m_debug_GJK_entity_1;
		std::optional<ECS::Entity> m_debug_GJK_entity_2;
		int m_debug_GJK_step;

		std::optional<size_t> pie_chart_node_index; // The current performance node being drawn. Nullptr = root.
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

		Editor(Platform::Input& p_input, Platform::Window& p_window
			, System::AssetManager& p_asset_manager
			, System::SceneSystem& p_scene_system
			, System::CollisionSystem& p_collision_system
			, System::PhysicsSystem& p_physics_system
			, OpenGL::OpenGLRenderer& p_openGL_renderer);

		void draw(const DeltaTime& p_duration_since_last_draw);
		//@return ViewInformation representing the state of the camera if editor is active, otherwise nullptr.
		Component::ViewInformation* get_editor_view_info();

		void log(const std::string& p_message);
		void log_warning(const std::string& p_message);
		void log_error(const std::string& p_message);

	private:
		void on_mouse_move_event(const glm::vec2 p_mouse_delta);
		void on_mouse_scroll_event(const glm::vec2 p_mouse_scroll);
		void on_mouse_button_event(Platform::MouseButton p_button, Platform::Action p_action);
		void on_key_event(Platform::Key p_key, Platform::Action p_action);

		void select_entity(ECS::Entity& p_entity);
		void deselect_entity(ECS::Entity& p_entity);
		void deselect_all_entity();

		void set_state(State p_new_state, bool p_force = false);

		// For all components of the entity, draw_component_UI.
		void draw_entity_UI(ECS::Entity& p_entity);
		// For all entities in the scene, draw_entity_UI.
		void draw_entity_tree_window();
		// If m_entity_to_draw_info_for is set, draw_entity_UI for the entity in a window.
		void draw_entity_properties();
		void draw_graphics_debug_window();
		void draw_physics_debug_window();
		void draw_console_window();
		void draw_performance_window();

		struct PieSlice
		{
			float percentage;
			glm::vec4 colour;
			std::string label;

			std::function<void()> on_click;
			std::function<void()> on_right_click;
			std::function<void()> on_hover;
		};
		void draw_pie_chart(const std::vector<PieSlice>& slices, const std::function<void()>& on_inner_circle_click = nullptr, const std::function<void()>& on_inner_circle_hover = nullptr, bool draw_border = true);

		void entity_creation_popup();

		void initialiseStyling();
	};
} // namespace UI