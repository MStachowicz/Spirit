#pragma once

#include "glm/vec3.hpp"

namespace Component
{
    class DirectionalLight
    {
    public:
        DirectionalLight() noexcept;
        DirectionalLight(float p_ambient_intensity, float m_diffuse_intensity) noexcept;

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
        PointLight(const glm::vec3& p_position) noexcept;

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