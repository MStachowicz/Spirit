#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

#include <array>

namespace Geometry
{
    class Triangle
    {
    public:
        glm::vec3 m_point_1;
        glm::vec3 m_point_2;
        glm::vec3 m_point_3;

        constexpr Triangle() noexcept
            : m_point_1{0.0f}
            , m_point_2{0.0f}
            , m_point_3{0.0f}
        {}
        constexpr Triangle(const glm::vec3& p_point_1, const glm::vec3& p_point_2, const glm::vec3& p_point_3) noexcept
            : m_point_1(p_point_1)
            , m_point_2(p_point_2)
            , m_point_3(p_point_3)
        {}
        constexpr bool operator==(const Triangle& p_other) const { return m_point_1 == p_other.m_point_1 && m_point_2 == p_other.m_point_2 && m_point_3 == p_other.m_point_3; }

        // Transform all the points in the triangle by the trasnformation matrix applying around the centroid of the triangle.
        void transform(const glm::mat4& p_transform);
        void translate(const glm::vec3& p_translate);

        // Returns the current world-space centroid of the triangle.
        glm::vec3 centroid() const;
        // Get the normalised direction vector representing the normal of the triangle.
        glm::vec3 normal() const;
        // Subdivide this triangle into 4 new triangles contained inside the original.
        std::array<Triangle, 4> subdivide() const;
    };
} // namespace Geometry