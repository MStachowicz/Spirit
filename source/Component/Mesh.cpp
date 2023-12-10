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
			{
				if (m_mesh->collision_shapes.size() == 0)
					ImGui::Text("No collision shapes.");
				else if (m_mesh->collision_shapes.size() == 1)
					std::visit([&](auto&& arg){ arg.draw_UI(); }, m_mesh->collision_shapes[0].shape);
				else if (ImGui::TreeNode("Collision Shapes"))
				{
					for (auto& shape : m_mesh->collision_shapes)
						std::visit([&](auto&& arg){ arg.draw_UI(); }, shape.shape);

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
	}
} // namespace Component