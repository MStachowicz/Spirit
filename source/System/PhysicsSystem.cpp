#include "PhysicsSystem.hpp"
#include "CollisionSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/Camera.hpp"
#include "Component/Collider.hpp"
#include "Component/Mesh.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
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

	void PhysicsSystem::integrate(const DeltaTime& p_delta_time)
	{
		m_update_count++;
		m_total_simulation_time += p_delta_time;

		auto& scene = m_scene_system.get_current_scene();
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
				// Recalculate the direction and rotation mat
				transform.m_direction = glm::normalize(transform.m_orientation * Component::Transform::Starting_Forward_Direction);
				transform.m_roll_pitch_yaw = glm::degrees(Utility::to_roll_pitch_yaw(transform.m_orientation));
			}

			const auto rotationMatrix = glm::mat4_cast(transform.m_orientation);

			transform.m_model = glm::translate(glm::identity<glm::mat4>(), transform.m_position);
			transform.m_model *= rotationMatrix;
			transform.m_model = glm::scale(transform.m_model, transform.m_scale);


			// Update the collider to match the new world-space position
			if (scene.has_components<Component::Collider>(entity) && scene.has_components<Component::Mesh>(entity))
			{
				auto& collider = scene.get_component<Component::Collider>(entity);
				auto& mesh     = scene.get_component<Component::Mesh>(entity);

				// Update the collider's world-space AABB
				collider.m_world_AABB = Geometry::AABB::transform(mesh.m_mesh->AABB, transform.m_position, rotationMatrix, transform.m_scale);

				collider.m_collision_shapes.clear();
				for (const auto& shape : mesh.m_mesh->collision_shapes) // No std::visit here because some shapes require the scale.
				{
					if (shape.is<Geometry::Cone>())
					{
						auto cone = shape.get<Geometry::Cone>();
						cone.transform(transform.m_model, transform.m_scale);
						collider.m_collision_shapes.emplace_back(cone);
					}
					else if (shape.is<Geometry::Cuboid>())
					{
						auto cuboid = shape.get<Geometry::Cuboid>();
						cuboid.transform(transform.m_position, transform.m_orientation, transform.m_scale);
						collider.m_collision_shapes.emplace_back(cuboid);
					}
					else if (shape.is<Geometry::Cylinder>())
					{
						auto cylinder = shape.get<Geometry::Cylinder>();
						cylinder.transform(transform.m_model, transform.m_scale);
						collider.m_collision_shapes.emplace_back(cylinder);
					}
					else if (shape.is<Geometry::Quad>())
					{
						auto quad = shape.get<Geometry::Quad>();
						quad.transform(transform.m_model);
						collider.m_collision_shapes.emplace_back(quad);
					}
					else if (shape.is<Geometry::Sphere>())
					{
						auto sphere = shape.get<Geometry::Sphere>();
						sphere.transform(transform.m_model, transform.m_scale);
						collider.m_collision_shapes.emplace_back(sphere);
					}
					else if (shape.is<Geometry::Triangle>())
					{
						auto triangle = shape.get<Geometry::Triangle>();
						triangle.transform(transform.m_model);
						collider.m_collision_shapes.emplace_back(triangle);
					}
					else
						ASSERT_THROW(false, "[DEBUG RENDERER] Unknown shape type for showing collision shape.");
				}

				collider.m_triangles.clear();
				for (const auto& triangle : mesh.m_mesh->triangles)
				{
					auto transformed_triangle = triangle;
					transformed_triangle.transform(transform.m_model);
					collider.m_triangles.emplace_back(transformed_triangle);
				}
			}

			// After moving and updating the Collider, check for collisions and respond
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
						auto& rigid_body_2 = m_scene_system.get_current_scene().get_component<Component::RigidBody>(collided_entity);
						auto& transform_2  = m_scene_system.get_current_scene().get_component<Component::Transform>(collided_entity);

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