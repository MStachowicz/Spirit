#include "CollisionSystem.hpp"

// ECS
#include "Storage.hpp"

// System
#include "MeshSystem.hpp"
#include "SceneSystem.hpp"

// Component
#include "Transform.hpp"
#include "Collider.hpp"

// Geometry
#include "Ray.hpp"

namespace System
{
    CollisionSystem::CollisionSystem(SceneSystem& pSceneSystem, const MeshSystem& pMeshSystem)
        : mSceneSystem{pSceneSystem}
        , mMeshSystem{pMeshSystem}
    {}

    std::optional<Geometry::Collision> CollisionSystem::getCollision(const Component::Transform& pTransform, const Component::Collider& pCollider) const
    {
        std::optional<Geometry::Collision> collision;

        mSceneSystem.getCurrentScene().foreach([&](Component::Transform& pTransformOther, Component::Collider& pColliderOther)
        {
            if (&pCollider != &pColliderOther)
            {
                if (Geometry::intersect(pCollider.mWorldAABB, pColliderOther.mWorldAABB)) // Quick cull AABB check
                {
                    // Now test at with higher accuracy
                    collision = Geometry::Collision();
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
            glm::vec3 collisionPoint;
            float lengthAlongRay;
            if (Geometry::intersect(pCollider.mWorldAABB, pRay, &collisionPoint, &lengthAlongRay))
            {
                pCollider.mCollided = true;

                if (!nearestIntersectionAlongRay.has_value() || lengthAlongRay < nearestIntersectionAlongRay)
                {
                    nearestIntersectionAlongRay = lengthAlongRay;
                    outFirstIntersection = collisionPoint;
                }
            }
        });

        return nearestIntersectionAlongRay.has_value();
    }

    std::vector<std::pair<ECS::EntityID, float>> CollisionSystem::getEntitiesAlongRay(const Geometry::Ray& pRay) const
    {
        std::vector<std::pair<ECS::EntityID, float>> entitiesAndDistance;

        mSceneSystem.getCurrentScene().foreach([&](ECS::EntityID& pEntity, Component::Collider& pCollider)
        {
            glm::vec3 collisionPoint;
            float lengthAlongRay;
            if (Geometry::intersect(pCollider.mWorldAABB, pRay, &collisionPoint, &lengthAlongRay))
                entitiesAndDistance.push_back({pEntity, lengthAlongRay});
        });

        return entitiesAndDistance;
    }
} // namespace System