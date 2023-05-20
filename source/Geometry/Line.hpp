#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    // A directed line segment. A finite section of a geometric line from m_start to m_end.
    class Line
    {
    public:
        constexpr Line(const glm::vec3& p_start, const glm::vec3& p_end) noexcept
            : m_start{p_start}
            , m_end{p_end}
        {}

        glm::vec3 m_start;
        glm::vec3 m_end;
    };
}