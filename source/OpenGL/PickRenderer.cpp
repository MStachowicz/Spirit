
#include "PickRenderer.hpp"
#include "DrawCall.hpp"
#include "GLState.hpp"

#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/Terrain.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"

#include "Utility/MeshBuilder.hpp"
#include "Utility/Screenshot.hpp"

#include "glad/glad.h"
#include "imgui.h"

#include "glm/gtc/matrix_transform.hpp"

#include <limits>
#include <vector>

namespace OpenGL
{
	static constexpr GLuint no_entity_sentinel = 0u;

	static Data::Mesh make_screen_quad()
	{
		auto mb = Utility::MeshBuilder<Data::TextureVertex, PrimitiveMode::Triangles>{};
		mb.add_quad(glm::vec3(-1.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
		return mb.get_mesh();
	}

	PickRenderer::PickRenderer() noexcept
		: m_pick_shader{"entityPick"}
		, m_debug_shader{"entityPickDebug"}
		, m_pick_FBO{}
		, m_debug_FBO{}
		, m_screen_quad{make_screen_quad()}
		, m_point_light_mesh{}
	{
		auto mb = Utility::MeshBuilder<Data::PositionVertex, PrimitiveMode::Triangles>{};
		mb.add_icosphere(glm::vec3(0.f), 1.f, 1);
		m_point_light_mesh = mb.get_mesh();
	}

	std::optional<ECS::Entity> PickRenderer::pick(
		const glm::vec2& p_cursor_pos,
		const glm::uvec2& p_viewport_resolution,
		ECS::Storage& p_entities,
		const Buffer& p_view_properties,
		bool p_draw_terrain,
		float p_light_position_scale,
		bool p_show_light_positions)
	{
		if (p_viewport_resolution.x == 0 || p_viewport_resolution.y == 0)
			return std::nullopt;
		if (p_cursor_pos.x < 0.f
			|| p_cursor_pos.y < 0.f
			|| p_cursor_pos.x >= static_cast<float>(p_viewport_resolution.x)
			|| p_cursor_pos.y >= static_cast<float>(p_viewport_resolution.y))
			return std::nullopt;

		// Lazily create or resize the pick FBO to match the viewport resolution.
		if (!m_pick_FBO || m_pick_FBO->resolution() != p_viewport_resolution)
			m_pick_FBO.emplace(p_viewport_resolution, true, true, false, TextureInternalFormat::R32UI);

		// Clear to the no-entity sentinel value.
		m_pick_FBO->set_clear_colour(glm::uvec4{no_entity_sentinel});
		m_pick_FBO->clear();

		std::vector<ECS::Entity> pick_entities;
		pick_entities.reserve(
			p_entities.count_components<Component::Transform, Component::Mesh>()
			+ (p_draw_terrain ? p_entities.count_components<Component::Terrain>() : 0)
			+ (p_show_light_positions ? p_entities.count_components<Component::PointLight>() : 0));

		auto register_pick_entity = [&pick_entities](ECS::Entity p_entity)
		{
			ASSERT_THROW(pick_entities.size() < std::numeric_limits<GLuint>::max(), "Too many entities in the pick pass.");
			pick_entities.push_back(p_entity);
			return static_cast<GLuint>(pick_entities.size());
		};

		// Render all Mesh+Transform entities with a transient pick ID as the colour.
		p_entities.foreach([&](ECS::Entity& p_entity, Component::Transform& p_transform, Component::Mesh& mesh_comp)
		{
			if (!mesh_comp.m_mesh)
				return;

			DrawCall dc;
			dc.m_depth_test_enabled    = true;
			dc.m_write_to_depth_buffer = true;
			dc.m_blending_enabled      = false;
			dc.set_uniform("model", p_transform.get_model());
			dc.set_uniform("entity_id", register_pick_entity(p_entity));
			dc.set_UBO("ViewProperties", p_view_properties);
			dc.submit(m_pick_shader, mesh_comp.m_mesh->get_VAO(), *m_pick_FBO);
		});

		if (p_draw_terrain)
		{
			p_entities.foreach([&](ECS::Entity& p_entity, Component::Terrain& p_terrain)
			{
				if (p_terrain.empty())
					return;

				DrawCall dc;
				dc.m_depth_test_enabled    = true;
				dc.m_write_to_depth_buffer = true;
				dc.m_blending_enabled      = false;
				dc.set_uniform("model", glm::identity<glm::mat4>());
				dc.set_uniform("entity_id", register_pick_entity(p_entity));
				dc.set_UBO("ViewProperties", p_view_properties);
				dc.submit(m_pick_shader, p_terrain.get_VAO(), *m_pick_FBO);
			});
		}

		// Render point light debug spheres individually so they are pickable.
		if (p_show_light_positions && m_point_light_mesh)
		{
			p_entities.foreach([&](ECS::Entity& p_entity, Component::PointLight& p_light)
			{
				glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), p_light.m_position);
				model = glm::scale(model, glm::vec3(p_light_position_scale));

				DrawCall dc;
				dc.m_depth_test_enabled    = true;
				dc.m_write_to_depth_buffer = true;
				dc.m_blending_enabled      = false;
				dc.set_uniform("model", model);
				dc.set_uniform("entity_id", register_pick_entity(p_entity));
				dc.set_UBO("ViewProperties", p_view_properties);
				dc.submit(m_pick_shader, m_point_light_mesh->get_VAO(), *m_pick_FBO);
			});
		}

		// Read back the pixel under the cursor.
		// ImGui cursor is top-left origin; OpenGL FBO is bottom-left origin — flip Y.
		int pixel_x = static_cast<int>(p_cursor_pos.x);
		int pixel_y = static_cast<int>(p_viewport_resolution.y) - 1 - static_cast<int>(p_cursor_pos.y);

		GLuint pixel = no_entity_sentinel;
		State::Get().bind_FBO(m_pick_FBO->m_handle);
		glReadPixels(pixel_x, pixel_y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pixel);
		State::Get().unbind_FBO();

		if (pixel == no_entity_sentinel || pixel > pick_entities.size())
			return std::nullopt;

		return pick_entities[pixel - 1];
	}

	void PickRenderer::reload_shaders()
	{
		m_pick_shader.reload();
		m_debug_shader.reload();
	}

	void PickRenderer::draw_UI()
	{
		ImGui::SeparatorText("Pick Renderer");
		ImGui::Checkbox("Show pick buffer", &m_show_debug_view);

		if (m_show_debug_view && m_pick_FBO)
		{
			const auto resolution = m_pick_FBO->resolution();
			if (resolution.x == 0 || resolution.y == 0)
				return;

			// Lazily create or resize the debug visualisation FBO (RGBA8) to match the pick FBO.
			if (!m_debug_FBO || m_debug_FBO->resolution() != resolution)
				m_debug_FBO.emplace(resolution, true, false, false);

			// Render the pick buffer through the debug shader into the RGBA8 FBO.
			m_debug_FBO->set_clear_colour(glm::vec4(0.f, 0.f, 0.f, 1.f));
			m_debug_FBO->clear();
			{
				DrawCall dc;
				dc.m_depth_test_enabled    = false;
				dc.m_write_to_depth_buffer = false;
				dc.m_cull_face_enabled     = false;
				dc.set_texture("pick_texture", m_pick_FBO->color_attachment());
				dc.submit(m_debug_shader, m_screen_quad.get_VAO(), *m_debug_FBO);
			}

			// Show the debug FBO in an ImGui window.
			ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Pick Buffer Debug", &m_show_debug_view))
			{
				auto content_size = ImGui::GetContentRegionAvail();
				float aspect      = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
				float width       = content_size.x;
				float height      = width / aspect;
				if (height > content_size.y)
				{
					height = content_size.y;
					width  = height * aspect;
				}
				ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(m_debug_FBO->color_attachment().handle())), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
					if (drag.x == 0.f && drag.y == 0.f)
						ImGui::OpenPopup("pick_debug_save");
				}
				if (ImGui::BeginPopup("pick_debug_save"))
				{
					if (ImGui::MenuItem("Save as PNG"))
					{
						auto& dir = Utility::screenshot_directory();
						if (!dir.empty())
						{
							auto pixels = m_debug_FBO->read_pixels();
							if (!pixels.empty())
							{
								const auto resolution = m_debug_FBO->resolution();
								Utility::save_pixels_to_file(
									dir,
									pixels,
									static_cast<int>(resolution.x),
									static_cast<int>(resolution.y),
									m_debug_FBO->channel_count(),
									true,
									"pick_debug");
							}
						}
					}
					ImGui::EndPopup();
				}
			}
			ImGui::End();
		}
	}
} // namespace OpenGL
