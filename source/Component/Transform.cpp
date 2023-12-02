#include "Transform.hpp"
#include "Utility/Utility.hpp"

#include "imgui.h"
#include "glm/gtx/euler_angles.hpp"

namespace Component
{
	void Transform::rotateEulerDegrees(const glm::vec3& p_roll_pitch_yawDegrees)
	{
		m_roll_pitch_yaw = p_roll_pitch_yawDegrees;
		m_orientation    = glm::normalize(Utility::to_quaternion(glm::radians(m_roll_pitch_yaw)));
		m_direction      = glm::normalize(m_orientation * Starting_Forward_Direction);
	}
	void Transform::look_at(const glm::vec3& p_point)
	{
		if (p_point != m_position)
		{
			m_direction      = glm::normalize(p_point - m_position);
			m_orientation    = Utility::get_rotation(Starting_Forward_Direction, m_direction);
			m_roll_pitch_yaw = Utility::to_roll_pitch_yaw(m_orientation);
		}
	}

	void Transform::draw_UI()
	{
		if (ImGui::TreeNode("Transform"))
		{
			ImGui::Slider("Position", m_position, -50.f, 50.f, "%.3f m");
			ImGui::Slider("Scale", m_scale, 0.1f, 10.f);

			// Editor shows the rotation as Euler Roll, Pitch, Yaw, when set these need to be converted to quaternion orientation and unit direction.
			if (ImGui::Slider("Roll Pitch Yaw", m_roll_pitch_yaw, -179.f, 179.f, "%.3f °"))
				rotateEulerDegrees(m_roll_pitch_yaw);

			ImGui::Separator();
			ImGui::Text("Directon",    m_direction);
			ImGui::Text("Orientation", m_orientation);

			ImGui::SeparatorText("Actions");
			if (ImGui::Button("Focus on origin"))
				look_at(glm::vec3(0.f));
			ImGui::SameLine();

			// https://glm.g-truc.net/0.9.2/api/a00259.html#
			if (ImGui::Button("Reset"))
			{
				m_position       = glm::vec3(0.0f, 0.0f, 0.0f);
				m_roll_pitch_yaw = glm::vec3(0.0f);
				m_scale          = glm::vec3(1.0f);
				m_direction      = Starting_Forward_Direction;
				m_orientation    = glm::identity<glm::quat>();
			}
			ImGui::TreePop();
		}
	}
} // namespace Component