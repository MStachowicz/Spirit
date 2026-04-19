#include "Collider.hpp"

#include "Utility/Serialise.hpp"

#include "imgui.h"
#include "glm/glm.hpp"

namespace Component
{
	void Collider::draw_UI()
	{
		if (ImGui::TreeNode("Collider"))
		{
			if (!m_physics_system_handle)
			{
				ImGui::Text("Awaiting physics body creation...");
				ImGui::TreePop();
				return;
			}

			auto body_ID = m_physics_system_handle->m_jolt_body_ID;
			ImGui::Text("Jolt Body ID", body_ID.GetIndexAndSequenceNumber());

			auto pos = get_position();
			auto rot = get_rotation();

			if (ImGui::SliderFloat3("Position", &pos.x, -100.f, 100.f))
				set_position(pos);
			if (ImGui::SliderFloat4("Rotation", &rot.x, -1.f, 1.f))
				set_rotation(rot);

			ImGui::Separator();
			auto vel = get_velocity();
			if (ImGui::SliderFloat3("Velocity", &vel.x, -50.f, 50.f))
				set_velocity(vel);

			ImGui::Separator();
			auto angular_vel = get_angular_velocity();
			if (ImGui::SliderFloat3("Angular Velocity", &angular_vel.x, -50.f, 50.f))
				set_angular_velocity(angular_vel);

			ImGui::Separator();
			auto bounds = get_bounds();
			ImGui::Text("Bounds Min: (%.2f, %.2f, %.2f)", bounds.m_min.x, bounds.m_min.y, bounds.m_min.z);
			ImGui::Text("Bounds Max: (%.2f, %.2f, %.2f)", bounds.m_max.x, bounds.m_max.y, bounds.m_max.z);
			ImGui::TreePop();
		}
	}

	void Collider::serialise(std::ostream& p_out, uint16_t p_version, const Collider& p_collider)
	{ UNUSED(p_out); UNUSED(p_version); UNUSED(p_collider);
	}
	Collider Collider::deserialise(std::istream& p_in, uint16_t p_version)
	{ UNUSED(p_in); UNUSED(p_version);
		throw std::runtime_error("Not implemented yet.");
	}
	static_assert(Utility::Is_Serializable_v<Collider>, "Collider is not serializable, check that the required functions are implemented.");
} // namespace Component