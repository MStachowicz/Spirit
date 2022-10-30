#pragma once

#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"

namespace Component
{
    struct SpotLight
    {
        glm::vec3 mPosition       = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mDirection      = glm::vec3(0.f, 0.f, -1.f);
        glm::vec3 mColour         = glm::vec3(1.f, 1.f, 1.f);
        float mAmbientIntensity   = 0.0f;
        float mDiffuseIntensity   = 1.0f;
        float mSpecularIntensity  = 1.0f;

        float mConstant           = 1.f;
        float mLinear             = 0.09f;
        float mQuadratic          = 0.032f;

        float mCutOff             = glm::cos(glm::radians(12.5f));
        float mOuterCutOff        = glm::cos(glm::radians(15.0f));

        void DrawImGui();

        bool operator== (const SpotLight& pOther) const
        {
            return mPosition == pOther.mPosition &&
                mDirection == pOther.mDirection &&
                mColour == pOther.mColour &&
                mAmbientIntensity == pOther.mAmbientIntensity &&
                mDiffuseIntensity == pOther.mDiffuseIntensity &&
                mSpecularIntensity == pOther.mSpecularIntensity &&
                mConstant == pOther.mConstant &&
                mLinear == pOther.mLinear &&
                mQuadratic == pOther.mQuadratic &&
                mCutOff == pOther.mCutOff &&
                mOuterCutOff == pOther.mOuterCutOff;
        }
        bool operator != (const SpotLight& pOther) const { return !(*this == pOther); }
    };
}