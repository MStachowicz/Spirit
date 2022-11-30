#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    class Sphere
    {
    public:
        Sphere(const glm::vec3& pPosition, const float& pRadius)
            : mCenter{pPosition}
            , mRadius{pRadius}
        {}

        glm::vec3 mCenter;
        float mRadius;
    }
}