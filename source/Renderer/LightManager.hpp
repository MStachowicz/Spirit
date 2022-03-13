#pragma once

#include "ComponentManager.hpp"
#include "Entity.hpp"

#include "glm/vec3.hpp"

// A point light emits light in all directions from a position.
// 1-1 relationship with light properties in shaders.
struct PointLight
{
    glm::vec3 mColour    = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 mPosition  = glm::vec3(1.0f, 1.0f, 1.0f);

    float mAmbientIntensity   = 0.2f;
    float mDiffuseIntensity   = 0.5f;
    float mSpecularIntensity  = 1.0f;
};

class LightManager
{
    public:
        LightManager()
        {
            mPointLights.Create(ECS::CreateEntity());
        }

        void outputImGui();

        const ECS::ComponentManager<PointLight>& getPointLights() const
        { return mPointLights; };

        bool mRenderLightPositions = true;

    private:
        ECS::ComponentManager<PointLight> mPointLights;
};