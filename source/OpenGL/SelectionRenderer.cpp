#include "SelectionRenderer.hpp"
#include "DrawCall.hpp"

#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"

#include "Utility/MeshBuilder.hpp"

#include "imgui.h"

namespace OpenGL
{
	static Data::Mesh make_screen_quad()
	{
		auto mb = Utility::MeshBuilder<Data::TextureVertex, OpenGL::PrimitiveMode::Triangles>{};
		mb.add_quad(glm::vec3(-1.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
		return mb.get_mesh();
	}

	SelectionRenderer::SelectionRenderer() noexcept
		: m_mask_shader{"selectionMask"}
		, m_edge_shader{"selectionEdge"}
		, m_mask_FBO{}
		, m_screen_quad{make_screen_quad()}
		, m_outline_pixels{2}
		, m_outline_colour{1.f, 0.f, 0.f, 1.f}
	{}

	void SelectionRenderer::selection_pass(std::span<const ECS::Entity> p_selected_entities, ECS::Storage& p_entities, const Buffer& p_view_properties, FBO& p_target_FBO)
	{
		if (p_selected_entities.empty())
			return;

		// Lazily create or resize the mask FBO to match the target resolution.
		const auto resolution = p_target_FBO.resolution();
		if (!m_mask_FBO || m_mask_FBO->resolution() != resolution)
			m_mask_FBO.emplace(resolution, true, true, false); // Colour + depth, no stencil.

		const glm::vec2 texel_size = glm::vec2(1.f / static_cast<float>(resolution.x), 1.f / static_cast<float>(resolution.y));

		// Per-entity loop ensures overlapping selected entities each get independent outlines
		// rather than a shared mask that loses interior edges between them.
		for (const auto& entity : p_selected_entities)
		{
			if (!p_entities.has_components<Component::Transform, Component::Mesh>(entity))
				continue;

			auto& transform = p_entities.get_component<Component::Transform>(entity);
			auto& mesh      = p_entities.get_component<Component::Mesh>(entity);

			if (!mesh.m_mesh)
				continue;

			// Pass 1: Always depth test keeps the mask hole-free — depth-testing against scene geometry
			// would punch holes where other objects occlude it, and the dilation shader would misread
			// those hole boundaries as edges, generating false outlines around foreground objects.
			m_mask_FBO->set_clear_colour(glm::vec4(0.f, 0.f, 0.f, 0.f));
			m_mask_FBO->clear();
			{
				DrawCall dc;
				dc.m_depth_test_enabled    = true;
				dc.m_depth_test_type       = DepthTestType::Always;
				dc.m_write_to_depth_buffer = true;
				dc.set_uniform("model", transform.get_model());
				dc.set_UBO("ViewProperties", p_view_properties);
				dc.submit(m_mask_shader, mesh.m_mesh->get_VAO(), *m_mask_FBO);
			}

			// Pass 2: mask_depth and scene_depth let the shader reject outline pixels that are
			// occluded (entity behind geometry at the neighbour) or overdrawn (closer geometry at the centre).
			{
				DrawCall dc;
				dc.m_depth_test_enabled    = false;
				dc.m_write_to_depth_buffer = false;
				dc.m_cull_face_enabled     = false;
				dc.set_texture("mask", m_mask_FBO->color_attachment());
				dc.set_texture("mask_depth", m_mask_FBO->depth_attachment());
				dc.set_texture("scene_depth", p_target_FBO.depth_attachment());
				dc.set_uniform("colour", m_outline_colour);
				dc.set_uniform("texel_size", texel_size);
				dc.set_uniform("radius", m_outline_pixels);
				dc.submit(m_edge_shader, m_screen_quad.get_VAO(), p_target_FBO);
			}
		}
	}

	void SelectionRenderer::draw_UI()
	{
		ImGui::SeparatorText("Selection outline");
		ImGui::ColorEdit4("Outline colour",  &m_outline_colour[0]);
		ImGui::SliderInt("Outline pixels", &m_outline_pixels, 1, 8);
	}

	void SelectionRenderer::reload_shaders()
	{
		m_mask_shader.reload();
		m_edge_shader.reload();
	}
} // namespace OpenGL
