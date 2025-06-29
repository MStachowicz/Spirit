#include "PhysicsSystem.hpp"
#include "CollisionSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"

#include "Geometry/Geometry.hpp"
#include "Utility/Performance.hpp"
#include "Utility/Utility.hpp"

namespace System
{
	PhysicsSystem::PhysicsSystem(SceneSystem& scene_system, CollisionSystem& collision_system)
		: m_update_count{0}
		, m_restitution{0.8f}
		, m_apply_collision_response{true}
		, m_bool_apply_kinematic{true}
		, m_scene_system{scene_system}
		, m_collision_system{collision_system}
		, m_total_simulation_time{DeltaTime::zero()}
		, m_gravity{glm::vec3(0.f, -9.81f, 0.f)}
	{}

	void PhysicsSystem::integrate(const DeltaTime& p_delta_time)
	{
		static_assert(std::is_same_v<DeltaTime, std::chrono::duration<float, std::ratio<1>>>, "PhysicsSystem::integrate expects DeltaTime to be a duration in seconds with float precision.");

		PERF(PhysicsSystemIntegrate);

		m_update_count++;
		m_total_simulation_time += p_delta_time;

		if (!m_bool_apply_kinematic)
			return;

		auto& scene = m_scene_system.get_current_scene_entities();
		scene.foreach([this, &p_delta_time, &scene](ECS::Entity& entity, Component::RigidBody& rigid_body, Component::Transform& transform)
		{
			if (rigid_body.m_apply_gravity)
				rigid_body.m_force += rigid_body.m_mass * m_gravity; // F = ma

			{ // Linear motion
				// Change in momentum is equal to the force = dp/dt = F
				const auto change_in_momentum = rigid_body.m_force * p_delta_time.count(); // dp = F dt
				rigid_body.m_momentum += change_in_momentum;

				// Convert momentum to velocity by dividing by mass: p = mv
				rigid_body.m_velocity = rigid_body.m_momentum / rigid_body.m_mass; // v = p/v

				// Integrate velocity to find new position: dx/dt = v
				const auto change_in_position = rigid_body.m_velocity * p_delta_time.count(); // dx = v dt
				transform.m_position += change_in_position;

				rigid_body.m_force = glm::vec3(0.f); // Reset back to 0 after applying the force on the body.
			}

			{ // Angular motion
				// http://physics.bu.edu/~redner/211-sp06/class-rigid-body/angularmo.html
				const auto change_in_angular_momentum = rigid_body.m_torque * p_delta_time.count(); // dL = T dt
				rigid_body.m_angular_momentum += change_in_angular_momentum;

				// Convert angular momentum to angular velocity by dividing by inertia tensor: L = Iω
				rigid_body.m_angular_velocity = rigid_body.m_angular_momentum / rigid_body.m_inertia_tensor; // ω = L / I

				// To integrate the new quat orientation we convert the angular velocity into quaternion form - spin.
				// Spin represents a time derivative of orientation. https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
				const glm::quat spin = 0.5f * glm::quat(0.f, (rigid_body.m_angular_velocity * p_delta_time.count())) * transform.m_orientation;

				// Integrate spin to find the new orientation
				transform.m_orientation += spin;
				transform.m_orientation = glm::normalize(transform.m_orientation);
			}

			ECS::Entity collided_entity = ECS::Entity(0);
			if (auto collision = m_collision_system.get_collision(entity, &collided_entity))
			{
				if (m_apply_collision_response)
				{
					// A collision has occurred at the new position, the response depends on the collided entity having a rigibBody to apply a response to.
					// We already know the collided Entity has a Transform component from CollisionSystem::getCollision so we dont have to check it here.
					// The collision data returned is original-Entity-centric this convention is carried over in the response here when calling angular_impulse.
					if (scene.has_components<Component::RigidBody>(collided_entity))
					{
						auto& rigid_body_2 = m_scene_system.get_current_scene_entities().get_component<Component::RigidBody>(collided_entity);
						auto& transform_2  = m_scene_system.get_current_scene_entities().get_component<Component::Transform>(collided_entity);

						auto impulse = Geometry::angular_impulse(collision->position, collision->normal, m_restitution,
																transform.m_position, rigid_body.m_velocity, rigid_body.m_angular_velocity, rigid_body.m_mass, rigid_body.m_inertia_tensor,
																transform_2.m_position, rigid_body_2.m_velocity, rigid_body_2.m_angular_velocity, rigid_body_2.m_mass, rigid_body_2.m_inertia_tensor);

						const auto r             = collision->position - transform.m_position;
						const auto inverseTensor = glm::inverse(rigid_body.m_inertia_tensor);

						rigid_body.m_velocity        = rigid_body.m_velocity + (impulse / rigid_body.m_mass);
						rigid_body.m_angular_velocity = rigid_body.m_angular_velocity + (glm::cross(r, impulse) * inverseTensor);

						// #TODO: Apply a response to collision.mEntity
					}
				}
			}
		});
	}
} // namespace System