#pragma once

#include "BoundingBox.hpp"

#include <vector>

namespace ECS
{
    class Entity;
}

namespace Collision
{
    class BoundingBoxNode
    {
        BoundingBox mBoundingBox;
        size_t mParentIndex;
        size_t mNextChild;
    };
    class BoundingBoxTree
    {
       private:
        std::vector<BoundingBoxNode> mNodes;

       public:
        void add(const ECS::Entity& pEntity, const BoundingBox& pBoundingBox);
        void remove(const ECS::Entity& pEntity);
    };
} // namespace Collision