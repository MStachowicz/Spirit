#include "Quad.hpp"
#include "Plane.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
    Quad::Quad(const Plane& p_plane) noexcept
        : m_point_1{}
        , m_point_2{}
        , m_point_3{}
        , m_point_4{}
    {
        glm::vec3 right;
        if (p_plane.m_normal.x != 0 || p_plane.m_normal.z != 0)
            right = glm::normalize(glm::vec3(p_plane.m_normal.z, 0.f, -p_plane.m_normal.x));
        else
            right = glm::normalize(glm::vec3(p_plane.m_normal.y, -p_plane.m_normal.x, 0.f));

        const glm::vec3 up = glm::normalize(glm::cross(right, p_plane.m_normal));

        // Since Plane has no real position, we construct the quad at the arbitrary center-point represented by the closest point on the plane to the origin.
        const glm::vec3 p = p_plane.m_normal * p_plane.m_distance;

        m_point_1 = glm::vec3(p + right + up);
        m_point_2 = glm::vec3(p - right + up);
        m_point_3 = glm::vec3(p - right - up);
        m_point_4 = glm::vec3(p + right - up);
    }

    glm::vec3 Quad::center() const
    {
        return (m_point_1 + m_point_2 + m_point_3 + m_point_4) / 4.0f;
    }
    void Quad::scale(const float p_scale)
    {
        const auto c = center();
        m_point_1 = m_point_1 + (glm::normalize(m_point_1 - c) * p_scale);
        m_point_2 = m_point_2 + (glm::normalize(m_point_2 - c) * p_scale);
        m_point_3 = m_point_3 + (glm::normalize(m_point_3 - c) * p_scale);
        m_point_4 = m_point_4 + (glm::normalize(m_point_4 - c) * p_scale);
    }

    std::array<Triangle, 2> Quad::get_triangles() const
    {
        return std::array<Triangle, 2>({{m_point_1, m_point_2, m_point_3}, {m_point_3, m_point_4, m_point_1}});
    }
}