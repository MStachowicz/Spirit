#include "Editor.hpp"
#include "Visualisers.hpp"

#include "Component/Collider.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/Label.hpp"
#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/ParticleEmitter.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Terrain.hpp"
#include "Component/Texture.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/AssetManager.hpp"
#include "System/CollisionSystem.hpp"
#include "System/PhysicsSystem.hpp"
#include "System/SceneSystem.hpp"

#include "OpenGL/DebugRenderer.hpp"
#include "OpenGL/OpenGLRenderer.hpp"
#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Performance.hpp"
#include "Utility/Utility.hpp"

#include "imgui.h"
#include "ImGuizmo.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <format>
#include <numbers>
#include <cstdint>

namespace UI
{
	Editor::Editor(Platform::Input& p_input, Platform::Window& p_window
		, System::AssetManager& p_asset_manager
		, System::SceneSystem& p_scene_system
		, System::CollisionSystem& p_collision_system
		, System::PhysicsSystem& p_physics_system
		, OpenGL::OpenGLRenderer& p_openGL_renderer)
		: m_input{p_input}
		, m_window{p_window}
		, m_asset_manager{p_asset_manager}
		, m_scene_system{p_scene_system}
		, m_collision_system{p_collision_system}
		, m_physics_system{p_physics_system}
		, m_openGL_renderer{p_openGL_renderer}
		, m_state{State::Editing}
		, m_scene_before_play{nullptr}
		, m_selected_entities{}
		, m_entity_to_draw_info_for{}
		, m_cursor_intersection{}
		, m_console{}
		, m_panes_to_display{}
		, m_player_info_pane{}
		, m_viewport_pane{}
		, m_dragging{false}
		, m_debug_GJK{false}
		, m_debug_GJK_entity_1{}
		, m_debug_GJK_entity_2{}
		, m_debug_GJK_step{0}
		, pie_chart_node_index{std::nullopt}
		, m_draw_count{0}
		, m_time_to_average_over{std::chrono::seconds(1)}
		, m_duration_between_draws{}
	{
		m_input.m_mouse_button_event.subscribe(this, &Editor::on_mouse_button_event);
		m_input.m_key_event.subscribe(this,          &Editor::on_key_event);
		m_input.m_mouse_move_event.subscribe(this,   &Editor::on_mouse_move_event);
		m_input.m_mouse_scroll_event.subscribe(this, &Editor::on_mouse_scroll_event);

		set_state(m_state, true); // Set the initial state.
		initialiseStyling();
	}

	void Editor::on_mouse_button_event(Platform::MouseButton p_button, Platform::Action p_action)
	{
		if (m_state != State::Editing)
			return;

		if (m_viewport_pane.m_cursor_hovered)
		{
			switch (p_button)
			{
				case Platform::MouseButton::Left:
				{
					if (p_action == Platform::Action::Press)
					{
						auto view_info            = m_viewport_pane.view_information();
						auto cursor_ray           = Utility::get_cursor_ray(m_viewport_pane.m_cursor_pos_content, m_viewport_pane.m_FBO.resolution(), view_info.m_view_position, view_info.m_projection, view_info.m_view);
						auto entities_under_mouse = m_collision_system.get_entities_along_ray(cursor_ray);

						if (!entities_under_mouse.empty())
						{
							std::sort(entities_under_mouse.begin(), entities_under_mouse.end(), [](const auto& left, const auto& right) { return left.second < right.second; });
							auto entity_collided = entities_under_mouse.front().first;

							// If the entity is not already selected, select it.
							auto it = std::find(m_selected_entities.begin(), m_selected_entities.end(), entity_collided);
							if (it == m_selected_entities.end())
								select_entity(entity_collided);
							else// If the entity is already selected, deselect it.
								deselect_entity(entity_collided);
						}
						else
						{
							deselect_all_entity();
						}
					}
					break;
				}
				case Platform::MouseButton::Right:
				{
					// If the user was not dragging, open the entity creation popup.
					if (p_action == Platform::Action::Release && !m_dragging)
					{
						auto view_info   = m_scene_system.get_current_scene_view_info();
						auto cursor_ray  = Utility::get_cursor_ray(m_input.cursor_position(), m_viewport_pane.m_FBO.resolution(), view_info.m_view_position, view_info.m_projection, view_info.m_view);
						auto floor_plane = Geometry::Plane{glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)};
						if ((m_cursor_intersection = Geometry::get_intersection(cursor_ray, floor_plane)))
							m_panes_to_display.add_entity_popup = true;
					}

					break;
				}
				case Platform::MouseButton::Middle: break;
				default: break;
			}
		}

		// We update the m_dragging flag here so above usages of m_dragging act a frame behind avoiding additional need for m_dragging ended state.
		if (p_action == Platform::Action::Release && m_dragging && !m_input.is_any_mouse_down())
			m_dragging = false;
	}

	void Editor::on_mouse_move_event(const glm::vec2 p_mouse_delta)
	{
		if (m_viewport_pane.m_cursor_hovered)
		{
			if (m_state == State::Editing)
			{
				if (m_input.is_mouse_down(Platform::MouseButton::Right))
				{
					m_viewport_pane.m_camera.mouse_look(p_mouse_delta);
				}
				else if (m_input.is_mouse_down(Platform::MouseButton::Middle))
				{
					m_viewport_pane.m_camera.pan(p_mouse_delta);
				}
			}
		}

		if (!m_dragging && m_input.is_any_mouse_down())
			m_dragging = true;
	}
	void Editor::on_mouse_scroll_event(const glm::vec2 p_mouse_scroll)
	{
		if (m_viewport_pane.m_cursor_hovered)
		{
			if (m_state == State::Editing)
			{
				m_viewport_pane.m_camera.zoom(p_mouse_scroll.y);
			}
		}
	}
	void Editor::on_key_event(Platform::Key p_key, Platform::Action p_action)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
			return;

		switch (p_key)
		{
			case Platform::Key::Left_Arrow:
			{
				if (p_action == Platform::Action::Release || p_action == Platform::Action::Repeat)
				{
					if (m_debug_GJK && m_debug_GJK_entity_1 && m_debug_GJK_entity_2 && m_debug_GJK_step > 0)
						m_debug_GJK_step--;
				}
				break;
			}
			case Platform::Key::Right_Arrow:
			{
				if (p_action == Platform::Action::Release || p_action == Platform::Action::Repeat)
				{
					if (m_debug_GJK && m_debug_GJK_entity_1 && m_debug_GJK_entity_2)
						m_debug_GJK_step++;
				}
				break;
			}
			case Platform::Key::C:
			{
				if (p_action == Platform::Action::Release)
				{
					set_state(m_state == State::CameraTesting ? State::Editing : State::CameraTesting);
				}
				break;
			}
			case Platform::Key::E:
			{
				if (p_action == Platform::Action::Release)
					m_panes_to_display.Entity = !m_panes_to_display.Entity;
				break;
			}
			case Platform::Key::G:
			{
				if (p_action == Platform::Action::Release)
				{
					if (!m_debug_GJK && m_selected_entities.size() == 2)
					{
						m_debug_GJK = true;
						m_debug_GJK_step = 0;
						m_debug_GJK_entity_1 = m_selected_entities[0];
						m_debug_GJK_entity_2 = m_selected_entities[1];
					}
					else
					{
						m_debug_GJK = false;
						m_debug_GJK_entity_1.reset();
						m_debug_GJK_entity_2.reset();
					}
				}
				break;
			}
			case Platform::Key::P:
			{
				if (p_action == Platform::Action::Release && m_state == State::Editing)
					if (m_viewport_pane.m_focused)
						m_viewport_pane.m_camera.toggle_orthographic();
				break;
			}
			case Platform::Key::U:
			{
				if (p_action == Platform::Action::Release)
					m_panes_to_display.Console = !m_panes_to_display.Console;
				break;
			}
			case Platform::Key::Escape:
			{
				if (p_action == Platform::Action::Release)
					if (ImGui::GetIO().WantCaptureKeyboard == false)
					{
						if (m_state == State::Playing || m_state == State::CameraTesting)
							set_state(State::Editing);
						else
							m_window.request_close();
					}
				break;
			}
			case Platform::Key::Space:
			{
				// Toggle playing and editing mode
				if (p_action == Platform::Action::Release)
					if (ImGui::GetIO().WantCaptureKeyboard == false)
						set_state(m_state == State::Editing ? State::Playing : State::Editing);

				break;
			}
			case Platform::Key::F11:
			{
				if (p_action == Platform::Action::Release)
					m_window.toggle_fullscreen();

				break;
			}
			default: break;
		}
	}

	void Editor::select_entity(ECS::Entity& p_entity)
	{
		// If the entity isn't already in the m_selected_entities list, add it.
		auto it = std::find(m_selected_entities.begin(), m_selected_entities.end(), p_entity);
		if (it == m_selected_entities.end())
		{
			m_selected_entities.push_back(p_entity);
			m_entity_to_draw_info_for = p_entity;
			LOG("[EDITOR] Entity {} has been selected", p_entity.ID);
		}
	}
	void Editor::deselect_entity(ECS::Entity& p_entity)
	{
		// If the entity is in the m_selected_entities list, remove it.
		auto it = std::find(m_selected_entities.begin(), m_selected_entities.end(), p_entity);

		if (it != m_selected_entities.end())
		{
			m_selected_entities.erase(it);
			if (m_entity_to_draw_info_for == p_entity)
				m_entity_to_draw_info_for.reset();

			LOG("[EDITOR] Entity {} has been deselected", p_entity.ID);
		}
	}
	void Editor::deselect_all_entity()
	{
		if (!m_selected_entities.empty())
		{
			m_selected_entities.clear();
			m_entity_to_draw_info_for.reset();

			LOG("[EDITOR] Deselected all entities");
		}
	}

	void Editor::set_state(State p_new_state, bool p_force /*= false*/)
	{
		if (m_state == p_new_state && !p_force)
			return;

		switch (p_new_state)
		{
			case State::Editing:
			{
				m_input.set_cursor_mode(Platform::CursorMode::Normal);
				m_window.m_show_menu_bar                = true;
				m_physics_system.m_bool_apply_kinematic = false;

				if (m_scene_before_play)
					m_scene_system.set_current_scene(*m_scene_before_play);
				break;
			}
			case State::Playing:
			{
				m_input.set_cursor_mode(Platform::CursorMode::Captured);
				m_window.m_show_menu_bar = false;
				deselect_all_entity();

				// Create a new scene and copy the current scene into it.
				// This is so that the current scene can be restored when the user stops playing.
				m_scene_before_play = &m_scene_system.get_current_scene();
				auto& play_scene    = m_scene_system.add_scene();
				play_scene          = m_scene_system.get_current_scene();
				m_scene_system.set_current_scene(play_scene);
				m_physics_system.m_bool_apply_kinematic = true;
				break;
			}
			case State::CameraTesting:
			{
				m_input.set_cursor_mode(Platform::CursorMode::Captured);
				m_window.m_show_menu_bar                = true;
				m_physics_system.m_bool_apply_kinematic = false;
				if (m_scene_before_play && m_state == State::Playing) // Only reset the scene if the user was playing before entering camera testing.
					m_scene_system.set_current_scene(*m_scene_before_play);
				break;
			}
			default: break;
		}

		m_state = p_new_state;
	}
	std::optional<Component::ViewInformation> Editor::get_editor_view_info()
	{
		if (m_state == State::Editing || m_state == State::CameraTesting)
			return m_viewport_pane.view_information();
		else
			return {};
	}

	void Editor::draw(const DeltaTime& p_duration_since_last_draw)
	{
		PERF(EditorDraw);

		if (m_state == State::Playing)
			return;

		m_duration_between_draws.push_back(p_duration_since_last_draw);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					auto file_path = Platform::file_dialog(Platform::FileDialogType::Save, Platform::FileDialogFilter::Scene, "Save scene", Config::Scene_Save_Directory);
					if (!file_path.empty())
					{
						std::ofstream file(file_path);
						auto& scene = m_scene_system.get_current_scene();
						System::Scene::serialise(file, Config::Save_Version, scene);
					}
				}
				if (ImGui::MenuItem("Load"))
				{
					auto file_path = Platform::file_dialog(Platform::FileDialogType::Open, Platform::FileDialogFilter::Scene, "Load scene", Config::Scene_Save_Directory);
					if (!file_path.empty())
					{
						std::ifstream file(file_path);
						auto& scene = m_scene_system.add_scene();
						scene       = System::Scene::deserialise(file, Config::Save_Version);
						m_scene_system.set_current_scene(scene);
					}
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Entity tree", NULL, &m_panes_to_display.Entity);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Viewport",           NULL, &m_viewport_pane.open);
				ImGui::MenuItem("Console",            NULL, &m_panes_to_display.Console);
				ImGui::MenuItem("Asset browser",      NULL, &m_panes_to_display.asset_browser);
				ImGui::Separator();
				ImGui::MenuItem("Camera",             NULL, &m_panes_to_display.editor_camera);
				ImGui::MenuItem("FPS Timer",          NULL, &m_panes_to_display.FPSTimer);
				ImGui::Separator();
				ImGui::MenuItem("Theme",              NULL, &m_panes_to_display.theme_editor);
				ImGui::MenuItem("ImGui style editor", NULL, &m_panes_to_display.ImGuiStyleEditor);

				if (ImGui::BeginMenu("ImGui"))
				{
					ImGui::MenuItem("Demo",  NULL, &m_panes_to_display.ImGuiDemo);
					ImGui::MenuItem("Stack", NULL, &m_panes_to_display.ImGuiStack);
					ImGui::MenuItem("About", NULL, &m_panes_to_display.ImGuiAbout);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug"))
			{
				ImGui::MenuItem("Performance",            NULL, &m_panes_to_display.Performance);
				ImGui::MenuItem("Graphics",               NULL, &m_panes_to_display.Graphics_Debug);
				ImGui::MenuItem("Physics",                NULL, &m_panes_to_display.Physics_Debug);
				ImGui::MenuItem("ImGui Metrics/Debugger", NULL, &m_panes_to_display.ImGuiMetrics);
				ImGui::MenuItem("Show Terrain Nodes",     NULL, &m_openGL_renderer.m_draw_terrain_nodes);
				ImGui::MenuItem("Draw Terrain Wireframe", NULL, &m_openGL_renderer.m_draw_terrain_wireframe);
				ImGui::MenuItem("Show Player info",       NULL, &m_player_info_pane.open);
				ImGui::Separator();

				if (ImGui::MenuItem("Play"))
					set_state(State::Playing);
				if (ImGui::MenuItem("Camera Test"))
					set_state(State::CameraTesting);
				ImGui::EndMenu();
			}
			if (m_panes_to_display.FPSTimer)
			{
				auto fps = get_fps(m_duration_between_draws, m_time_to_average_over);
				std::string fps_str = std::format("FPS: {:.1f}", fps);
				ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(fps_str.c_str()).x - ImGui::GetStyle().ItemSpacing.x) / 2.f);

				glm::vec4 colour;
				if      (fps > 60.f) colour = Platform::Core::s_theme.success_text;
				else if (fps > 30.f) colour = Platform::Core::s_theme.warning_text;
				else                 colour = Platform::Core::s_theme.error_text;

				ImGui::TextColored(colour, "%s", fps_str.c_str());
			}
			ImGui::EndMenuBar();
		}
		if (m_panes_to_display.Entity)         draw_entity_tree_pane();
		if (m_panes_to_display.Console)        m_console.draw("Console", &m_panes_to_display.Console);
		if (m_panes_to_display.asset_browser)  m_asset_manager.draw_UI(&m_panes_to_display.asset_browser);
		if (m_panes_to_display.Performance)    draw_performance_pane();
		if (m_panes_to_display.Graphics_Debug) draw_graphics_debug_pane();
		if (m_panes_to_display.Physics_Debug)  draw_physics_debug_pane();
		m_player_info_pane.draw(*this);
		if (m_panes_to_display.ImGuiDemo)      ImGui::ShowDemoWindow(&m_panes_to_display.ImGuiDemo);
		if (m_panes_to_display.ImGuiMetrics)   ImGui::ShowMetricsWindow(&m_panes_to_display.ImGuiMetrics);
		if (m_panes_to_display.ImGuiStack)     ImGui::ShowStackToolWindow(&m_panes_to_display.ImGuiStack);
		if (m_panes_to_display.ImGuiAbout)     ImGui::ShowAboutWindow(&m_panes_to_display.ImGuiAbout);
		if (m_panes_to_display.theme_editor)   Platform::Core::s_theme.draw_theme_editor_UI();
		if (m_panes_to_display.ImGuiStyleEditor)
		{
			ImGui::Begin("Dear ImGui Style Editor", &m_panes_to_display.ImGuiStyleEditor);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
		if (m_panes_to_display.editor_camera)
		{
			ImGui::Begin("Editor Camera", &m_panes_to_display.editor_camera, ImGuiWindowFlags_AlwaysAutoResize);
			m_viewport_pane.m_camera.draw_UI();
			ImGui::End();
		}

		m_viewport_pane.draw(p_duration_since_last_draw, m_openGL_renderer);
		draw_entity_properties();
		entity_creation_popup();

		{// Manipulators
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			auto window_pos  = ImGui::GetWindowPos();
			auto window_size = ImGui::GetWindowSize();
			ImGuizmo::SetRect(window_pos.x, window_pos.y, window_size.x, window_size.y);

			if (!m_selected_entities.empty())
			{
				// Find the first selected entity from the back of m_selected_entities that has a transform component
				auto it = std::find_if(m_selected_entities.rbegin(), m_selected_entities.rend(), [&](const auto& p_entity)
					{ return m_scene_system.get_current_scene_entities().has_components<Component::Transform>(p_entity); });

				if (it != m_selected_entities.rend())
				{
					auto& transform = m_scene_system.get_current_scene_entities().get_component<Component::Transform>(*it);
					glm::mat4 model = transform.get_model();
					ImGuizmo::Manipulate(
						glm::value_ptr(m_scene_system.get_current_scene_view_info().m_view),
						glm::value_ptr(m_scene_system.get_current_scene_view_info().m_projection),
						ImGuizmo::OPERATION::UNIVERSAL,
						ImGuizmo::LOCAL,
						glm::value_ptr(model));

					if (ImGuizmo::IsUsing())
						transform.set_model(model);
				}
			}
		}

		m_draw_count++;
	}
	void Editor::draw_entity_UI(ECS::Entity& p_entity)
	{
		auto& scene = m_scene_system.get_current_scene_entities();

		if (scene.has_components<Component::Transform>(p_entity))
			scene.get_component<Component::Transform&>(p_entity).draw_UI();
		if (scene.has_components<Component::Collider>(p_entity))
			scene.get_component<Component::Collider&>(p_entity).draw_UI();
		if (scene.has_components<Component::RigidBody>(p_entity))
			scene.get_component<Component::RigidBody&>(p_entity).draw_UI();
		if (scene.has_components<Component::DirectionalLight>(p_entity))
			scene.get_component<Component::DirectionalLight&>(p_entity).draw_UI();
		if (scene.has_components<Component::SpotLight>(p_entity))
			scene.get_component<Component::SpotLight&>(p_entity).draw_UI();
		if (scene.has_components<Component::PointLight>(p_entity))
			scene.get_component<Component::PointLight&>(p_entity).draw_UI();
		if (scene.has_components<Component::FirstPersonCamera>(p_entity))
			scene.get_component<Component::FirstPersonCamera>(p_entity).draw_UI();
		if (scene.has_components<Component::ParticleEmitter>(p_entity))
			scene.get_component<Component::ParticleEmitter>(p_entity).draw_UI(m_asset_manager);
		if (scene.has_components<Component::Terrain>(p_entity))
			scene.get_component<Component::Terrain>(p_entity).draw_UI(m_asset_manager);
		if (scene.has_components<Component::Mesh>(p_entity))
			scene.get_component<Component::Mesh>(p_entity).draw_UI();
		if (scene.has_components<Component::Texture>(p_entity))
			scene.get_component<Component::Texture>(p_entity).draw_UI(m_asset_manager);

		ImGui::SeparatorText("Quick options");
		if (ImGui::Button("Delete entity"))
		{
			// If the entity was selected, remove it from the selected entities list.
			deselect_entity(p_entity);
			scene.delete_entity(p_entity);
		}
	}
	void Editor::draw_entity_tree_pane()
	{
		if (ImGui::Begin("Entities", &m_panes_to_display.Entity))
		{
			auto& scene = m_scene_system.get_current_scene_entities();
			scene.foreach([&](ECS::Entity& p_entity)
			{
				ImGui::PushID(static_cast<int>(p_entity.ID));
				std::string title = "Entity " + std::to_string(p_entity.ID);
				if (scene.has_components<Component::Label>(p_entity))
				{
					auto label = scene.get_component<Component::Label&>(p_entity);
					title = label.m_name;
				}

				if (ImGui::TreeNode(title.c_str()))
				{
					draw_entity_UI(p_entity);

					ImGui::Separator();
					ImGui::TreePop();
				}
				ImGui::PopID();
			});
		}
		ImGui::End();
	}
	void Editor::draw_entity_properties()
	{
		// If we have an entity to draw info for, draw the UI for it.
		m_panes_to_display.ent_properties = m_entity_to_draw_info_for.has_value();

		if (m_entity_to_draw_info_for)
		{
			auto& ent   = *m_entity_to_draw_info_for;
			auto& scene = m_scene_system.get_current_scene_entities();

			std::string title = "";
			if (scene.has_components<Component::Label>(ent))
			{
				auto label = scene.get_component<Component::Label&>(ent);
				title = label.m_name;
			}
			else
				title = "Entity " + std::to_string(ent.ID);

			ImGui::Begin(title.c_str(), &m_panes_to_display.ent_properties);
			draw_entity_UI(ent);
			ImGui::End();

			if (!m_panes_to_display.ent_properties)
				m_entity_to_draw_info_for.reset(); // If the window is closed, reset the entity to draw info for.
		}
	}
	void Editor::draw_graphics_debug_pane()
	{
		auto& debug_options = OpenGL::DebugRenderer::m_debug_options;

		if (ImGui::Begin("Graphics", &m_panes_to_display.Graphics_Debug))
		{
			ImGui::Text_Manual("Window:   %dx%d (Aspect Ratio: %.2f)", m_window.size().x, m_window.size().y, m_window.aspect_ratio());
			ImGui::Text_Manual("Viewport: %dx%d (Aspect Ratio: %.2f)", m_viewport_pane.m_FBO.resolution().x, m_viewport_pane.m_FBO.resolution().y, m_viewport_pane.aspect_ratio());

			ImGui::Text("Frame count",   m_draw_count);
			ImGui::Separator();
			ImGui::Text("View Position", m_scene_system.get_current_scene_view_info().m_view_position);
			ImGui::Text("View",          m_scene_system.get_current_scene_view_info().m_view);
			ImGui::Text("Proj",          m_scene_system.get_current_scene_view_info().m_projection);
			ImGui::Separator();
			ImGui::Checkbox("Show light positions", &debug_options.m_show_light_positions);
			ImGui::Checkbox("Show player frustrum", &m_player_info_pane.render_player_frustrum);
			ImGui::Checkbox("Visualise normals",    &debug_options.m_show_mesh_normals);
			bool VSync = m_window.get_VSync();
			if (ImGui::Checkbox("VSync", &VSync))
				m_window.set_VSync(VSync);

			int framerate_cap = m_window.m_framerate_cap;
			if (ImGui::InputInt("Framerate cap (0 = unlimited)", &framerate_cap))
			{
				if (framerate_cap < 0 || framerate_cap > std::numeric_limits<decltype(m_window.m_framerate_cap)>::max())
					framerate_cap = 0;
				m_window.m_framerate_cap = static_cast<uint16_t>(framerate_cap);
			}

			ImGui::SeparatorText("Renderer");
			m_openGL_renderer.draw_UI();

			if (ImGui::Button("Reset"))
			{
				debug_options.m_show_light_positions = false;
				debug_options.m_show_mesh_normals    = false;
				m_window.set_VSync(true);
				m_window.m_framerate_cap = 0;
				m_openGL_renderer.reset_debug_options();
			}
		}
		ImGui::End();
	}
	void Editor::draw_physics_debug_pane()
	{
		auto& debug_options = OpenGL::DebugRenderer::m_debug_options;

		if (ImGui::Begin("Physics", &m_panes_to_display.Physics_Debug))
		{
			{// GJK visualiser
				if (m_selected_entities.size() != 2)
					ImGui::BeginDisabled();

				ImGui::Checkbox("Toggle GJK debug", &m_debug_GJK);

				if (m_selected_entities.size() != 2)
				{
					ImGui::SameLine();
					ImGui::TextColored(Platform::Core::s_theme.error_text, "Select 2 entities to debug GJK");
					ImGui::EndDisabled();
				}
			}

			ImGui::Checkbox("Show orientations", &debug_options.m_show_orientations);
			ImGui::Checkbox("Show bounding box", &debug_options.m_show_bounding_box);
			ImGui::Checkbox("Fill bounding box", &debug_options.m_fill_bounding_box);

			if (!debug_options.m_show_bounding_box) ImGui::BeginDisabled();
			ImGui::ColorEdit3("Bounding box colour",          &debug_options.m_bounding_box_colour[0]);
			ImGui::ColorEdit3("Bounding box collided colour", &debug_options.m_bounding_box_collided_colour[0]);
			if (!debug_options.m_show_bounding_box) ImGui::EndDisabled();

			ImGui::Slider("Position offset factor", debug_options.m_position_offset_factor, -10.f, 10.f);
			ImGui::Slider("Position offset units",  debug_options.m_position_offset_units,  -10.f, 10.f);

			if (ImGui::Button("Reset"))
			{
				debug_options.m_show_orientations            = false;
				debug_options.m_show_bounding_box            = false;
				debug_options.m_fill_bounding_box            = false;
				debug_options.m_bounding_box_colour          = glm::vec3(0.f, 1.f, 0.f);
				debug_options.m_bounding_box_collided_colour = glm::vec3(1.f, 0.f, 0.f);
				debug_options.m_position_offset_factor       = 1.f;
				debug_options.m_position_offset_units        = 0.f;
			}
		}
		ImGui::End();

		{// Regardless of the debug window being displayed, we want to display or do certain things related to the options.
			if (m_debug_GJK && m_debug_GJK_entity_1 && m_debug_GJK_entity_2)
				draw_GJK_debugger(*m_debug_GJK_entity_1, *m_debug_GJK_entity_2, m_scene_system.get_current_scene(), m_debug_GJK_step);
		}
	}
	void Editor::draw_pie_chart(const std::vector<PieSlice>& slices, const std::function<void()>& on_inner_circle_click, const std::function<void()>& on_inner_circle_hover, bool draw_border)
	{
		constexpr int num_segments = 64; // Number of segments for the pie chart

		if (slices.empty())
			return;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 window_size    = ImGui::GetContentRegionAvail();
		ImU32 highlight_color = IM_COL32(102, 170, 255, 255);

		// Pie chart settings
		float buffer       = 0.05f; // Buffer around the pie chart
		float radius       = std::min(window_size.x, window_size.y) * (0.5f - buffer);
		float inner_radius = radius * 0.03f;

		// Center the chart in the available space
		ImVec2 center = ImGui::GetCursorScreenPos();
		center.x += window_size.x * 0.5f;
		center.y += radius + ImGui::GetStyle().ItemSpacing.y;

		// Calculate distance from mouse to center of circle
		ImVec2 mouse_pos = ImGui::GetMousePos();
		float distance_sq = (mouse_pos.x - center.x) * (mouse_pos.x - center.x) +
		(mouse_pos.y - center.y) * (mouse_pos.y - center.y);
		bool inner_circle_hovering = distance_sq <= inner_radius * inner_radius;

		ImGui::Dummy(ImVec2(0, radius * 2 + ImGui::GetStyle().ItemSpacing.y * 2)); // Add some space for the pie chart

		if (slices.size() == 1) // Special case for a single slice to use AddCircleFilled
		{
			const auto& slice = slices[0];
			bool slice_hovered = !inner_circle_hovering && distance_sq <= radius * radius;

			draw_list->AddCircleFilled(center, radius, slice_hovered ? highlight_color : IM_COL32(slice.colour.r, slice.colour.g, slice.colour.b, slice.colour.a), num_segments);

			if (draw_border)
				draw_list->AddCircle(center, radius, IM_COL32(0, 0, 0, 255), num_segments, 1.5f);

			if (slice_hovered)
			{
				if (slice.on_click && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					slice.on_click();
				else if (slice.on_right_click && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					slice.on_right_click();
				else if (slice.on_hover)
					slice.on_hover();
			}

			ImVec2 text_size = ImGui::CalcTextSize(slice.label.c_str());
			ImVec2 label_pos = ImVec2(
				center.x - text_size.x * 0.5f,
				((center.y - inner_radius * 2.f) - text_size.y * 0.5f)
			);
			draw_list->AddText(label_pos, IM_COL32(255, 255, 255, 255), slice.label.c_str());
		}
		else
		{
			float angle = 0.0f;
			for (size_t slice_index = 0; slice_index < slices.size(); ++slice_index)
			{
				const auto& slice = slices[slice_index];
				float next_angle = angle + slice.percentage * 2.0f * static_cast<float>(std::numbers::pi);
				float mid_angle = (angle + next_angle) * 0.5f;
						// Calculate the angle of the mouse position relative to the center
				float mouse_angle = atan2f(mouse_pos.y - center.y, mouse_pos.x - center.x);
				if (mouse_angle < 0.0f) mouse_angle += 2.0f * static_cast<float>(std::numbers::pi); // Convert to 0-2π range (atan2 returns -π to π)

				bool slice_hovered = !inner_circle_hovering && distance_sq <= radius * radius &&
				                     mouse_angle >= angle && mouse_angle <= next_angle;

				draw_list->PathClear();
				draw_list->PathLineTo(center);
				for (int i = 0; i <= num_segments; i++)
				{
					float a = angle + (next_angle - angle) * i / (float)num_segments;
					draw_list->PathLineTo(ImVec2(
						center.x + cosf(a) * radius,
						center.y + sinf(a) * radius
					));
				}
				draw_list->PathFillConvex(slice_hovered ? highlight_color : IM_COL32(slice.colour.r, slice.colour.g, slice.colour.b, slice.colour.a));

				if (slice_hovered)
				{
					if (slice.on_click && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						slice.on_click();
					else if (slice.on_right_click && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						slice.on_right_click();
					else if (slice.on_hover)
						slice.on_hover();
				}

				if (draw_border)
				{
					draw_list->PathClear();
					for (int i = 0; i <= num_segments; i++)
					{
						float a = angle + (next_angle - angle) * i / (float)num_segments;
						draw_list->PathLineTo(ImVec2(
							center.x + cosf(a) * radius,
							center.y + sinf(a) * radius
						));
					}
					draw_list->PathStroke(IM_COL32(0, 0, 0, 255), ImDrawFlags_None, 1.5f);
				}

				// Draw slice label (only if slice is big enough)
				if (slice.percentage > 0.05f) // Only show label if slice is > 5%
				{
					ImVec2 text_size = ImGui::CalcTextSize(slice.label.c_str());
					float label_radius = radius * 0.7f; // Position label between center and edge
					ImVec2 label_pos = ImVec2(
						center.x + cosf(mid_angle) * label_radius - text_size.x * 0.5f,
						center.y + sinf(mid_angle) * label_radius - text_size.y * 0.5f
					);
					draw_list->AddText(label_pos, IM_COL32(255, 255, 255, 255), slice.label.c_str());
				}

				angle = next_angle;
			}
		}

		// Add a circle in the middle of the pie chart
		draw_list->AddCircleFilled(center, inner_radius, inner_circle_hovering ? highlight_color : IM_COL32(255, 255, 255, 255), 32);

		// Handle inner circle interaction
		if (inner_circle_hovering)
		{
			if (on_inner_circle_click && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				on_inner_circle_click();
			else if (on_inner_circle_hover)
				on_inner_circle_hover();
		}
	}

	void Editor::draw_performance_pane()
	{
		if (ImGui::Begin("Performance", &m_panes_to_display.Performance))
		{
			ImGui::Text("Draw calls", m_draw_count);
			ImGui::Text("FPS", ImGui::GetIO().Framerate);
			ImGui::Separator();

			std::array<glm::vec4, 9> colours = {
				glm::vec4(150, 0, 0, 255),      // Red
				glm::vec4(0, 150, 0, 255),      // Green
				glm::vec4(0, 0, 255, 255),      // Blue
				glm::vec4(255, 255, 0, 255),    // Yellow
				glm::vec4(200, 165, 0, 255),    // Orange
				glm::vec4(75, 0, 130, 255),     // Indigo
				glm::vec4(238, 130, 238, 255),  // Violet
				glm::vec4(0, 255, 255, 255),    // Cyan
				glm::vec4(255, 192, 203, 255),  // Pink
			};

			const auto& perf_tree = Utility::ScopedPerformanceBench::s_performance_benchmarks;
			const Utility::PerformanceTree::Node* pie_chart_node = pie_chart_node_index ? &perf_tree[pie_chart_node_index.value()] : nullptr;
			std::vector<size_t> pie_nodes;

			if (pie_chart_node_index)
			{
				ImGui::Text_Manual("Current layer: Root:%s", pie_chart_node->name.c_str());
				pie_nodes = perf_tree[*pie_chart_node_index].children;
			}
			else
			{
				ImGui::Text("Current layer: Root");
				pie_nodes = perf_tree.get_root_nodes();
			}

			if (pie_nodes.empty())
				ImGui::Text("No performance data available for this layer.");
			else
			{
				// Check if the parent node is valid before calling!
				auto UpLayerFunc = [this, &pie_chart_node]() { pie_chart_node_index = pie_chart_node->parent; };

				// Calculate total duration for the current layer
				Utility::PerformanceTree::Duration total_duration_ms = Utility::PerformanceTree::Duration::zero();
				for (const auto& node_index : pie_nodes)
				{
					const auto& node = perf_tree[node_index];
					if (node.average_duration.count() > 0)
						total_duration_ms += node.average_duration;
				}

				std::vector<PieSlice> slices;
				slices.reserve(pie_nodes.size());
				for (size_t i = 0; i < pie_nodes.size(); ++i)
				{
					const size_t node_index = pie_nodes[i];
					const auto& slice_node = perf_tree[node_index];
					float percentage = slice_node.average_duration / total_duration_ms;

					PieSlice slice;
					slice.percentage = percentage;
					slice.colour     = colours[i % colours.size()];
					slice.label      = slice_node.stem;
					slice.on_hover   = [slice_node, percentage]()
					{
						ImGui::SetTooltip("%s (%.1f%%)\nAvg:%.3fms\nMax:%.3fms\nMin:%.3fms",
							slice_node.stem.c_str(),
							percentage * 100.f,
							std::chrono::duration<float, std::milli>(slice_node.average_duration).count(),
							std::chrono::duration<float, std::milli>(slice_node.max_duration).count(),
							std::chrono::duration<float, std::milli>(slice_node.min_duration).count());
					};
					if (pie_chart_node)               slice.on_right_click = UpLayerFunc;
					if (!slice_node.children.empty()) slice.on_click       = [this, &slice_node, node_index]() { pie_chart_node_index = node_index; };

					slices.push_back(slice);
				}

				draw_pie_chart(slices, UpLayerFunc, [this](){ if (pie_chart_node_index) ImGui::SetTooltip("Click to move up a layer"); });
			}
		}
		ImGui::End();
	}

	void Editor::entity_creation_popup()
	{
		// ImGui API requires that the OpenPopup is called outside of the BeginPopup context.
		if (m_panes_to_display.add_entity_popup)
		{// On right click, open the entity creation popup
			ImGui::OpenPopup("Create entity");
			m_panes_to_display.add_entity_popup = false;
		}

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), ImGuiCond_Appearing);
		if (ImGui::BeginPopup("Create entity"))
		{
			ASSERT(m_cursor_intersection.has_value(), "Cursor intersection should have a value when the entity creation popup is open.");

			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::BeginMenu("Shape"))
				{
					if (ImGui::Button("Cube"))
					{
						m_scene_system.get_current_scene_entities().add_entity(
							Component::Label{"Cube"},
							Component::RigidBody{},
							Component::Transform{*m_cursor_intersection},
							Component::Mesh{m_asset_manager.m_cube},
							Component::Collider{});
					}
					else if (ImGui::Button("Terrain"))
					{
						m_scene_system.get_current_scene_entities().add_entity(
							Component::Label{"Terrain"},
							Component::Transform{*m_cursor_intersection},
							Component::Terrain{5.f});
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Light"))
				{
					if (ImGui::Button("Point light"))
					{
						m_scene_system.get_current_scene_entities().add_entity(
							Component::Label{"Light"},
							Component::Transform{*m_cursor_intersection},
							Component::PointLight{*m_cursor_intersection});
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Particle"))
				{
					if (ImGui::Button("Smoke"))
					{
						auto emitter = Component::ParticleEmitter{m_asset_manager.get_texture(Config::Texture_Directory / "smoke.png")};
						emitter.emit_position_min  = *m_cursor_intersection;
						emitter.emit_position_max  = *m_cursor_intersection;
						emitter.start_colour       = glm::vec4(1.f);
						emitter.end_colour         = glm::vec4(0.f);
						emitter.start_size         = 0.15f;
						emitter.end_size           = 5.f;
						emitter.spawn_per_second   = 50.f;
						emitter.lifetime_min       = DeltaTime{10.f};
						emitter.lifetime_max       = DeltaTime{20.f};
						emitter.max_particle_count = 65'000;
						emitter.acceleration       = glm::vec3(0.f, -0.05f, 0.f);

						m_scene_system.get_current_scene_entities().add_entity(
							Component::Label{"Particle Emitter"},
							Component::Transform{*m_cursor_intersection},
							std::move(emitter));
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem(m_openGL_renderer.m_draw_grid ? "Hide grid" : "Show grid"))
				m_openGL_renderer.m_draw_grid = !m_openGL_renderer.m_draw_grid;
			if (ImGui::MenuItem(m_openGL_renderer.m_draw_axes ? "Hide axis" : "Show axis"))
				m_openGL_renderer.m_draw_axes = !m_openGL_renderer.m_draw_axes;
			ImGui::EndPopup();
		}
	}

	void Editor::log(const std::string& p_message)
	{
		m_console.add_log({p_message, Platform::Core::s_theme.general_text});
	}
	void Editor::log_warning(const std::string& p_message)
	{
		m_console.add_log({p_message, Platform::Core::s_theme.warning_text});
	}
	void Editor::log_error(const std::string& p_message)
	{
		m_console.add_log({p_message, Platform::Core::s_theme.error_text});
	}
	void Editor::initialiseStyling()
	{
		Platform::Core::s_theme.dark_mode ? ImGui::StyleColorsDark() : ImGui::StyleColorsLight();

		// Round out the UI and make more compact
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding            = ImVec2(4.f, 2.f);
		style.FramePadding             = ImVec2(4.f, 2.f);
		style.CellPadding              = ImVec2(4.f, 0.f);
		style.ItemSpacing              = ImVec2(4.f, 3.f);
		style.ItemInnerSpacing         = ImVec2(4.f, 2.f);
		style.TouchExtraPadding        = ImVec2(0.f, 0.f);
		style.IndentSpacing            = 16.f;
		style.ScrollbarSize            = 10.f;
		style.GrabMinSize              = 10.f;

		style.WindowBorderSize         = 1.f;
		style.ChildBorderSize          = 1.f;
		style.PopupBorderSize          = 1.f;
		style.FrameBorderSize          = 0.f;
		style.TabBorderSize            = 0.f;

		style.WindowRounding           = 4.f;
		style.ChildRounding            = 4.f;
		style.FrameRounding            = 4.f;
		style.PopupRounding            = 4.f;
		style.ScrollbarRounding        = 4.f;
		style.GrabRounding             = 4.f;
		style.LogSliderDeadzone        = 4.f;
		style.TabRounding              = 4.f;

		style.WindowTitleAlign         = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ColorButtonPosition      = ImGuiDir_Right;
		style.ButtonTextAlign          = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign      = ImVec2(0.5f, 0.5f);
		style.DisplaySafeAreaPadding   = ImVec2(0.f, 0.f);

		//auto theme_grey = ImVec4(0.174f, 0.174f, 0.174f, 1.000f);
		//style.Colors[ImGuiCol_MenuBarBg] = theme_grey;
	}
	void Editor::PlayerInfoPane::draw(Editor& p_editor)
	{
		const Component::Transform* player_transform      = nullptr;
		const Component::FirstPersonCamera* player_camera = nullptr;

		auto& scene = p_editor.m_scene_system.get_current_scene_entities();
		scene.foreach([&](const Component::FirstPersonCamera& p_camera, const Component::Transform& p_transform)
		{
			if (p_camera.m_primary)
			{
				player_transform = &p_transform;
				player_camera    = &p_camera;
				return;
			}
		});

		if (open)
		{
			if (ImGui::Begin("Player Info", &open))
			{
				ImGui::Text_Manual("Position: %.0f, %.0f, %.0f", player_transform->m_position.x, player_transform->m_position.y, player_transform->m_position.z);
				ImGui::Text_Manual("Forward:  %.0f, %.0f, %.0f", player_camera->forward().x, player_camera->forward().y, player_camera->forward().z);
				ImGui::Text_Manual("Right:    %.0f, %.0f, %.0f", player_camera->right().x,   player_camera->right().y,   player_camera->right().z);
				ImGui::Text_Manual("Up:       %.0f, %.0f, %.0f", player_camera->up().x,      player_camera->up().y,      player_camera->up().z);
				ImGui::Text_Manual("View distance: %.0f", player_camera->get_maximum_view_distance(p_editor.m_viewport_pane.aspect_ratio()));
				ImGui::Text_Manual("Open %s", open ? "true" : "false");

				ImGui::SeparatorText("Scene debug");
				ImGui::Checkbox("Show player position", &render_player_position);
				if (ImGui::Button("Focus camera on player"))
					p_editor.m_viewport_pane.m_camera.look_at(player_transform->m_position);
				ImGui::Checkbox("Show player view distance", &render_player_view_distance);
				ImGui::Checkbox("Show player frustrum", &render_player_frustrum);
			}
			ImGui::End();
		}

		if (player_transform)
		{
			auto& pos = player_transform->m_position;

			if (render_player_position)
			{
				float camera_distance = glm::distance(pos, p_editor.m_viewport_pane.m_camera.position());
				OpenGL::DebugRenderer::add(Geometry::Sphere{pos, camera_distance * 0.01f}, glm::vec4{1.f});
			}
			if (render_player_view_distance && player_camera)
			{
				auto player_view_distance = player_camera->get_maximum_view_distance(p_editor.m_viewport_pane.aspect_ratio());
				OpenGL::DebugRenderer::add(Geometry::Sphere{pos, player_view_distance}, glm::vec4{1.f, 1.f, 0.f, 0.25f}, 3);
			}
			if (render_player_frustrum)
			{
				OpenGL::DebugRenderer::add(player_camera->frustrum(p_editor.m_viewport_pane.aspect_ratio(), pos));
				auto camera_view_dist = player_camera->get_maximum_view_distance(p_editor.m_viewport_pane.aspect_ratio());
				OpenGL::DebugRenderer::add(Geometry::LineSegment(pos, pos + player_camera->forward() * camera_view_dist), glm::vec4(0.f, 0.f, 1.f, 1.f));
				OpenGL::DebugRenderer::add(Geometry::LineSegment(pos, pos - player_camera->forward() * camera_view_dist), glm::vec4(0.f, 0.f, 1.f, 1.f));
				OpenGL::DebugRenderer::add(Geometry::LineSegment(pos, pos + player_camera->right()   * camera_view_dist), glm::vec4(1.f, 0.f, 0.f, 1.f));
				OpenGL::DebugRenderer::add(Geometry::LineSegment(pos, pos - player_camera->right()   * camera_view_dist), glm::vec4(1.f, 0.f, 0.f, 1.f));
			}
		}
	}
	Editor::ViewportPane::ViewportPane()
		: open{true}
		, m_cursor_hovered{false}
		, m_focused{false}
		, m_cursor_pos_content{-1.f}
		, m_camera{}
		, m_FBO{{800, 600}, true, true, false}
	{
		// TODO: Use the current scene bounds (not initialised at this point)
		m_camera.set_orbit_point(glm::vec3(50.f, 0.f, 50.f));
		m_camera.set_orbit_distance(75.f);
		m_camera.set_view_direction(glm::normalize(glm::vec3(0.f, -1.f, 0.f)));
	}
	void Editor::ViewportPane::draw(const DeltaTime& p_delta_time, OpenGL::OpenGLRenderer& p_renderer)
	{
		if (open)
		{
			ImGui::SetNextWindowBgAlpha(1.f); // Opaque background for the viewport
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			if (ImGui::Begin("Viewport", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				// Origin and size of the actual content area (excludes tab bar/title/menu/borders/padding)
				ImVec2 content_pos  = ImGui::GetCursorScreenPos();
				ImVec2 content_size = ImGui::GetContentRegionAvail();
				ImVec2 mouse_pos    = ImGui::GetMousePos();

				m_cursor_pos_content = glm::vec2(mouse_pos.x - content_pos.x, mouse_pos.y - content_pos.y);
				m_cursor_hovered     = m_cursor_pos_content.x >= 0.f && m_cursor_pos_content.y >= 0.f &&
				m_cursor_pos_content.x <= content_size.x && m_cursor_pos_content.y <= content_size.y;
				m_focused            = ImGui::IsWindowFocused(ImGuiFocusedFlags_None);

				m_FBO.resize({content_size.x, content_size.y});
				p_renderer.draw(p_delta_time, m_FBO);
				ImGui::Image(reinterpret_cast<ImTextureID>(reinterpret_cast<void*>(static_cast<std::intptr_t>(m_FBO.color_attachment().handle()))), content_size, ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));

			}
			ImGui::PopStyleVar(2);
			ImGui::End();

		}
	}
} // namespace UI