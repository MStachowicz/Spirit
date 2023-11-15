#include "CollisionSystem.hpp"
#include "Component/Collider.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "System/MeshSystem.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/Ray.hpp"
#include "Geometry/Triangle.hpp"

namespace System
{
	CollisionSystem::CollisionSystem(SceneSystem& pSceneSystem, const MeshSystem& pMeshSystem)
		: mSceneSystem{pSceneSystem}
		, mMeshSystem{pMeshSystem}
	{}

	std::optional<Collision> CollisionSystem::getCollision(const ECS::Entity& pEntity, const Component::Transform& pTransform, const Component::Collider& pCollider) const
	{
		std::optional<Collision> collision;

		auto& currentScene = mSceneSystem.getCurrentScene();
		currentScene.foreach([&](const ECS::Entity& pEntityOther, Component::Transform& pTransformOther, Component::Collider& pColliderOther)
		{
			if (&pCollider != &pColliderOther)
			{
				if (Geometry::intersecting(pCollider.m_world_AABB, pColliderOther.m_world_AABB)) // Quick cull AABB check
				{
					collision = std::make_optional<Collision>({glm::vec3(0.f),glm::vec3(0.f), pEntityOther });
					return;
				}
			}
		});

		return collision;
	}

	bool CollisionSystem::castRay(const Geometry::Ray& pRay, glm::vec3& outFirstIntersection) const
	{
		std::optional<float> nearestIntersectionAlongRay;

		mSceneSystem.getCurrentScene().foreach([&](Component::Collider& pCollider)
		{
			if (auto line_intersection = Geometry::get_intersection(pCollider.m_world_AABB, pRay))
			{
				pCollider.m_collided = true;

				if (!nearestIntersectionAlongRay.has_value() || line_intersection->length_along_ray < nearestIntersectionAlongRay)
				{
					nearestIntersectionAlongRay = line_intersection->length_along_ray;
					outFirstIntersection        = line_intersection->intersection_point;
				}
			}
		});

		return nearestIntersectionAlongRay.has_value();
	}

	std::vector<std::pair<ECS::Entity, float>> CollisionSystem::getEntitiesAlongRay(const Geometry::Ray& pRay) const
	{
		std::vector<std::pair<ECS::Entity, float>> entitiesAndDistance;

		mSceneSystem.getCurrentScene().foreach([&](ECS::Entity& pEntity, Component::Collider& pCollider)
		{
			if (auto intersection = Geometry::get_intersection(pCollider.m_world_AABB, pRay))
				entitiesAndDistance.push_back({pEntity, intersection->length_along_ray});
		});

		return entitiesAndDistance;
	}
} // namespace System