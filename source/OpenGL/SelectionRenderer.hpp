#pragma once

#include "Shader.hpp"
#include "Types.hpp"

#include "Component/Mesh.hpp"

#include "glm/vec4.hpp"

#include <optional>
#include <span>

namespace ECS
{
	class Entity;
	class Storage;
}
namespace OpenGL
{
	class Buffer;

	// Renders screen-space selection outlines around selected entities using a per-entity
	// two-pass silhouette dilation approach with depth-based occlusion culling.
	class SelectionRenderer
	{
		Shader m_mask_shader;
		Shader m_edge_shader;
		std::optional<FBO> m_mask_FBO;
		Data::Mesh m_screen_quad;

	public:
		int m_outline_pixels;
		glm::vec4 m_outline_colour;

		SelectionRenderer() noexcept;

		// Render screen-space outlines around the selected entities into the target FBO.
		// O(E * P^2) where E = selected entity count, P = outline pixel radius.
		// Each entity costs two full-screen passes; the edge shader samples a (2*radius+1)^2 neighbourhood per fragment.
		void selection_pass(std::span<const ECS::Entity> p_selected_entities, ECS::Storage& p_entities, const Buffer& p_view_properties, FBO& p_target_FBO);

		void draw_UI();
		void reload_shaders();
	};
}