#pragma once

#include "glm/vec3.hpp"

namespace Component
{
    class Collider
    {
       public:

        //bool operator== (const Collider& pOther)  const { return mBoundingBox == pOther.mBoundingBox; };
        //bool operator != (const Collider& pOther) const { return !(*this == pOther); }
        void DrawImGui();
    };
}