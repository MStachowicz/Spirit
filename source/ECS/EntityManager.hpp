#pragma once

#include "Entity.hpp"
#include "ComponentManager.hpp"

#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "DirectionalLight.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"

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
        EntityManager() : mNextEntityID(0)
        {}

        Entity& CreateEntity()
        {
            mEntities.push_back({++mNextEntityID});
            mEntityCreatedEvent.Dispatch(mEntities.back(), *this);
            return mEntities.back();
        }

        void ForEach(const std::function<void(const Entity& pEntity)>& pFunction) const
        {
            for (const auto& entity : mEntities)
                pFunction(entity);
        }

        Utility::EventDispatcher<const Entity&, const EntityManager&> mEntityCreatedEvent;
        Utility::EventDispatcher<const Entity&, const EntityManager&> mEntityRemovedEvent;

        ComponentManager<Data::PointLight>       mPointLights;
        ComponentManager<Data::SpotLight>        mSpotLights;
        ComponentManager<Data::DirectionalLight> mDirectionalLights;
        ComponentManager<Data::Transform>        mTransforms;
        ComponentManager<Data::MeshDraw>         mMeshes;

        void DrawImGui();
    private:
        std::vector<Entity> mEntities;
        EntityID mNextEntityID;
    };
}