#include "Mesh.hpp"

#include "Utility/Utility.hpp"

#include "imgui.h"

namespace Data
{
	void Mesh::draw_UI()
	{
		auto formated_verts = Utility::number_with_seperator(VAO.draw_count());
		ImGui::Text_Manual("Vertices:   %s",formated_verts.c_str());

		if (VAO.draw_primitive_mode() == OpenGL::PrimitiveMode::Triangles)
		{
			auto formated_tris = Utility::number_with_seperator(VAO.draw_count() / 3);
			ImGui::Text_Manual("Triangles:  %s", formated_tris.c_str());
		}
		else if (VAO.draw_primitive_mode() == OpenGL::PrimitiveMode::Lines)
		{
			auto formated_lines = Utility::number_with_seperator(VAO.draw_count() / 2);
			ImGui::Text_Manual("Lines:      %s", formated_lines.c_str());
		}
		else if (VAO.draw_primitive_mode() == OpenGL::PrimitiveMode::Points)
		{
			auto formated_points = Utility::number_with_seperator(VAO.draw_count());
			ImGui::Text_Manual("Points:     %s", formated_points.c_str());
		}
		else
			ImGui::Text("Unknown primitive mode");

		ImGui::SameLine();
		ImGui::Text(VAO.is_indexed() ? " (Indexed)" : " (Not Indexed)");

		auto formatted_capacity      = Utility::format_number(vert_buffer.capacity());
		auto formatted_used_capacity = Utility::format_number(vert_buffer.used_capacity());
		ImGui::Text_Manual("Buffer size %sB", formatted_capacity.c_str());
		ImGui::Text_Manual("Buffer used %sB (%.2f%%)", formatted_used_capacity.c_str(), vert_buffer.used_capacity_ratio() * 100.f);

		AABB.draw_UI("Bounds");
	}
}

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
				m_mesh->draw_UI();
			else
				ImGui::Text("Mesh is null");

			ImGui::TreePop();
		}
	}
} // namespace Component