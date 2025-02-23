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
#include "Utility/Utility.hpp"

#include "imgui.h"
#include "ImGuizmo.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <format>

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
		, m_camera{}
		, m_view_info{m_camera.view_information(m_window.aspect_ratio())}
		, m_selected_entities{}
		, m_entity_to_draw_info_for{}
		, m_cursor_intersection{}
		, m_console{}
		, m_windows_to_display{}
		, m_dragging{false}
		, m_debug_GJK{false}
		, m_debug_GJK_entity_1{}
		, m_debug_GJK_entity_2{}
		, m_debug_GJK_step{0}
		, m_show_primary_camera_frustrum{false}
		, m_draw_count{0}
		, m_time_to_average_over{std::chrono::seconds(1)}
		, m_duration_between_draws{}
	{
		m_input.m_mouse_button_event.subscribe(this, &Editor::on_mouse_button_event);
		m_input.m_key_event.subscribe(this,          &Editor::on_key_event);
		m_input.m_mouse_move_event.subscribe(this,   &Editor::on_mouse_move_event);
		m_input.m_mouse_scroll_event.subscribe(this, &Editor::on_mouse_scroll_event);

		// TODO: Use the current scene bounds (not initialised at this point)
		m_camera.set_orbit_point(glm::vec3(50.f, 0.f, 50.f));
		m_camera.set_orbit_distance(75.f);
		m_camera.set_view_direction(glm::normalize(glm::vec3(0.f, -1.f, 0.f)));

		set_state(m_state, true); // Set the initial state.
		initialiseStyling();
	}

	void Editor::on_mouse_button_event(Platform::MouseButton p_button, Platform::Action p_action)
	{
		if (m_state != State::Editing)
			return;

		if (!m_input.cursor_over_UI())
		{
			switch (p_button)
			{
				case Platform::MouseButton::Left:
				{
					if (p_action == Platform::Action::Press)
					{
						const auto& view_info = m_scene_system.get_current_scene_view_info();
						auto cursor_ray = Utility::get_cursor_ray(m_input.cursor_position(), m_window.size(), view_info.m_view_position, view_info.m_projection, view_info.m_view);
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
							// const auto& view_info = m_scene_system.get_current_scene_view_info();
							// auto cursor_ray = Utility::get_cursor_ray(m_input.cursor_position(), m_window.size(), view_info.m_view_position, view_info.m_projection, view_info.m_view);

							// m_scene_system.get_current_scene_entities().foreach([&](Component::Terrain& terrain)
							// {
							// 	for (auto& node : terrain.quad_tree)
							// 	{
							// 		if (node.leaf())
							// 		{
							// 			Geometry::AABB bound_3D{glm::vec3(node.bounds.min.x, 0.f, node.bounds.min.y), glm::vec3(node.bounds.max.x, 0.f, node.bounds.max.y)};
							// 			auto intersection = Geometry::get_intersection(bound_3D, cursor_ray);
							// 			if (intersection)
							// 			{
							// 				node.
							// 				terrain.quad_tree.subdivide(node, Component::Terrain::BufferHandle{}, Component::Terrain::BufferHandle{}, Component::Terrain::BufferHandle{}, Component::Terrain::BufferHandle{});
							// 			}
							// 		}
							// 	}
							// });

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
						auto cursor_ray  = Utility::get_cursor_ray(m_input.cursor_position(), m_window.size(), m_view_info.m_view_position, m_view_info.m_projection, m_view_info.m_view);
						auto floor_plane = Geometry::Plane{glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)};
						if ((m_cursor_intersection = Geometry::get_intersection(cursor_ray, floor_plane)))
							m_windows_to_display.add_entity_popup = true;
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
		if (m_input.cursor_over_UI())
			return;

		if (m_state == State::Editing)
		{
			if (m_input.is_mouse_down(Platform::MouseButton::Right))
			{
				m_camera.mouse_look(p_mouse_delta);
				m_view_info = m_camera.view_information(m_window.aspect_ratio());
			}
			else if (m_input.is_mouse_down(Platform::MouseButton::Middle))
			{
				m_camera.pan(p_mouse_delta);
				m_view_info = m_camera.view_information(m_window.aspect_ratio());
			}
		}

		if (!m_dragging && m_input.is_any_mouse_down())
			m_dragging = true;
	}
	void Editor::on_mouse_scroll_event(const glm::vec2 p_mouse_scroll)
	{
		if (m_input.cursor_over_UI())
			return;

		if (m_state == State::Editing)
		{
			m_camera.zoom(p_mouse_scroll.y);
			m_view_info = m_camera.view_information(m_window.aspect_ratio());
		}
	}
	void Editor::on_key_event(Platform::Key p_key, Platform::Action p_action)
	{
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
					m_windows_to_display.Entity = !m_windows_to_display.Entity;
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
					m_camera.toggle_orthographic();
				break;
			}
			case Platform::Key::U:
			{
				if (p_action == Platform::Action::Release)
					m_windows_to_display.Console = !m_windows_to_display.Console;
				break;
			}
			case Platform::Key::Escape:
			{
				if (p_action == Platform::Action::Release)
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
				m_show_primary_camera_frustrum          = false;

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
				m_show_primary_camera_frustrum          = false;
				break;
			}
			case State::CameraTesting:
			{
				m_input.set_cursor_mode(Platform::CursorMode::Captured);
				m_window.m_show_menu_bar                = true;
				m_physics_system.m_bool_apply_kinematic = false;
				m_show_primary_camera_frustrum          = true;
				if (m_scene_before_play && m_state == State::Playing) // Only reset the scene if the user was playing before entering camera testing.
					m_scene_system.set_current_scene(*m_scene_before_play);
				break;
			}
			default: break;
		}

		m_state = p_new_state;
	}
	Component::ViewInformation* Editor::get_editor_view_info()
	{
		if (m_state == State::Editing || m_state == State::CameraTesting)
		{
			m_view_info = m_camera.view_information(m_window.aspect_ratio());
			return &m_view_info;
		}
		else
			return nullptr;
	}

	void Editor::draw(const DeltaTime& p_duration_since_last_draw)
	{
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
				ImGui::MenuItem("Entity tree", NULL, &m_windows_to_display.Entity);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Console",            NULL, &m_windows_to_display.Console);
				ImGui::MenuItem("Asset browser",      NULL, &m_windows_to_display.asset_browser);
				ImGui::Separator();
				ImGui::MenuItem("Camera",             NULL, &m_windows_to_display.editor_camera);
				ImGui::MenuItem("FPS Timer",          NULL, &m_windows_to_display.FPSTimer);
				ImGui::Separator();
				ImGui::MenuItem("Theme",              NULL, &m_windows_to_display.theme_editor);
				ImGui::MenuItem("ImGui style editor", NULL, &m_windows_to_display.ImGuiStyleEditor);

				if (ImGui::BeginMenu("ImGui"))
				{
					ImGui::MenuItem("Demo",  NULL, &m_windows_to_display.ImGuiDemo);
					ImGui::MenuItem("Stack", NULL, &m_windows_to_display.ImGuiStack);
					ImGui::MenuItem("About", NULL, &m_windows_to_display.ImGuiAbout);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug"))
			{
				ImGui::MenuItem("Graphics",               NULL, &m_windows_to_display.Graphics_Debug);
				ImGui::MenuItem("Physics",                NULL, &m_windows_to_display.Physics_Debug);
				ImGui::MenuItem("ImGui Metrics/Debugger", NULL, &m_windows_to_display.ImGuiMetrics);
				ImGui::Separator();
				if (ImGui::MenuItem("Play"))
					set_state(State::Playing);
				if (ImGui::MenuItem("Camera Test"))
					set_state(State::CameraTesting);
				ImGui::EndMenu();
			}
			if (m_windows_to_display.FPSTimer)
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
		if (m_windows_to_display.Entity)         draw_entity_tree_window();
		if (m_windows_to_display.Console)        draw_console_window();
		if (m_windows_to_display.asset_browser)  m_asset_manager.draw_UI(&m_windows_to_display.asset_browser);
		if (m_windows_to_display.Graphics_Debug) draw_graphics_debug_window();
		if (m_windows_to_display.Physics_Debug)  draw_physics_debug_window();
		if (m_windows_to_display.ImGuiDemo)      ImGui::ShowDemoWindow(&m_windows_to_display.ImGuiDemo);
		if (m_windows_to_display.ImGuiMetrics)   ImGui::ShowMetricsWindow(&m_windows_to_display.ImGuiMetrics);
		if (m_windows_to_display.ImGuiStack)     ImGui::ShowStackToolWindow(&m_windows_to_display.ImGuiStack);
		if (m_windows_to_display.ImGuiAbout)     ImGui::ShowAboutWindow(&m_windows_to_display.ImGuiAbout);
		if (m_windows_to_display.theme_editor)   Platform::Core::s_theme.draw_theme_editor_UI();
		if (m_windows_to_display.ImGuiStyleEditor)
		{
			ImGui::Begin("Dear ImGui Style Editor", &m_windows_to_display.ImGuiStyleEditor);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
		if (m_windows_to_display.editor_camera)
		{
			ImGui::Begin("Editor Camera", &m_windows_to_display.editor_camera, ImGuiWindowFlags_AlwaysAutoResize);
			m_camera.draw_UI();
			ImGui::End();
		}

		if (m_show_primary_camera_frustrum)
		{
			auto& scene = m_scene_system.get_current_scene_entities();
			scene.foreach([&](Component::FirstPersonCamera& p_camera, const Component::Transform& p_transform)
			{
				if (!p_camera.m_primary)
					return;

				OpenGL::DebugRenderer::add(p_camera.frustrum(m_window.aspect_ratio(), p_transform.m_position));

				auto camera_view_dist = p_camera.get_maximum_view_distance(m_window.aspect_ratio());
				//OpenGL::DebugRenderer::add(Geometry::Sphere(p_transform.m_position, camera_view_dist), glm::vec4(1.f, 1.f, 0.f, 0.25f), 3);
				OpenGL::DebugRenderer::add(Geometry::LineSegment(p_transform.m_position, p_transform.m_position + p_camera.forward() * camera_view_dist), glm::vec4(0.f, 0.f, 1.f, 1.f));
				OpenGL::DebugRenderer::add(Geometry::LineSegment(p_transform.m_position, p_transform.m_position - p_camera.forward() * camera_view_dist), glm::vec4(0.f, 0.f, 1.f, 1.f));
				OpenGL::DebugRenderer::add(Geometry::LineSegment(p_transform.m_position, p_transform.m_position + p_camera.right() * camera_view_dist), glm::vec4(1.f, 0.f, 0.f, 1.f));
				OpenGL::DebugRenderer::add(Geometry::LineSegment(p_transform.m_position, p_transform.m_position - p_camera.right() * camera_view_dist), glm::vec4(1.f, 0.f, 0.f, 1.f));
			});
		}

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
	void Editor::draw_entity_tree_window()
	{
		ImGui::SetNextWindowDockID(1, ImGuiCond_Once);
		if (ImGui::Begin("Entities", &m_windows_to_display.Entity))
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
		m_windows_to_display.ent_properties = m_entity_to_draw_info_for.has_value();

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

			ImGui::Begin(title.c_str(), &m_windows_to_display.ent_properties);
			draw_entity_UI(ent);
			ImGui::End();

			if (!m_windows_to_display.ent_properties)
				m_entity_to_draw_info_for.reset(); // If the window is closed, reset the entity to draw info for.
		}
	}
	void Editor::draw_graphics_debug_window()
	{
		auto& debug_options = OpenGL::DebugRenderer::m_debug_options;

		if (ImGui::Begin("Graphics", &m_windows_to_display.Graphics_Debug))
		{
			ImGui::Text("Window size",  m_window.size());
			ImGui::Text("Aspect ratio", m_window.aspect_ratio());
			ImGui::Text("Frame count",  m_draw_count);
			ImGui::Separator();
			ImGui::Text("View Position", m_scene_system.get_current_scene_view_info().m_view_position);
			ImGui::Text("View",          m_scene_system.get_current_scene_view_info().m_view);
			ImGui::Text("Proj",          m_scene_system.get_current_scene_view_info().m_projection);
			ImGui::Separator();
			ImGui::Checkbox("Show light positions", &debug_options.m_show_light_positions);
			ImGui::Checkbox("Show camera frustrum", &m_show_primary_camera_frustrum);
			ImGui::Checkbox("Visualise normals",    &debug_options.m_show_mesh_normals);
			bool VSync = m_window.get_VSync();
			if (ImGui::Checkbox("VSync", &VSync))
				m_window.set_VSync(VSync);

			ImGui::SeparatorText("Renderer");
			m_openGL_renderer.draw_UI();

			if (ImGui::Button("Reset"))
			{
				debug_options.m_show_light_positions = false;
				debug_options.m_show_mesh_normals    = false;
				m_window.set_VSync(true);
				m_openGL_renderer.reset_debug_options();
			}
		}
		ImGui::End();
	}
	void Editor::draw_physics_debug_window()
	{
		auto& debug_options = OpenGL::DebugRenderer::m_debug_options;

		if (ImGui::Begin("Physics", &m_windows_to_display.Physics_Debug))
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
	void Editor::draw_console_window()
	{
		m_console.draw("Console", &m_windows_to_display.Console);
	}

	void Editor::entity_creation_popup()
	{
		// ImGui API requires that the OpenPopup is called outside of the BeginPopup context.
		if (m_windows_to_display.add_entity_popup)
		{// On right click, open the entity creation popup
			ImGui::OpenPopup("Create entity");
			m_windows_to_display.add_entity_popup = false;
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
} // namespace UI