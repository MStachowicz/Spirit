#pragma once

#include "Types.hpp"
#include "Shader.hpp"

#include "glm/vec2.hpp"

namespace System
{
	class Scene;
}
namespace OpenGL
{
	class ShadowMapper
	{
		FBO m_depth_map_FBO;
		Shader m_shadow_depth_shader;

	public:
		ShadowMapper(const glm::uvec2& p_resolution) noexcept;

		// Renders the scene from the perspective of the light source to fill a depth texture map.
		void shadow_pass(System::Scene& p_scene);
		const Texture& get_depth_map() const { return m_depth_map_FBO.depth_attachment(); };

		void draw_UI();
	};
}