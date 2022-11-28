#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    // A Ray is a line with a startpoint and no defined endpoint. It extends infinitely in one direction.
    struct Ray
    {
        glm::vec3 mStart;
        glm::vec3 mDirection;
    };
}