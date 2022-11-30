#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    class Cylinder
    {
    public:
        Cylinder(const glm::vec3& pBase, const glm::vec3& pTop, const float& pDiameter)
        : mBase{pBase}
        , mTop{pTop}
        , mDiameter{pDiameter}
        {}

        glm::vec3 mBase;
        glm::vec3 mTop;
        float mDiameter;
    };
}