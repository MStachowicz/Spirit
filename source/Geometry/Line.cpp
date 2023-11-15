#include "Line.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
	Line::Line(const glm::vec3& p_point, const glm::vec3& p_direction, bool p_normalize_direction) noexcept
		: m_point_1{p_point}
		, m_point_2{p_point + (p_normalize_direction ? glm::normalize(p_direction) : p_direction)}
	{}
}