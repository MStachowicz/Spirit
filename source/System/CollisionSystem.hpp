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


	// ContactPoint encapsulates the information about a point of contact between two shapes.
	// A displacement applied along the normal by the penetration_depth separates the two shapes.
	struct ContactPoint
	{
		glm::vec3 position      = glm::vec3(0.f); // The point of contact on the surface of shape A.
		glm::vec3 normal        = glm::vec3(0.f); // The collision response normal of the contact point from the perspective of shape A (normalised).
		float penetration_depth = 0.f;            // The depth of overlap. Unsigned displacement required to separate the two shapes along normal.
	};

	// An optimisation layer and helper for quickly finding collision information for an Entity in a scene.
	class CollisionSystem
	{
	private:
		SceneSystem& m_scene_system;

	public:
		CollisionSystem(SceneSystem& p_scene_system) noexcept;

		std::optional<ContactPoint> get_collision(const ECS::Entity& p_entity, const ECS::Entity* p_collided_entity = nullptr) const;

		// Does this ray collide with any entities.
		bool castRay(const Geometry::Ray& p_ray, glm::vec3& out_first_intersection) const;
		// Returns all the entities colliding with p_ray. These are returned as pairs of Entity and the length along the ray from the Ray origin.
		std::vector<std::pair<ECS::Entity, float>> get_entities_along_ray(const Geometry::Ray& p_ray) const;
	};
} // namespace System