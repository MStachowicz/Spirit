#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

#include "Storage.hpp"

#include "Intersect.hpp"

#include <optional>
#include <vector>

namespace Geometry
{
    class Ray;
}
namespace Component
{
    struct Transform;
    struct Collider;
}

namespace System
{
    class MeshSystem;
    class SceneSystem;

    // Information about the point of contact between objects.
    struct Collision
    {
        glm::vec3 mPoint;
        glm::vec3 mNormal;
        ECS::Entity mEntity;
    };

    // An optimisation layer and helper for quickly finding collision information for an Entity in a scene.
    // A client of the Geometry library.
    class CollisionSystem
    {
    private:
        const MeshSystem& mMeshSystem;
        SceneSystem& mSceneSystem;

    public:
        CollisionSystem(SceneSystem& pSceneSystem, const MeshSystem& pMeshSystem);

        // Return the first (if-any) Collision found for pEntity.
        // If a Collision is returned it is guranteed the Entity collided with has a Collider and Transform component of its own.
        std::optional<Collision> getCollision(const ECS::Entity& pEntity, const Component::Transform& pTransform, const Component::Collider& pCollider) const;
        // Does this ray collide with any entities.
        bool castRay(const Geometry::Ray& pRay, glm::vec3& outFirstIntersection) const;
        // Returns all the entities colliding with pRay. These are returned as pairs of Entity and the length along the ray from the Ray origin.
        std::vector<std::pair<ECS::Entity, float>> getEntitiesAlongRay(const Geometry::Ray& pRay) const;
    };
} // namespace System