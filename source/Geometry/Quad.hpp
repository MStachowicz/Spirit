#pragma once

#include "Triangle.hpp"

#include "glm/vec3.hpp"

#include <array>

namespace Geometry
{
    class Plane;

    // A quadrilateral. Four-sided polygon, having four edges (sides) and four corners (vertices).
    // Quad is a 2-dimensional shape.
    class Quad
    {
    public:
        glm::vec3 m_point_1;
        glm::vec3 m_point_2;
        glm::vec3 m_point_3;
        glm::vec3 m_point_4;

        constexpr Quad(const glm::vec3& p_point_1, const glm::vec3& p_point_2, const glm::vec3& p_point_3, const glm::vec3& p_point_4) noexcept
            : m_point_1{p_point_1}
            , m_point_2{p_point_2}
            , m_point_3{p_point_3}
            , m_point_4{p_point_4}
        {}
        // Construct a unit quad inside the plane.
        Quad(const Plane& p_plane) noexcept;

        // Uniformly scale the Quad by p_scale factor from its center.
        void scale(const float p_scale);

        glm::vec3 center() const;
        // Get a pair of triangles that represent this quad.
        std::array<Triangle, 2> get_triangles() const;
    };
}