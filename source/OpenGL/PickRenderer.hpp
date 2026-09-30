#pragma once

#include "Shader.hpp"
#include "Types.hpp"

#include "Component/Mesh.hpp"

#include "ECS/Entity.hpp"

#include "glm/vec2.hpp"

#include <optional>

namespace ECS
{
	class Storage;
}
namespace System
{
	class SceneSystem;
}

namespace OpenGL
{
	class Buffer;

	// GPU Color-ID picking renderer.
	// Renders each selectable entity with a flat colour encoding its Entity ID into a dedicated FBO.
	// On click, reads the single pixel under the cursor to determine the selected entity.
	class PickRenderer
	{
		Shader m_pick_shader;
		Shader m_debug_shader;
		std::optional<FBO> m_pick_FBO;
		std::optional<FBO> m_debug_FBO;        // RGBA8 FBO for visualising the pick buffer.
		Data::Mesh m_screen_quad;
		std::optional<Data::Mesh> m_point_light_mesh; // Icosphere used for picking point lights.

	public:
		bool m_show_debug_view = false;

		PickRenderer() noexcept;

		// Render the pick pass and read back the entity under the cursor.
		//@param p_cursor_pos The cursor position in viewport-content-space (origin top-left).
		//@param p_viewport_resolution The resolution of the viewport FBO.
		//@param p_entities The ECS storage containing all entities.
		//@param p_view_properties The UBO containing view/projection matrices.
		//@param p_draw_terrain Whether the terrain surface is currently visible and pickable.
		//@param p_light_position_scale The scale for point light debug spheres.
		//@param p_show_light_positions Whether point light debug spheres are visible and therefore pickable.
		//@return The entity under the cursor, or std::nullopt if no entity was hit.
		std::optional<ECS::Entity> pick(
			const glm::vec2& p_cursor_pos,
			const glm::uvec2& p_viewport_resolution,
			ECS::Storage& p_entities,
			const Buffer& p_view_properties,
			bool p_draw_terrain,
			float p_light_position_scale,
			bool p_show_light_positions);

		void reload_shaders();

		// Draw an ImGui window showing the pick buffer debug visualisation.
		void draw_UI();
	};
} // namespace OpenGL
