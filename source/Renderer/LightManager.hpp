#pragma once

#include "Logger.hpp"
#include "glm/vec3.hpp"
#include "ComponentManager.hpp"
#include "Entity.hpp"

#include "vector"

// A point light emits light in all directions from a position.
// 1-1 relationship with light properties in shaders.
struct PointLight
{
    glm::vec3 mColour    = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 mPosition  = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 mAmbient   = glm::vec3(0.2f);
    glm::vec3 mDiffuse   = glm::vec3(0.5f);
    glm::vec3 mSpecular  = glm::vec3(0.0f, 1.0f, 1.0f);
};

class LightManager
{
    public:
        LightManager()
        {
            mPointLights.Create(ECS::CreateEntity());
        }

        const ECS::ComponentManager<PointLight>& getPointLights() const
        { return mPointLights; };

    private:
        ECS::ComponentManager<PointLight> mPointLights;
};