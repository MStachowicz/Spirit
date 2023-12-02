#pragma once

#include "ECS/Storage.hpp"
#include "Geometry/Intersect.hpp"

#include "glm/fwd.hpp"

#include <optional>
#include <vector>
#include <utility>

namespace Geometry
{
	class Ray;
}
namespace Component
{
	struct Transform;
}
namespace System
{
	class SceneSystem;

	// An optimisation layer and helper for quickly finding collision information for an Entity in a scene.
	class CollisionSystem
	{
	private:
		SceneSystem& m_scene_system;

	public:
		CollisionSystem(SceneSystem& p_scene_system) noexcept;

		// Returns the collision shape of p_entity in world space.
		std::optional<Geometry::ContactPoint> get_collision(const ECS::Entity& p_entity, const ECS::Entity* p_collided_entity = nullptr) const;

		// Does this ray collide with any entities.
		bool castRay(const Geometry::Ray& p_ray, glm::vec3& out_first_intersection) const;
		// Returns all the entities colliding with p_ray. These are returned as pairs of Entity and the length along the ray from the Ray origin.
		std::vector<std::pair<ECS::Entity, float>> get_entities_along_ray(const Geometry::Ray& p_ray) const;
	};
} // namespace System