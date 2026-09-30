#pragma once

#include "Shader.hpp"

#include "Component/Mesh.hpp"

namespace Component
{
	struct ViewInformation;
}

namespace OpenGL
{
	class FBO;

	class GridRenderer
	{
		static Data::Mesh make_screen_triangle();
		static Data::Mesh make_origin_arrows_mesh();

		Shader m_grid_shader;
		Shader m_origin_shader;
		Data::Mesh m_screen_triangle;
		Data::Mesh m_origin_arrows;

	public:
		GridRenderer() noexcept;

		void draw(const FBO& p_target_FBO, const Component::ViewInformation& p_view_info, const Buffer& p_view_properties);
		void reload_shaders();
	};
}