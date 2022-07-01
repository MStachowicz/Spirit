#pragma once

#include "glm/vec3.hpp"

namespace Data
{
    struct PointLight
    {
        glm::vec3 mPosition      = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mColour        = glm::vec3(1.f, 1.f, 1.f);
        float mAmbientIntensity  = 0.05f;
        float mDiffuseIntensity  = 0.8f;
        float mSpecularIntensity = 1.0f;

        float mConstant          = 1.f;
        float mLinear            = 0.09f;
        float mQuadratic         = 0.032f;

        void DrawImGui();
    };
}