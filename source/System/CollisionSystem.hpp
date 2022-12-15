#pragma once

#include "glm/fwd.hpp"

namespace ECS
{
    typedef size_t EntityID;
}
namespace Geometry
{
    struct Ray;
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
        void checkCollisions();

        bool castRay(const Geometry::Ray& pRay, glm::vec3& outFirstIntersection) const;
        // Returns all the entities colliding with pRay.
        std::vector<std::pair<ECS::EntityID, float>> getEntitiesAlongRay(const Geometry::Ray& pRay) const;
    };
} // namespace System