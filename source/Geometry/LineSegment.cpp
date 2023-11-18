#include "LineSegment.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
	float LineSegment::length() const noexcept
	{
		return glm::length(m_end - m_start);
	}
	glm::vec3 LineSegment::direction() const noexcept
	{
		return glm::normalize(m_end - m_start);
	}
	glm::vec3 LineSegment::point_along_ray(const float& p_distance_along_ray) const noexcept
	{
		return m_start + (direction() * p_distance_along_ray);
	}
} // namespace Geometry