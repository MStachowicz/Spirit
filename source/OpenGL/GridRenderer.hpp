#pragma once

#include "Shader.hpp"

#include "Component/Mesh.hpp"

namespace OpenGL
{
	class FBO;

	class GridRenderer
	{
		static Data::Mesh make_grid_mesh();
		static Data::Mesh make_origin_arrows_mesh();

		Shader m_grid_shader;
		Data::Mesh m_grid;
		Data::Mesh m_origin_arrows;

	public:
		GridRenderer() noexcept;

		void draw(const FBO& target_FBO);
		void reload_shaders();
	};
}