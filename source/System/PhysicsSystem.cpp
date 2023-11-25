#include "PhysicsSystem.hpp"
#include "Component/Camera.hpp"
#include "Component/Mesh.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/CollisionSystem.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/Geometry.hpp"

#include "Utility/Utility.hpp"

namespace System
{
	PhysicsSystem::PhysicsSystem(SceneSystem& scene_system, CollisionSystem& collision_system)
		: m_update_count{0}
		, m_restitution{0.8f}
		, m_apply_collision_response{true}
		, m_scene_system{scene_system}
		, m_collision_system{collision_system}
		, m_total_simulation_time{DeltaTime::zero()}
		, m_gravity{glm::vec3(0.f, -9.81f, 0.f)}
	{}

	void PhysicsSystem::integrate(const DeltaTime& pDeltaTime)
	{
		m_update_count++;
		m_total_simulation_time += pDeltaTime;

		auto& scene = m_scene_system.getCurrentScene();
		scene.foreach([this, &pDeltaTime, &scene](ECS::Entity& entity, Component::RigidBody& rigid_body, Component::Transform& transform)
		{
			if (rigid_body.mApplyGravity)
				rigid_body.mForce += rigid_body.mMass * m_gravity; // F = ma

			{ // Linear motion
				// Change in momentum is equal to the force = dp/dt = F
				const auto changeInMomentum = rigid_body.mForce * pDeltaTime.count(); // dp = F dt
				rigid_body.mMomentum += changeInMomentum;

				// Convert momentum to velocity by dividing by mass: p = mv
				rigid_body.mVelocity = rigid_body.mMomentum / rigid_body.mMass; // v = p/v

				// Integrate velocity to find new position: dx/dt = v
				const auto changeInPosition = rigid_body.mVelocity * pDeltaTime.count(); // dx = v dt
				transform.mPosition += changeInPosition;

				rigid_body.mForce = glm::vec3(0.f); // Reset back to 0 after applying the force on the body.
			}

			{ // Angular motion
				// http://physics.bu.edu/~redner/211-sp06/class-rigid-body/angularmo.html
				const auto changeInAngularMomentum = rigid_body.mTorque * pDeltaTime.count(); // dL = T dt
				rigid_body.mAngularMomentum += changeInAngularMomentum;

				// Convert angular momentum to angular velocity by dividing by inertia tensor: L = Iω
				rigid_body.mAngularVelocity = rigid_body.mAngularMomentum / rigid_body.mInertiaTensor; // ω = L / I

				// To integrate the new quat orientation we convert the angular velocity into quaternion form - spin.
				// Spin represents a time derivative of orientation. https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
				const glm::quat spin = 0.5f * glm::quat(0.f, (rigid_body.mAngularVelocity * pDeltaTime.count())) * transform.mOrientation;

				// Integrate spin to find the new orientation
				transform.mOrientation += spin;
				transform.mOrientation = glm::normalize(transform.mOrientation);
				// Recalculate the direction and rotation mat
				transform.mDirection = glm::normalize(transform.mOrientation * Component::Transform::Starting_Forward_Direction);
				transform.mRollPitchYaw = glm::degrees(Utility::toRollPitchYaw(transform.mOrientation));
			}

			const auto rotationMatrix = glm::mat4_cast(transform.mOrientation);

			transform.mModel = glm::translate(glm::identity<glm::mat4>(), transform.mPosition);
			transform.mModel *= rotationMatrix;
			transform.mModel = glm::scale(transform.mModel, transform.mScale);

			// After moving and updating the Collider, check for collisions and respond

			ECS::Entity collided_entity = ECS::Entity(0);
			if (auto collision = m_collision_system.get_collision(entity, &collided_entity))
			{
				if (m_apply_collision_response)
				{
					// A collision has occurred at the new position, the response depends on the collided entity having a rigibBody to apply a response to.
					// We already know the collided Entity has a Transform component from CollisionSystem::getCollision so we dont have to check it here.
					// The collision data returned is original-Entity-centric this convention is carried over in the response here when calling angularImpulse.
					if (scene.hasComponents<Component::RigidBody>(collided_entity))
					{
						auto& rigidBody2 = m_scene_system.getCurrentScene().getComponentMutable<Component::RigidBody>(collided_entity);
						auto& transform2 = m_scene_system.getCurrentScene().getComponentMutable<Component::Transform>(collided_entity);

						auto impulse = Geometry::angularImpulse(collision->position, collision->normal, m_restitution,
																transform.mPosition, rigid_body.mVelocity, rigid_body.mAngularVelocity, rigid_body.mMass, rigid_body.mInertiaTensor,
																transform2.mPosition, rigidBody2.mVelocity, rigidBody2.mAngularVelocity, rigidBody2.mMass, rigidBody2.mInertiaTensor);

						const auto r             = collision->position - transform.mPosition;
						const auto inverseTensor = glm::inverse(rigid_body.mInertiaTensor);

						rigid_body.mVelocity        = rigid_body.mVelocity + (impulse / rigid_body.mMass);
						rigid_body.mAngularVelocity = rigid_body.mAngularVelocity + (glm::cross(r, impulse) * inverseTensor);

						// #TODO: Apply a response to collision.mEntity
					}
				}
			}
		});
	}
} // namespace System