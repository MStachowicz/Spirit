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
    public:
        EntityManager() : mNextEntityID(0)
        {}

        Entity& CreateEntity()
        {
            mEntities.push_back({++mNextEntityID});
            return mEntities.back();
        }

        void ForEach(const std::function<void(const Entity& pEntity)>& pFunction) const
        {
            for (const auto& entity : mEntities)
                pFunction(entity);
        }

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