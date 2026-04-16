#pragma once

#include "IPhysicsSystem.hpp"

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtc/quaternion.hpp"

#include <vector>

namespace System
{
	class SceneSystem;

	// A numerical integrator, PhysicsSystem take Transform and RigidBody components and applies kinematic equations.
	// The system is force based and numerically integrates
	class PhysicsSystem : public IPhysicsSystem
	{
		// ContactPoint encapsulates the information about a point of contact between two shapes.
		// A displacement applied along the normal by the penetration_depth separates the two shapes.
		struct ContactPoint
		{
			glm::vec3 position      = glm::vec3(0.f); // The point of contact on the surface of shape A.
			glm::vec3 normal        = glm::vec3(0.f); // The collision response normal of the contact point from the perspective of shape A (normalised).
			float penetration_depth = 0.f;            // The depth of overlap. Unsigned displacement required to separate the two shapes along normal.
		};
		// An idealised body that exhibits 0 deformation. All units are in SI.
		class RigidBody
		{
		public:
			Geometry::AABB m_AABB; // World space AABB used for broad phase collision detection.

			glm::vec3 m_position;    // Position of the body's center of mass in world space (m)
			glm::quat m_orientation; // Orientation of the body in world space (unitless)

			// Linear motion
			// -----------------------------------------------------------------------------
			glm::vec3 m_force;        // Linear force F in Newtons (kg m/s²), This force is applied on a PhysicsSystem tick and gets reset to 0.
			glm::vec3 m_momentum;     // Linear momentum p in Newton seconds (kg m/s)
			glm::vec3 m_acceleration; // Linear acceleration a (m/s²)
			glm::vec3 m_velocity;     // Linear velocity v (m/s)

			// Angular motion
			// -----------------------------------------------------------------------------
			glm::vec3 m_torque;           // Angular force T in Newton meters producing a change in rotational motion (kg m²/s²)
			glm::vec3 m_angular_momentum; // Angular momentum L in Newton meter seconds, a conserved quantity if no external torque is applied (kg m²/s)
			glm::vec3 m_angular_velocity; // Angular velocity ω representing how quickly (Hz) this body revolves relative to it's axis (/s)
			glm::mat3 m_inertia_tensor;   // Moment of inertia tensor J, a symmetric matrix determining the torque needed for a desired angular acceleration about a rotational axis (kg m2)

			float m_restitution; // Coefficient of restitution applied in collision response.
			float m_mass;        // Inertial mass measuring the body's resistance to acceleration when a force is applied (kg)
			bool m_apply_gravity;
			// Position and orientation are stored in Component::Transform.

			RigidBody(bool p_apply_gravity = true) noexcept;
			// Apply a linear p_force (kg m/s²) on the body. Force is applied on a PhysicsSystem::update tick.
			void apply_impulse(const glm::vec3& p_force) { m_force += p_force; }
		};

		std::vector<RigidBody> m_bodies;

		SceneSystem& m_scene_system;

		DeltaTime m_total_simulation_time; // Total time simulated using the integrate function.
		glm::vec3 m_gravity;               // The acceleration due to gravity.

	public:
		PhysicsSystem(SceneSystem& scene_system);

		void step(const DeltaTime& p_delta_time) override;

		[[nodiscard]] PhysicsSystemHandle create_body(const BodySettings& p_body_settings, const ECS::Entity& p_entity) override { (void)p_body_settings; (void)p_entity; throw std::runtime_error("Not implemented"); };
		void destroy_body(const PhysicsSystemHandle& p_handle) override                             { (void)p_handle; throw std::runtime_error("Not implemented"); };


		void apply_impulse(const PhysicsSystemHandle& p_ID, const glm::vec3& p_force) override { (void)p_ID; (void)p_force; throw std::runtime_error("Not implemented"); };

		void set_position(const PhysicsSystemHandle& p_ID, const glm::vec3& p_position) override { (void)p_ID; (void)p_position; throw std::runtime_error("Not implemented"); };
		glm::vec3 get_position(const PhysicsSystemHandle& p_ID) const override                   { (void)p_ID; throw std::runtime_error("Not implemented"); };

		void set_rotation(const PhysicsSystemHandle& p_ID, const glm::quat& p_rotation) override { (void)p_ID; (void)p_rotation; throw std::runtime_error("Not implemented"); };
		glm::quat get_rotation(const PhysicsSystemHandle& p_ID) const override                   { (void)p_ID; throw std::runtime_error("Not implemented"); };

		Geometry::AABB get_bounding_box() const override;
		std::optional<ECS::Entity> cast_ray(const Geometry::Ray& p_ray) const override { (void)p_ray; throw std::runtime_error("Not implemented"); };
	};
} // namespace System