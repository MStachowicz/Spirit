#include "Mesh.hpp"

#include "imgui.h"

namespace Component
{
	Mesh::Mesh(MeshRef& p_mesh)
		: m_mesh{p_mesh}
	{}

	void Mesh::draw_UI()
	{
		if (ImGui::TreeNode("Mesh"))
		{
			if (m_mesh)
				ImGui::Text("Draw size: %d", m_mesh->get_VAO().draw_count());
			else
				ImGui::Text("Mesh is null");

			ImGui::TreePop();
		}
	}
} // namespace Component