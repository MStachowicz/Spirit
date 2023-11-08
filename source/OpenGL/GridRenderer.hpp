#include "OpenGL/Shader.hpp"

#include "Component/Mesh.hpp"

namespace OpenGL
{
	class GridRenderer
	{
		static Data::NewMesh make_grid_mesh();

		Shader m_grid_shader;
		Data::NewMesh m_grid_mesh;

	public:
		GridRenderer() noexcept;
		void draw();
	};
}