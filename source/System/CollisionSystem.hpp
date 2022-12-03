#pragma once

#include "glm/fwd.hpp"

namespace ECS
{
    class Storage;
}
namespace System
{
    class MeshSystem;
}
namespace Geometry
{
    struct Ray;
}

namespace System
{
    class CollisionSystem
    {
       private:
        ECS::Storage& mStorage;
        const System::MeshSystem& mMeshSystem;

       public:
        CollisionSystem(ECS::Storage& pStorage, const System::MeshSystem& pMeshSystem);
        void checkCollisions();

        bool castRay(const Geometry::Ray& pRay, glm::vec3& outFirstIntersection) const;

    };
} // namespace System