#pragma once

#include "glm/vec3.hpp"

namespace Component
{
    class DirectionalLight
    {
    public:
        DirectionalLight() noexcept;

        glm::vec3 mDirection;
        glm::vec3 mColour;
        float mAmbientIntensity;
        float mDiffuseIntensity;
        float mSpecularIntensity;

        void DrawImGui();
    };

    class PointLight
    {
    public:
        PointLight() noexcept;

        glm::vec3 mPosition;
        glm::vec3 mColour;

        float mAmbientIntensity;
        float mDiffuseIntensity;
        float mSpecularIntensity;

        float mConstant;
        float mLinear;
        float mQuadratic;

        void DrawImGui();
    };

    class SpotLight
    {
    public:
        SpotLight() noexcept;

        glm::vec3 mPosition;
        glm::vec3 mDirection;
        glm::vec3 mColour;
        float mAmbientIntensity;
        float mDiffuseIntensity;
        float mSpecularIntensity;

        float mConstant;
        float mLinear;
        float mQuadratic;

        float mCutOff;
        float mOuterCutOff;

        void DrawImGui();
    };
}