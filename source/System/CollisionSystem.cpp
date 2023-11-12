#include "CollisionSystem.hpp"

// ECS
#include "Storage.hpp"

// System
#include "MeshSystem.hpp"
#include "SceneSystem.hpp"

// Component
#include "Transform.hpp"
#include "Collider.hpp"
#include "Mesh.hpp"

// Geometry
#include "Ray.hpp"
#include "Triangle.hpp"

#include "glm/gtx/string_cast.hpp"

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
                if (Geometry::intersect(pCollider.mWorldAABB, pColliderOther.mWorldAABB)) // Quick cull AABB check
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

    std::vector<std::pair<ECS::Entity, float>> CollisionSystem::getEntitiesAlongRay(const Geometry::Ray& pRay) const
    {
        std::vector<std::pair<ECS::Entity, float>> entitiesAndDistance;

        mSceneSystem.getCurrentScene().foreach([&](ECS::Entity& pEntity, Component::Collider& pCollider)
        {
            glm::vec3 collisionPoint;
            float lengthAlongRay;
            if (Geometry::intersect(pCollider.mWorldAABB, pRay, &collisionPoint, &lengthAlongRay))
                entitiesAndDistance.push_back({pEntity, lengthAlongRay});
        });

        return entitiesAndDistance;
    }
} // namespace System