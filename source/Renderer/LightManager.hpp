#pragma once

#include "ComponentManager.hpp"
#include "Light.hpp"

class LightManager
{
    public:
        LightManager();
        void outputImGui();

        const ECS::ComponentManager<DirectionalLight>& getDirectionalLights() const { return mDirectionalLights; };
        const ECS::ComponentManager<PointLight>& getPointLights() const             { return mPointLights; };
        const ECS::ComponentManager<SpotLight>& getSpotlightsLights() const         { return mSpotLights; };
        bool mRenderLightPositions = true;

    private:
        ECS::ComponentManager<DirectionalLight> mDirectionalLights;
        ECS::ComponentManager<PointLight> mPointLights;
        ECS::ComponentManager<SpotLight> mSpotLights;
};