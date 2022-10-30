#include "CollisionSystem.hpp"

#include "ComponentManager.hpp"

namespace Collision
{
    CollisionSystem::CollisionSystem(ECS::ComponentManager<Component::Collider>& pColliders)
        : mColliders{pColliders}
    {
        mColliders.mComponentAddedEvent.Subscribe(std::bind(&CollisionSystem::onCollisionComponentAdded, this, std::placeholders::_1, std::placeholders::_2));
        mColliders.mComponentChangedEvent.Subscribe(std::bind(&CollisionSystem::onCollisionComponentChanged, this, std::placeholders::_1, std::placeholders::_2));
        mColliders.mComponentRemovedEvent.Subscribe(std::bind(&CollisionSystem::onCollisionComponentRemoved, this, std::placeholders::_1));
    }

    void CollisionSystem::onCollisionComponentAdded(const ECS::Entity& pEntity, const Component::Collider& pCollider)
    {
    }
    void CollisionSystem::onCollisionComponentChanged(const ECS::Entity& pEntity, const Component::Collider& pCollider)
    {
    }
    void CollisionSystem::onCollisionComponentRemoved(const ECS::Entity& pEntity)
    {

    }


} // namespace Collision