#include "CollisionSystem.hpp"
#include "Component/Collider.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "System/MeshSystem.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/Point.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Triangle.hpp"

namespace System
{
	CollisionSystem::CollisionSystem(SceneSystem& p_scene_system) noexcept
		: m_scene_system{p_scene_system}
	{}

	std::optional<Geometry::ContactPoint> CollisionSystem::get_collision(const ECS::Entity& p_entity, ECS::Entity* p_collided_entity) const
	{
		auto& scene = m_scene_system.getCurrentScene();
		if (scene.hasComponents<Component::Collider, Component::Mesh, Component::Transform>(p_entity))
		{
			auto& collider  = scene.getComponentMutable<Component::Collider>(p_entity);
			auto& mesh      = scene.getComponentMutable<Component::Mesh>(p_entity);
			auto& transform = scene.getComponentMutable<Component::Transform>(p_entity);

			const auto rotation_matrix = glm::mat4_cast(transform.mOrientation);
			collider.m_world_AABB = Geometry::AABB::transform(mesh.m_mesh->AABB, transform.mPosition, rotation_matrix, transform.mScale);
			collider.m_collided   = false;

			std::optional<Geometry::ContactPoint> collision_shape = std::nullopt;
			scene.foreach([&](const ECS::Entity& p_entity_other, Component::Transform& p_transform_other, Component::Mesh& p_mesh_other, Component::Collider& p_collider_other)
			{
				if (&collider != &p_collider_other)
				{
					const auto rotation_matrix_other = glm::mat4_cast(p_transform_other.mOrientation);
					p_collider_other.m_world_AABB    = Geometry::AABB::transform(mesh.m_mesh->AABB, p_transform_other.mPosition, rotation_matrix_other, p_transform_other.mScale);

					if (Geometry::intersecting(collider.m_world_AABB, p_collider_other.m_world_AABB)) // Broad phase AABB check
					{
						collider.m_collided = true;
						return;
					}
				}
			});
		}

		return std::nullopt;
	}

	bool CollisionSystem::castRay(const Geometry::Ray& pRay, glm::vec3& outFirstIntersection) const
	{
		std::optional<float> nearestIntersectionAlongRay;

		m_scene_system.getCurrentScene().foreach([&](Component::Collider& pCollider)
		{
			float length_along_ray = 0.f;
			if (auto intersection = Geometry::get_intersection(pCollider.m_world_AABB, pRay, &length_along_ray))
			{
				pCollider.m_collided = true;

				if (!nearestIntersectionAlongRay.has_value() || length_along_ray < nearestIntersectionAlongRay)
				{
					nearestIntersectionAlongRay = length_along_ray;
					outFirstIntersection        = intersection->position;
				}
			}
		});

		return nearestIntersectionAlongRay.has_value();
	}

	std::vector<std::pair<ECS::Entity, float>> CollisionSystem::get_entities_along_ray(const Geometry::Ray& pRay) const
	{
		std::vector<std::pair<ECS::Entity, float>> entitiesAndDistance;

		m_scene_system.getCurrentScene().foreach([&](ECS::Entity& pEntity, Component::Collider& pCollider)
		{
			float length_along_ray = 0.f;
			if (auto intersection = Geometry::get_intersection(pCollider.m_world_AABB, pRay, &length_along_ray))
				entitiesAndDistance.push_back({pEntity, length_along_ray});
		});

		return entitiesAndDistance;
	}
} // namespace System