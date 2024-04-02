#include "Transform.hpp"
#include "Utility/Utility.hpp"
#include "Utility/Serialise.hpp"

#include "imgui.h"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace Component
{
	void Transform::rotate_euler_degrees(const glm::vec3& p_roll_pitch_yaw_degrees)
	{
		m_orientation = glm::normalize(Utility::to_quaternion(glm::radians(p_roll_pitch_yaw_degrees)));
	}
	void Transform::look_at(const glm::vec3& p_point)
	{
		if (p_point != m_position)
		{
			auto new_forward = glm::normalize(p_point - m_position);
			m_orientation    = Utility::get_rotation(Starting_Forward_Direction, new_forward);
		}
	}

	void Transform::set_model(const glm::mat4& p_model)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(p_model, m_scale, m_orientation, m_position, skew, perspective);
	}

	void Transform::draw_UI()
	{
		if (ImGui::TreeNode("Transform"))
		{
			ImGui::Slider("Position", m_position, -50.f, 50.f, "%.3f m");
			ImGui::Slider("Scale", m_scale, 0.1f, 10.f);

			// Editor shows the rotation as Euler Roll, Pitch, Yaw, when set these need to be converted to quaternion orientation and unit direction.
			glm::vec3 euler_degrees = Utility::to_roll_pitch_yaw(m_orientation);
			if (ImGui::Slider("Roll Pitch Yaw", euler_degrees, -179.f, 179.f, "%.3f °"))
				rotate_euler_degrees(euler_degrees);

			ImGui::Separator();
			ImGui::Text("Directon",    forward());
			ImGui::Text("Orientation", m_orientation);

			ImGui::SeparatorText("Actions");
			if (ImGui::Button("Focus on origin"))
				look_at(glm::vec3(0.f));
			ImGui::SameLine();

			// https://glm.g-truc.net/0.9.2/api/a00259.html#
			if (ImGui::Button("Reset"))
			{
				m_position       = glm::vec3(0.0f, 0.0f, 0.0f);
				m_scale          = glm::vec3(1.0f);
				m_orientation    = glm::identity<glm::quat>();
			}
			ImGui::TreePop();
		}
	}
	void Transform::Serialise(const Transform& p_transform, std::ofstream& p_out, uint16_t p_version)
	{ (void) p_version;
		Utility::write_binary(p_out, p_transform.m_position);
		Utility::write_binary(p_out, p_transform.m_scale);
		Utility::write_binary(p_out, p_transform.m_orientation);
	}
	Transform Transform::Deserialise(std::ifstream& p_in, uint16_t p_version)
	{ (void) p_version;
		Transform transform;
		Utility::read_binary(p_in, transform.m_position);
		Utility::read_binary(p_in, transform.m_scale);
		Utility::read_binary(p_in, transform.m_orientation);
		return transform;
	}
} // namespace Component