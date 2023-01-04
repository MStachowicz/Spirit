#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

#include "Intersect.hpp"

#include <optional>
#include <vector>

namespace ECS
{
    typedef size_t EntityID;
}
namespace Geometry
{
    struct Ray;
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

    class CollisionSystem
    {
    private:
        const MeshSystem& mMeshSystem;
        SceneSystem& mSceneSystem;

    public:
        CollisionSystem(SceneSystem& pSceneSystem, const MeshSystem& pMeshSystem);

        // For a given Transform and Mesh return the collision information
        std::optional<Geometry::Collision> getCollision(const Component::Transform& pTransform, const Component::Collider& pCollider) const;

        bool castRay(const Geometry::Ray& pRay, glm::vec3& outFirstIntersection) const;
        // Returns all the entities colliding with pRay.
        std::vector<std::pair<ECS::EntityID, float>> getEntitiesAlongRay(const Geometry::Ray& pRay) const;
    };
} // namespace System