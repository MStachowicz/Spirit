#include "Collider.hpp"

#include "imgui.h"
#include "glm/glm.hpp"

namespace Component
{
	Collider::Collider()
		: m_world_AABB{}
		, m_collided(false)
	{}

	void Collider::draw_UI()
	{
		if (ImGui::TreeNode("Collider"))
		{
			ImGui::Checkbox("Colliding", &m_collided);
			ImGui::Text("World AABB min", m_world_AABB.m_min);
			ImGui::Text("World AABB max", m_world_AABB.m_max);

			ImGui::TreePop();
		}
	}
}