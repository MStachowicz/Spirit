#pragma once

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"

#include <iostream>

namespace Component
{
	// An idealised body that exhibits 0 deformation. All units are in SI.
	class RigidBody
	{
	public:
		constexpr static size_t Persistent_ID = 3;

		// Linear motion
		// -----------------------------------------------------------------------------
		glm::vec3 m_force;        // Linear force F in Newtons (kg m/s²), This force is applied on a PhysicsSystem tick and gets reset to 0.
		glm::vec3 m_momentum;     // Linear momentum p in Newton seconds (kg m/s)
		glm::vec3 m_acceleration; // Linear acceleration a (m/s²)
		glm::vec3 m_velocity;     // Linear velocity v (m/s)

		// Angular motion
		// -----------------------------------------------------------------------------
		glm::vec3 m_torque;          // Angular force T in Newton meters producing a change in rotational motion (kg m²/s²)
		glm::vec3 m_angular_momentum; // Angular momentum L in Newton meter seconds, a conserved quantity if no external torque is applied (kg m²/s)
		glm::vec3 m_angular_velocity; // Angular velocity ω representing how quickly (Hz) this body revolves relative to it's axis (/s)
		glm::mat3 m_inertia_tensor;   // Moment of inertia tensor J, a symmetric matrix determining the torque needed for a desired angular acceleration about a rotational axis (kg m2)

		float m_mass; // Inertial mass measuring the body's resistance to acceleration when a force is applied (kg)
		bool m_apply_gravity;
		// Position and orientation are stored in Component::Transform.

		RigidBody(bool p_apply_gravity = true) noexcept;
		// Apply a linear p_force (kg m/s²) on the body. Force is applied on a PhysicsSystem::update tick.
		void apply_linear_force(const glm::vec3& p_force);
		void draw_UI();

		static void serialise(std::ostream& p_out, uint16_t p_version, const RigidBody& p_rigid_body);
		static RigidBody deserialise(std::istream& p_in, uint16_t p_version);
	};
}