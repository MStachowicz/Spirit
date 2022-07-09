#pragma once

#include "glm/vec3.hpp"

namespace Data
{
    struct Transform
    {
        glm::vec3 mPosition = glm::vec3(0.0f);
        glm::vec3 mRotation = glm::vec3(0.0f);
        glm::vec3 mScale    = glm::vec3(1.0f);

        void DrawImGui();
        bool operator==(const Transform& pOther) const
        {
            return mPosition == pOther.mPosition &&
                mRotation == pOther.mRotation &&
                mScale == pOther.mScale;
        }
        bool operator != (const Transform& pOther) const { return !(*this == pOther); }
    };
}