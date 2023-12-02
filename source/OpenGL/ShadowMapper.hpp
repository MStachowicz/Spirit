#pragma once

#include "Types.hpp"
#include "Shader.hpp"

#include "glm/vec2.hpp"

namespace System
{
	class Scene;
}
namespace Platform
{
	class Window;
}
namespace OpenGL
{
	class ShadowMapper
	{
		FBO m_depth_map_FBO;
		Shader m_shadow_depth_shader;
		glm::uvec2 m_resolution;

		Platform::Window& m_window;

	public:
		ShadowMapper(Platform::Window& p_window) noexcept;

		void shadow_pass(System::Scene& p_scene);
		const Texture& get_depth_map() const { return m_depth_map_FBO.m_depth_map.value(); };

		void draw_UI();
	};
}