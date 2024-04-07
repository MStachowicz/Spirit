#pragma once

#include "glm/vec3.hpp"

#include "Utility/Config.hpp"

namespace System
{
	class SceneSystem;
	class CollisionSystem;

	// A numerical integrator, PhysicsSystem take Transform and RigidBody components and applies kinematic equations.
	// The system is force based and numerically integrates
	class PhysicsSystem
	{
	public:
		PhysicsSystem(SceneSystem& scene_system, CollisionSystem& collision_system);
		void integrate(const DeltaTime& delta_time);

		size_t m_update_count;
		float m_restitution;             // Coefficient of restitution applied in collision response.
		bool m_apply_collision_response; // Whether to apply collision response or not.
		bool m_bool_apply_kinematic;     // Whether to apply kinematic equations or not.

	private:
		SceneSystem& m_scene_system;
		CollisionSystem& m_collision_system;

		DeltaTime m_total_simulation_time; // Total time simulated using the integrate function.
		glm::vec3 m_gravity;               // The acceleration due to gravity.
	};
} // namespace System