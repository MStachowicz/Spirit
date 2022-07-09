#pragma once

#include "glm/vec3.hpp"

namespace Data
{
    struct DirectionalLight
    {
        glm::vec3 mDirection     = glm::vec3(0.f, 0.f, 1.f);
        glm::vec3 mColour        = glm::vec3(1.f, 1.f, 1.f);
        float mAmbientIntensity  = 0.05f;
        float mDiffuseIntensity  = 0.15f;
        float mSpecularIntensity = 0.5f;

        bool operator== (const DirectionalLight& pOther) const
        {
            return mDirection == pOther.mDirection &&
                mColour == pOther.mColour &&
                mAmbientIntensity == pOther.mAmbientIntensity &&
                mDiffuseIntensity == pOther.mDiffuseIntensity &&
                mSpecularIntensity == pOther.mSpecularIntensity;
        };
        bool operator != (const DirectionalLight& pOther) const { return !(*this == pOther); }
        void DrawImGui();
    };
}