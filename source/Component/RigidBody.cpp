#include "RigidBody.hpp"

#include "Utility/Serialise.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

namespace Component
{
	RigidBody::RigidBody(bool p_apply_gravity/*= true*/) noexcept
		: m_force{0.f, 0.f, 0.f}
		, m_momentum{0.f, 0.f, 0.f}
		, m_acceleration{0.f, 0.f, 0.f}
		, m_velocity{0.f, 0.f, 0.f}
		, m_torque{0.f, 0.f, 0.f}
		, m_angular_momentum{0.f, 0.f, 0.f}
		, m_angular_velocity{0.f, 0.f, 0.f}
		, m_inertia_tensor{glm::identity<glm::mat3>()}
		, m_mass{1}
		, m_apply_gravity{p_apply_gravity}
	{}

	void RigidBody::apply_linear_force(const glm::vec3& p_force)
	{
		m_force += p_force;
	}

	void RigidBody::draw_UI()
	{
		if(ImGui::TreeNode("Rigid body"))
		{
			ImGui::SliderFloat3("Force                  (N)", &m_force.x, -10.f, 10.f);
			ImGui::SliderFloat3("Momentum          (kg m/s)", &m_momentum.x, -10.f, 10.f);
			ImGui::SliderFloat3("Acceleration        (m/s²)", &m_acceleration.x, -10.f, 10.f);
			ImGui::SliderFloat3("Velocity             (m/s)", &m_velocity.x, -10.f, 10.f);
			ImGui::SliderFloat( "Mass                  (kg)", &m_mass, 0.001f, 100.f);

			ImGui::Separator();
			ImGui::SliderFloat3("Torque               (N m)", &m_torque.x, -10.f, 10.f);
			ImGui::SliderFloat3("Angular Momentum (kg m²/s)", &m_angular_momentum.x, -10.f, 10.f);
			ImGui::SliderFloat3("Angular Velocity   (rad/s)", &m_angular_velocity.x, -10.f, 10.f);

			ImGui::Separator();
			const float inertiaLimit = m_mass * 100.f;
			ImGui::SliderFloat3("Angular Tensor 1   (kg m²)", &m_inertia_tensor[0][0], 0.001f, inertiaLimit);
			ImGui::SliderFloat3("Angular Tensor 2   (kg m²)", &m_inertia_tensor[1][0], 0.001f, inertiaLimit);
			ImGui::SliderFloat3("Angular Tensor 3   (kg m²)", &m_inertia_tensor[2][0], 0.001f, inertiaLimit);

			ImGui::Separator();
			ImGui::Checkbox("Apply Gravity", &m_apply_gravity);
			ImGui::TreePop();

			if (ImGui::Button("Reset"))
			{
				//m_mass            = {1};
				m_apply_gravity    = {false};
				m_force            = {0.f, 0.f, 0.f};
				m_momentum         = {0.f, 0.f, 0.f};
				m_acceleration     = {0.f, 0.f, 0.f};
				m_velocity         = {0.f, 0.f, 0.f};
				m_torque           = {0.f, 0.f, 0.f};
				m_angular_momentum = {0.f, 0.f, 0.f};
				m_angular_velocity = {0.f, 0.f, 0.f};
				//m_inertia_tensor   = {glm::identity<glm::mat3>()};
			}
		}
	}
	void RigidBody::serialise(std::ostream& p_out, uint16_t p_version, const RigidBody& p_rigid_body)
	{
		Utility::write_binary(p_out, p_version, p_rigid_body.m_force);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_momentum);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_acceleration);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_velocity);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_torque);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_angular_momentum);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_angular_velocity);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_inertia_tensor);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_mass);
		Utility::write_binary(p_out, p_version, p_rigid_body.m_apply_gravity);
	}
	RigidBody RigidBody::deserialise(std::istream& p_in, uint16_t p_version)
	{
		RigidBody rigid_body;
		Utility::read_binary(p_in, p_version, rigid_body.m_force);
		Utility::read_binary(p_in, p_version, rigid_body.m_momentum);
		Utility::read_binary(p_in, p_version, rigid_body.m_acceleration);
		Utility::read_binary(p_in, p_version, rigid_body.m_velocity);
		Utility::read_binary(p_in, p_version, rigid_body.m_torque);
		Utility::read_binary(p_in, p_version, rigid_body.m_angular_momentum);
		Utility::read_binary(p_in, p_version, rigid_body.m_angular_velocity);
		Utility::read_binary(p_in, p_version, rigid_body.m_inertia_tensor);
		Utility::read_binary(p_in, p_version, rigid_body.m_mass);
		Utility::read_binary(p_in, p_version, rigid_body.m_apply_gravity);
		return rigid_body;
	}
	static_assert(Utility::Is_Serializable_v<RigidBody>, "RigidBody is not serializable, check that the required functions are implemented.");
} // namespace Component