#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    // A Ray is a line with a startpoint and no defined endpoint. It extends infinitely in one direction.
    class Ray
    {
    public:
        Ray(const glm::vec3& p_start, const glm::vec3& p_direction)
            : m_start(p_start)
            , m_direction(p_direction)
        {}

        glm::vec3 m_start;
        glm::vec3 m_direction;
    };
}