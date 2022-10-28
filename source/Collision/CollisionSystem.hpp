#pragma once

#include "BoundingBoxTree.hpp"

namespace ECS
{
    template <class>
    class ComponentManager;
    class Entity;
}
namespace Data
{
    class Collider;
}

namespace Collision
{
    class CollisionSystem
    {
       private:
        ECS::ComponentManager<Data::Collider>& mColliders;
        BoundingBoxTree mBoundingBoxTree;

       public:
        CollisionSystem(ECS::ComponentManager<Data::Collider>& pColliders);

       private:
        void onCollisionComponentAdded(const ECS::Entity& pEntity, const Data::Collider& pCollider);
        void onCollisionComponentChanged(const ECS::Entity& pEntity, const Data::Collider& pCollider);
        void onCollisionComponentRemoved(const ECS::Entity& pEntity);
    };

} // namespace Collision