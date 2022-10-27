#pragma once

#include "glm/vec3.hpp"

#include "BoundingBox.hpp"

namespace Data
{
    struct Collider
    {
        Collision::BoundingBox mBoundingBox;

        bool operator== (const Collider& pOther)  const { return mBoundingBox == pOther.mBoundingBox; };
        bool operator != (const Collider& pOther) const { return !(*this == pOther); }
        void DrawImGui();
    };
}