#include "Mesh.hpp"

namespace Component
{
	Mesh::Mesh(MeshRef& p_mesh)
		: m_mesh{p_mesh}
	{}

	void Mesh::draw_UI()
	{
	}
} // namespace Component