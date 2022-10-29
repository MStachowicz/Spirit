#pragma once

#include "Entity.hpp"
#include "ComponentManager.hpp"

#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "DirectionalLight.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"
#include "Collider.hpp"

#include <vector>
#include <functional>

namespace ECS
{
    class EntityManager
    {
        // The mediator for Entities and Components.
        // The EntityManager allows subscribing to events for adding and removing entities.
        // Changes to entities can be subscribed to via the specific ComponentManager members for the specific component type of interest.
    public:
        EntityManager()
            : mNextEntityID(0)
            , inactiveIDCount(0)
        {}

        Entity& CreateEntity()
        {
            mEntities.push_back({mNextEntityID++});
            mEntityCreatedEvent.Dispatch(mEntities.back(), *this);
            return mEntities.back();
        }
        void removeEntity(const Entity& pEntity)
        {
            // Set the Entity as inactive and remove all components owned by it.
            // There is no system in place to reuse the inactive entities so over time fragmentation occurs.
            // This removal preserves each Entitiy unique ID and position in the mEntities vec after removing.

            mEntities[pEntity.mID].mActive = false;
            inactiveIDCount++;

            mPointLights.Remove(pEntity.mID);
            mSpotLights.Remove(pEntity.mID);
            mDirectionalLights.Remove(pEntity.mID);
            mTransforms.Remove(pEntity.mID);
            mMeshes.Remove(pEntity.mID);

            mEntityRemovedEvent.Dispatch(pEntity.mID, *this);

            LOG_INFO("Entity removed ID:{}", pEntity.mID);
        }


        void ForEach(const std::function<void(const Entity& pEntity)>& pFunction) const
        {
            for (const auto& entity : mEntities)
                pFunction(entity);
        }

        Utility::EventDispatcher<const Entity&, const EntityManager&> mEntityCreatedEvent;
        Utility::EventDispatcher<const Entity&, const EntityManager&> mEntityRemovedEvent;

        // #Optimisation - Lay out components entity by entity instead of component lists. Data locality could be favoured if each entity component was side by side in memory
        ComponentManager<Data::PointLight>       mPointLights;
        ComponentManager<Data::SpotLight>        mSpotLights;
        ComponentManager<Data::DirectionalLight> mDirectionalLights;
        ComponentManager<Data::Transform>        mTransforms;
        ComponentManager<Data::MeshDraw>         mMeshes;
        ComponentManager<Data::Collider>         mColliders;

        void DrawImGui();
    private:
        std::vector<Entity> mEntities;
        EntityID mNextEntityID;
        size_t inactiveIDCount;
    };
}