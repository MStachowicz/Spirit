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

    std::optional<Geometry::Collision> CollisionSystem::getCollision(const ECS::Entity& pEntity, const Component::Transform& pTransform, const Component::Collider& pCollider) const
    {
        std::optional<Geometry::Collision> collision;

        auto& currentScene = mSceneSystem.getCurrentScene();
        currentScene.foreach([&](const ECS::Entity& pEntityOther, Component::Transform& pTransformOther, Component::Collider& pColliderOther)
        {
            if (&pCollider != &pColliderOther)
            {
                if (Geometry::intersect(pCollider.mWorldAABB, pColliderOther.mWorldAABB)) // Quick cull AABB check
                {
                    // If the AABBs of the geometries are found to collide but also have Mesh components, then they can be tested with a higher accuracy
                    // using mesh-aware collision testing.

                    // Go through both mesh trees of both models and check every combination triangle for triangle.
                    // The points are in object space so additionally transform the triangles before checking them.
                    if (currentScene.hasComponents<Component::Mesh>(pEntity) && currentScene.hasComponents<Component::Mesh>(pEntityOther))
                    {
                        auto& composite      = currentScene.getComponent<Component::Mesh>(pEntity).mModel->mCompositeMesh;
                        const auto& model    = pTransform.mModel;

                        auto& compositeOther = currentScene.getComponent<Component::Mesh>(pEntityOther).mModel->mCompositeMesh;
                        const auto& modelOther    = pTransform.mModel;

                        composite.forEachMesh([this, &compositeOther, &composite, &collision, &model, &modelOther](const Data::Mesh& pMesh)
                        {
                            compositeOther.forEachMesh([this, &pMesh, &collision, &model, &modelOther](const Data::Mesh& pMeshOther)
                            {
                                for (auto triangle : pMesh.mTriangles) // #Perf Copying mesh triangle here to be able to transform it.
                                {
                                    triangle.transform(model);

                                    for (auto triangleOther : pMeshOther.mTriangles) // #Perf Copying mesh triangle here to be able to transform it.
                                    {
                                        triangleOther.transform(modelOther);

                                        if (Geometry::intersect_triangle_triangle_static(triangle, triangleOther))
                                        {
                                            collision = std::make_optional<Geometry::Collision>();
                                            return;
                                            throw std::logic_error("Have to return collision point and normal here.");
                                        }
                                    }
                                }
                            });

                            if (collision.has_value())
                                return;
                        });

                        if (collision.has_value())
                            return;
                    }
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