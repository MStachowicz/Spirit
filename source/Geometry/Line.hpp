#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
	// An infinitely long object with no width, depth, or curvature. Defined by two points along the line.
	class Line
	{
	public:
		// Construct a line from a pair of points on the line.
		constexpr Line(const glm::vec3& p_point_1, const glm::vec3& p_point_2) noexcept
			: m_point_1{p_point_1}
			, m_point_2{p_point_2}
		{}
		// Construct a line from a point on the line and a direction.
		Line(const glm::vec3& p_point, const glm::vec3& p_direction, bool p_normalize_direction) noexcept;

		glm::vec3 m_point_1;
		glm::vec3 m_point_2;
	};
}