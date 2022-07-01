#pragma once

#include "Entity.hpp"
#include "ComponentManager.hpp"

#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "DirectionalLight.hpp"

namespace ECS
{
    class EntityManager
    {
    public:
        EntityManager() : mNextEntity(0)
        {}

        Entity CreateEntity() { return ++mNextEntity; }

        ComponentManager<Data::PointLight> mPointLights;
        ComponentManager<Data::SpotLight> mSpotLights;
        ComponentManager<Data::DirectionalLight> mDirectionalLights;

    private:
        Entity mNextEntity;
    };
}