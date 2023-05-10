#include "Plane.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
    Plane::Plane(const glm::vec3& p_point, const glm::vec3& p_direction) noexcept
      : m_normal{glm::normalize(p_direction)}
      , m_distance{glm::dot(m_normal, p_point)}
    {}
}