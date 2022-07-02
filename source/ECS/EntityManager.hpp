#pragma once

#include "Entity.hpp"
#include "ComponentManager.hpp"

#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "DirectionalLight.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"

#include <vector>

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
        inline void ForEach(const std::function<void(const Entity& pEntity)>& pFunction) const
        {
            for (size_t i = 0; i < mEntities.size(); i++)
            {
                pFunction(mEntities[i]);
            }
        }

        ComponentManager<Data::PointLight>       mPointLights;
        ComponentManager<Data::SpotLight>        mSpotLights;
        ComponentManager<Data::DirectionalLight> mDirectionalLights;
        ComponentManager<Data::Transform>        mTransforms;
        ComponentManager<Data::MeshDraw>         mMeshes;

        void DrawImGui() {};
    private:
        std::vector<Entity> mEntities;
        EntityID mNextEntityID;
    };
}