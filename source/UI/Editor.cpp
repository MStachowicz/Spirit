#include "Editor.hpp"
#include "Visualisers.hpp"

#include "ECS/Storage.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/Collider.hpp"
#include "Component/Label.hpp"
#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Terrain.hpp"
#include "Component/Texture.hpp"
#include "Component/Transform.hpp"
#include "System/CollisionSystem.hpp"
#include "System/MeshSystem.hpp"
#include "System/SceneSystem.hpp"
#include "System/TextureSystem.hpp"

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
	Editor::Editor(Platform::Input& p_input, Platform::Window& p_window, System::TextureSystem& p_texture_system, System::MeshSystem& p_mesh_system, System::SceneSystem& p_scene_system, System::CollisionSystem& p_collision_system, OpenGL::OpenGLRenderer& p_openGL_renderer)
		: m_input{p_input}
		, m_window{p_window}
		, m_texture_system{p_texture_system}
		, m_mesh_system{p_mesh_system}
		, m_scene_system{p_scene_system}
		, m_collision_system{p_collision_system}
		, m_openGL_renderer{p_openGL_renderer}
		, m_state{State::Editing}
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
			case Platform::Key::Escape:
			{
				if (p_action == Platform::Action::Release)
					m_window.request_close();
				break;
			}
			case Platform::Key::Space:
			{
				// Toggle playing and editing mode
				if (p_action == Platform::Action::Release)
					set_state(m_state == State::Editing ? State::Playing : State::Editing);

				break;
			}
			case Platform::Key::F11: m_window.toggle_fullscreen(); break;
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

		if (p_new_state == State::Editing)
		{
			m_input.set_cursor_mode(Platform::CursorMode::Normal);
			m_window.m_show_menu_bar = true;
		}
		else
		{
			m_input.set_cursor_mode(Platform::CursorMode::Captured);
			m_window.m_show_menu_bar = false;
		}

		m_state = p_new_state;
	}
	Component::ViewInformation* Editor::get_editor_view_info()
	{
		if (m_state == State::Editing)
		{
			m_view_info = m_camera.view_information(m_window.aspect_ratio());
			return &m_view_info;
		}
		else
			return nullptr;
	}

	void Editor::draw(const DeltaTime& p_duration_since_last_draw)
	{
		if (m_state != State::Editing)
			return;

		m_duration_between_draws.push_back(p_duration_since_last_draw);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Entity hierarchy", NULL, &m_windows_to_display.Entity);
				ImGui::MenuItem("Console",          NULL, &m_windows_to_display.Console);

				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("Debug options", NULL, &m_windows_to_display.Debug);
					ImGui::MenuItem("FPS Timer",     NULL, &m_windows_to_display.FPSTimer);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("ImGui"))
				{
					ImGui::MenuItem("Demo",             NULL, &m_windows_to_display.ImGuiDemo);
					ImGui::MenuItem("Metrics/Debugger", NULL, &m_windows_to_display.ImGuiMetrics);
					ImGui::MenuItem("Stack",            NULL, &m_windows_to_display.ImGuiStack);
					ImGui::MenuItem("About",            NULL, &m_windows_to_display.ImGuiAbout);
					ImGui::MenuItem("Style Editor",     NULL, &m_windows_to_display.ImGuiStyleEditor);

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (m_windows_to_display.FPSTimer)
			{
				auto fps = get_fps(m_duration_between_draws, m_time_to_average_over);
				std::string fps_str = std::format("FPS: {:.1f}", fps);
				ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(fps_str.c_str()).x - ImGui::GetStyle().ItemSpacing.x) / 2.f);

				glm::vec4 colour;
				if      (fps > 60.f) colour = glm::vec4(0.f, 255.f, 0.f, 255.f);
				else if (fps > 30.f) colour = glm::vec4(255.f, 255.f, 0.f, 255.f);
				else                 colour = glm::vec4(255.f, 0.f, 0.f, 255.f);

				ImGui::TextColored(colour, "%s", fps_str.c_str());
			}
			ImGui::EndMenuBar();
		}
		if (m_windows_to_display.Entity)        draw_entity_tree_window();
		if (m_windows_to_display.Console)       draw_console_window();
		draw_debug_window();
		if (m_windows_to_display.ImGuiDemo)     ImGui::ShowDemoWindow(&m_windows_to_display.ImGuiDemo);
		if (m_windows_to_display.ImGuiMetrics)  ImGui::ShowMetricsWindow(&m_windows_to_display.ImGuiMetrics);
		if (m_windows_to_display.ImGuiStack)    ImGui::ShowStackToolWindow(&m_windows_to_display.ImGuiStack);
		if (m_windows_to_display.ImGuiAbout)    ImGui::ShowAboutWindow(&m_windows_to_display.ImGuiAbout);
		if (m_windows_to_display.ImGuiStyleEditor)
		{
			ImGui::Begin("Dear ImGui Style Editor", &m_windows_to_display.ImGuiStyleEditor);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
		draw_entity_properties();

		entity_creation_popup();

		if (m_state == State::Editing)
			m_camera.draw_UI();

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
					ImGuizmo::Manipulate(
						glm::value_ptr(m_scene_system.get_current_scene_view_info().m_view),
						glm::value_ptr(m_scene_system.get_current_scene_view_info().m_projection),
						ImGuizmo::OPERATION::UNIVERSAL,
						ImGuizmo::LOCAL,
						glm::value_ptr(transform.m_model));

					if (ImGuizmo::IsUsing())
						transform.set_model_matrix(transform.m_model);
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
			scene.get_component<Component::ParticleEmitter>(p_entity).draw_UI(m_texture_system);
		if (scene.has_components<Component::Terrain>(p_entity))
			scene.get_component<Component::Terrain>(p_entity).draw_UI(m_texture_system);
		if (scene.has_components<Component::Mesh>(p_entity))
			scene.get_component<Component::Mesh>(p_entity).draw_UI();
		if (scene.has_components<Component::Texture>(p_entity))
			scene.get_component<Component::Texture>(p_entity).draw_UI(m_texture_system);

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
		if (ImGui::Begin("Entities", &m_windows_to_display.Entity))
		{
			auto& scene = m_scene_system.get_current_scene_entities();
			scene.foreach([&](ECS::Entity& p_entity)
			{
				std::string title = "Entity " + std::to_string(p_entity.ID);
				if (scene.has_components<Component::Label>(p_entity))
				{
					auto label = scene.get_component<Component::Label&>(p_entity);
					title = label.mName;
				}

				if (ImGui::TreeNode(title.c_str()))
				{
					draw_entity_UI(p_entity);

					ImGui::Separator();
					ImGui::TreePop();
				}
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
				title = label.mName;
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
	void Editor::draw_console_window()
	{
		m_console.draw("Console", &m_windows_to_display.Console);
	}
	void Editor::draw_debug_window()
	{
		if (m_windows_to_display.Debug)
		{
			if (ImGui::Begin("Debug options", &m_windows_to_display.Debug))
			{
				auto& debug_options = OpenGL::DebugRenderer::m_debug_options;
				{ ImGui::SeparatorText("Graphics");
					ImGui::Text("Window size", m_window.size());
					ImGui::Text("Aspect ratio", m_window.aspect_ratio());
					bool VSync = m_window.get_VSync();
					ImGui::Text("View Position", m_scene_system.get_current_scene_view_info().m_view_position);
					ImGui::Separator();
					ImGui::Checkbox("Show light positions", &debug_options.m_show_light_positions);
					ImGui::Checkbox("Visualise normals", &debug_options.m_show_mesh_normals);
					if (ImGui::Checkbox("VSync", &VSync))
						m_window.set_VSync(VSync);
				}

				{ ImGui::SeparatorText("Post Processing");
					ImGui::Checkbox("Invert", &m_openGL_renderer.m_post_processing_options.mInvertColours);
					ImGui::Checkbox("Grayscale", &m_openGL_renderer.m_post_processing_options.mGrayScale);
					ImGui::Checkbox("Sharpen", &m_openGL_renderer.m_post_processing_options.mSharpen);
					ImGui::Checkbox("Blur", &m_openGL_renderer.m_post_processing_options.mBlur);
					ImGui::Checkbox("Edge detection", &m_openGL_renderer.m_post_processing_options.mEdgeDetection);

					const bool isPostProcessingOn = m_openGL_renderer.m_post_processing_options.mInvertColours
						|| m_openGL_renderer.m_post_processing_options.mGrayScale || m_openGL_renderer.m_post_processing_options.mSharpen
						|| m_openGL_renderer.m_post_processing_options.mBlur      || m_openGL_renderer.m_post_processing_options.mEdgeDetection;

					if (!isPostProcessingOn) ImGui::BeginDisabled();
						ImGui::SliderFloat("Kernel offset", &m_openGL_renderer.m_post_processing_options.mKernelOffset, -1.f, 1.f);
					if (!isPostProcessingOn) ImGui::EndDisabled();
				}

				{ImGui::SeparatorText("Physics");
					{// GJK visualiser
						if (m_selected_entities.size() != 2)
							ImGui::BeginDisabled();

						ImGui::Checkbox("Toggle GJK debug", &m_debug_GJK);

						if (m_selected_entities.size() != 2)
						{
							ImGui::SameLine();
							ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Select 2 entities to debug GJK");
							ImGui::EndDisabled();
						}
					}

					ImGui::Checkbox("Show orientations",        &debug_options.m_show_orientations);
					ImGui::Checkbox("Show bounding box",        &debug_options.m_show_bounding_box);
					ImGui::Checkbox("Fill bounding box",        &debug_options.m_fill_bounding_box);
					ImGui::Checkbox("Show collision shapes",    &debug_options.m_show_collision_shapes);

					bool showing_collision_shapes = debug_options.m_show_bounding_box || debug_options.m_show_collision_shapes;
					if (!showing_collision_shapes) ImGui::BeginDisabled();
					ImGui::ColorEdit3("Bounding box colour",          &debug_options.m_bounding_box_colour[0]);
					ImGui::ColorEdit3("Bounding box collided colour", &debug_options.m_bounding_box_collided_colour[0]);
					if (!showing_collision_shapes) ImGui::EndDisabled();

					ImGui::Slider("Position offset factor",            debug_options.m_position_offset_factor, -10.f, 10.f);
					ImGui::Slider("Position offset units",             debug_options.m_position_offset_units, -10.f, 10.f);
				}

				if (ImGui::Button("Reset"))
					debug_options = OpenGL::DebugRenderer::DebugOptions();
			}
			ImGui::End();
		}

		{// Regardless of the debug window being displayed, we want to display or do certain things related to the options.
			if (m_debug_GJK && m_debug_GJK_entity_1 && m_debug_GJK_entity_2)
				draw_GJK_debugger(*m_debug_GJK_entity_1, *m_debug_GJK_entity_2, m_scene_system.get_current_scene(), m_debug_GJK_step);
		}
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
							Component::Mesh{m_mesh_system.m_cube},
							Component::Collider{});
					}
					else if (ImGui::Button("Light"))
					{
						m_scene_system.get_current_scene_entities().add_entity(
							Component::Label{"Light"},
							Component::Transform{*m_cursor_intersection},
							Component::PointLight{*m_cursor_intersection});
					}
					else if (ImGui::Button("Terrain"))
					{
						m_scene_system.get_current_scene_entities().add_entity(
							Component::Label{"Terrain"},
							Component::Terrain{*m_cursor_intersection, 10, 10});
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	void Editor::log(const std::string& p_message)
	{
		m_console.add_log({p_message});
	}
	void Editor::log_warning(const std::string& p_message)
	{
		m_console.add_log({p_message, glm::vec3(1.f, 1.f, 0.f)});
	}
	void Editor::log_error(const std::string& p_message)
	{
		m_console.add_log({p_message, glm::vec3(1.f, 0.f, 0.f)});
	}
	void Editor::initialiseStyling()
	{
		ImGui::StyleColorsDark();

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

		auto theme_grey = ImVec4(0.174f, 0.174f, 0.174f, 1.000f);
		style.Colors[ImGuiCol_MenuBarBg] = theme_grey;
	}
} // namespace UI