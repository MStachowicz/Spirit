#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
	// A directed line segment. A finite section of a Line from m_start to m_end.
	class LineSegment
	{
	public:
		constexpr LineSegment(const glm::vec3& p_start, const glm::vec3& p_end) noexcept
			: m_start{p_start}
			, m_end{p_end}
		{}
		constexpr float length() const noexcept
		{
			return glm::length(m_end - m_start);
		}
		constexpr glm::vec3 direction() const noexcept
		{
			return glm::normalize(m_end - m_start);
		}
		constexpr glm::vec3 pointAt(const float& p_distance_along_ray) const noexcept
		{
			return m_start + (direction() * p_distance_along_ray);
		}

		glm::vec3 m_start;
		glm::vec3 m_end;
	};
}