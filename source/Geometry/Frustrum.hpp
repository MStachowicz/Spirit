#pragma once

#include "Plane.hpp"

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace Geometry
{
    // Frustrum represnts a portion of space bounded by 6 planes.
    // By convention the plane normal's point inside the bounded volume/frustrum.
    class Frustrum
    {
    public:
        Geometry::Plane m_left;
        Geometry::Plane m_right;
        Geometry::Plane m_bottom;
        Geometry::Plane m_top;
        Geometry::Plane m_near;
        Geometry::Plane m_far;

        // Construct a frustrum from a projection matrix. p_projection is expected to be using right-handed system.
        Frustrum(const glm::mat4& p_projection) noexcept;
    };
}