#include "Collider.hpp"

#include "Utility/Serialise.hpp"

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

	void Collider::serialise(std::ostream& p_out, uint16_t p_version, const Collider& p_collider)
	{
		Utility::write_binary(p_out, p_version, p_collider.m_world_AABB);
		Utility::write_binary(p_out, p_version, p_collider.m_collided);
	}
	Collider Collider::deserialise(std::istream& p_in, uint16_t p_version)
	{
		Collider collider;
		Utility::read_binary(p_in, p_version, collider.m_world_AABB);
		Utility::read_binary(p_in, p_version, collider.m_collided);
		return collider;
	}
	static_assert(Utility::Is_Serializable_v<Collider>, "Collider is not serializable, check that the required functions are implemented.");
}