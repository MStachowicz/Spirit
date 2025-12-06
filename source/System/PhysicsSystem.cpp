#include "PhysicsSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/Transform.hpp"
#include "Component/Collider.hpp"
#include "Component/Mesh.hpp"
#include "ECS/Storage.hpp"

#include "Geometry/Geometry.hpp"
#include "Geometry/Intersect.hpp"

#include "Utility/Performance.hpp"
#include "Utility/Utility.hpp"

#include <thread>

namespace System
{
	PhysicsSystem::PhysicsSystem(SceneSystem& scene_system)
		: m_scene_system{scene_system}
		, m_total_simulation_time{DeltaTime::zero()}
		, m_gravity{glm::vec3(0.f, -9.81f, 0.f)}
	{}

	void PhysicsSystem::update(const DeltaTime& p_delta_time)
	{
		static_assert(std::is_same_v<DeltaTime, std::chrono::duration<float, std::ratio<1>>>, "PhysicsSystem::integrate expects DeltaTime to be a duration in seconds with float precision.");
		PERF(PhysicsSystemIntegrate);

		if (m_pause_simulation)
			return;

		m_update_count++;
		m_total_simulation_time += p_delta_time;

		for (auto& rigid_body : m_bodies)
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
				rigid_body.m_position += change_in_position;

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
				const glm::quat spin = 0.5f * glm::quat(0.f, (rigid_body.m_angular_velocity * p_delta_time.count())) * rigid_body.m_orientation;

				// Integrate spin to find the new orientation
				rigid_body.m_orientation += spin;
				rigid_body.m_orientation = glm::normalize(rigid_body.m_orientation);
			}
		}

		// ECS::Entity collided_entity = ECS::Entity(0);
		// if (auto collision = get_collision(entity, &collided_entity))
		// {
		// 	// A collision has occurred at the new position, the response depends on the collided entity having a rigibBody to apply a response to.
		// 	// We already know the collided Entity has a Transform component from CollisionSystem::getCollision so we dont have to check it here.
		// 	// The collision data returned is original-Entity-centric this convention is carried over in the response here when calling angular_impulse.
		// 	if (scene.has_components<Component::Collider>(collided_entity))
		// 	{
		// 		auto& rigid_body_2 = m_scene_system.get_current_scene_entities().get_component<Component::Collider>(collided_entity).m_rigid_body;
		// 		auto& transform_2  = m_scene_system.get_current_scene_entities().get_component<Component::Transform>(collided_entity);

		// 		auto impulse = Geometry::angular_impulse(collision->position, collision->normal, rigid_body.m_restitution,
		// 												transform.m_position, rigid_body.m_velocity, rigid_body.m_angular_velocity, rigid_body.m_mass, rigid_body.m_inertia_tensor,
		// 												transform_2.m_position, rigid_body_2.m_velocity, rigid_body_2.m_angular_velocity, rigid_body_2.m_mass, rigid_body_2.m_inertia_tensor);

		// 		const auto r             = collision->position - transform.m_position;
		// 		const auto inverseTensor = glm::inverse(rigid_body.m_inertia_tensor);

		// 		rigid_body.m_velocity        = rigid_body.m_velocity + (impulse / rigid_body.m_mass);
		// 		rigid_body.m_angular_velocity = rigid_body.m_angular_velocity + (glm::cross(r, impulse) * inverseTensor);

		// 		// #TODO: Apply a response to collision.mEntity
		// 	}
		// }
	}

	PhysicsSystem::RigidBody::RigidBody(bool p_apply_gravity) noexcept
		: m_position{0.f}
		, m_orientation{glm::identity<glm::quat>()}
		, m_force{0.f}
		, m_momentum{0.f}
		, m_acceleration{0.f}
		, m_velocity{0.f}
		, m_torque{0.f}
		, m_angular_momentum{0.f}
		, m_angular_velocity{0.f}
		, m_inertia_tensor{glm::identity<glm::mat3>()}
		, m_restitution{0.8f}
		, m_mass{1.f}
		, m_apply_gravity{p_apply_gravity}
	{}

} // namespace System