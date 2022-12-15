#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    // A Ray is a line with a startpoint and no defined endpoint. It extends infinitely in one direction.
    struct Ray
    {
        Ray(const glm::vec3& pStart, const glm::vec3& pDirection)
            : mStart(pStart)
            , mDirection(pDirection)
        {}

        glm::vec3 mStart;
        glm::vec3 mDirection;
    };
}