#include "Collider.hpp"

#include "imgui.h"
#include "glm/glm.hpp"

namespace Component
{
	Collider::Collider()
		: m_world_AABB{}
		, m_collision_shapes{}
		, m_collided(false)
	{}

	void Collider::draw_UI()
	{
		if (ImGui::TreeNode("Collider"))
		{
			ImGui::Checkbox("Colliding", &m_collided);
			ImGui::Text("World AABB min", m_world_AABB.m_min);
			ImGui::Text("World AABB max", m_world_AABB.m_max);

			if (m_collision_shapes.size() == 0)
				ImGui::Text("No collision shapes.");
			else if (m_collision_shapes.size() == 1)
				std::visit([&](auto&& arg){ arg.draw_UI(); }, m_collision_shapes[0].shape);
			else if (ImGui::TreeNode("Collision Shapes"))
			{
				for (auto& shape : m_collision_shapes)
					std::visit([&](auto&& arg){ arg.draw_UI(); }, shape.shape);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
}