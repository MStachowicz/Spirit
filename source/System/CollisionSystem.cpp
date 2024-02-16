#include "CollisionSystem.hpp"
#include "MeshSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/Collider.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"

#include "Geometry/Point.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Triangle.hpp"

namespace System
{
	CollisionSystem::CollisionSystem(SceneSystem& p_scene_system) noexcept
		: m_scene_system{p_scene_system}
	{}

	std::optional<ContactPoint> CollisionSystem::get_collision(const ECS::Entity& p_entity, const ECS::Entity* p_collided_entity) const
	{
		auto& scene = m_scene_system.get_current_scene();
		if (scene.has_components<Component::Collider, Component::Mesh, Component::Transform>(p_entity))
		{
			auto& collider  = scene.get_component<Component::Collider>(p_entity);
			auto& mesh      = scene.get_component<Component::Mesh>(p_entity);
			auto& transform = scene.get_component<Component::Transform>(p_entity);

			const auto rotation_matrix = glm::mat4_cast(transform.m_orientation);
			collider.m_world_AABB      = Geometry::AABB::transform(mesh.m_mesh->AABB, transform.m_position, rotation_matrix, transform.m_scale);
			collider.m_collided        = false;

			scene.foreach([&](const ECS::Entity& p_entity_other, Component::Transform& p_transform_other, Component::Mesh& p_mesh_other, Component::Collider& p_collider_other)
			{(void)p_mesh_other;
				if (&collider != &p_collider_other)
				{
					const auto rotation_matrix_other = glm::mat4_cast(p_transform_other.m_orientation);
					p_collider_other.m_world_AABB    = Geometry::AABB::transform(mesh.m_mesh->AABB, p_transform_other.m_position, rotation_matrix_other, p_transform_other.m_scale);

					if (Geometry::intersecting(collider.m_world_AABB, p_collider_other.m_world_AABB)) // Broad phase AABB check
					{
						p_collided_entity = &p_entity_other;
						collider.m_collided = true;
						return;
					}
				}
			});
		}

		return std::nullopt;
	}

	bool CollisionSystem::castRay(const Geometry::Ray& p_ray, glm::vec3& out_first_intersection) const
	{
		std::optional<float> min_intersection_along_ray;

		m_scene_system.get_current_scene().foreach([&](Component::Collider& p_collider)
		{
			float length_along_ray = 0.f;
			if (auto intersection = Geometry::get_intersection(p_collider.m_world_AABB, p_ray, &length_along_ray))
			{
				p_collider.m_collided = true;

				if (!min_intersection_along_ray.has_value() || length_along_ray < min_intersection_along_ray)
				{
					min_intersection_along_ray = length_along_ray;
					out_first_intersection     = *intersection;
				}
			}
		});

		return min_intersection_along_ray.has_value();
	}

	std::vector<std::pair<ECS::Entity, float>> CollisionSystem::get_entities_along_ray(const Geometry::Ray& p_ray) const
	{
		std::vector<std::pair<ECS::Entity, float>> entities_and_distance;

		m_scene_system.get_current_scene().foreach([&](ECS::Entity& p_entity, Component::Collider& p_collider)
		{
			float length_along_ray = 0.f;
			if (auto intersection = Geometry::get_intersection(p_collider.m_world_AABB, p_ray, &length_along_ray))
				entities_and_distance.push_back({p_entity, length_along_ray});
		});

		return entities_and_distance;
	}
} // namespace System