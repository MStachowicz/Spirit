#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    struct Triangle
    {
        Triangle(const glm::vec3& pPoint1, const glm::vec3& pPoint2, const glm::vec3& pPoint3);

        glm::vec3 mPoint1;
        glm::vec3 mPoint2;
        glm::vec3 mPoint3;
    };
} // namespace Geometry