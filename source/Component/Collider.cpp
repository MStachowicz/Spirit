#include "Collider.hpp"

#include "imgui.h"
#include "glm/glm.hpp"

namespace Component
{
	Collider::Collider()
		: m_collided(false)
		, m_world_AABB{}
	{}

	void Collider::draw_UI()
	{
		if (ImGui::TreeNode("Collider"))
		{
			ImGui::Checkbox("Colliding", &m_collided);
			ImGui::Text("World AABB min: {}, {}, {}", m_world_AABB.mMin.x, m_world_AABB.mMin.y, m_world_AABB.mMin.z);
			ImGui::Text("World AABB max: {}, {}, {}", m_world_AABB.mMax.x, m_world_AABB.mMax.y, m_world_AABB.mMax.z);

			ImGui::TreePop();
		}
	}
}