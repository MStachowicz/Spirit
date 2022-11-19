#pragma once

#include "BoundingBoxTree.hpp"

namespace ECS
{
    class Storage;
}

namespace Collision
{
    class CollisionSystem
    {
       private:
        BoundingBoxTree mBoundingBoxTree;
        ECS::Storage& mStorage;

       public:
        CollisionSystem(ECS::Storage& pStorage);
    };
} // namespace Collision