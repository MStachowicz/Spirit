#include "OpenGL/Shader.hpp"

#include "Component/Mesh.hpp"

namespace OpenGL
{
	class GridRenderer
	{
		static Data::NewMesh make_grid_mesh();
		static Data::NewMesh make_origin_arrows_mesh();

		Shader m_grid_shader;
		Data::NewMesh m_grid;
		Data::NewMesh m_origin_arrows;

	public:
		GridRenderer() noexcept;
		void draw();
	};
}