#pragma once

#include "BoundingBoxTree.hpp"

namespace ECS
{
    template <class>
    class ComponentManager;
    class Entity;
}
namespace Component
{
    class Collider;
}

namespace Collision
{
    class CollisionSystem
    {
       private:
        ECS::ComponentManager<Component::Collider>& mColliders;
        BoundingBoxTree mBoundingBoxTree;

       public:
        CollisionSystem(ECS::ComponentManager<Component::Collider>& pColliders);

       private:
        void onCollisionComponentAdded(const ECS::Entity& pEntity, const Component::Collider& pCollider);
        void onCollisionComponentChanged(const ECS::Entity& pEntity, const Component::Collider& pCollider);
        void onCollisionComponentRemoved(const ECS::Entity& pEntity);
    };

} // namespace Collision