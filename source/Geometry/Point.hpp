#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
	class Point
	{
	public:
		glm::vec3 m_position;

		constexpr Point(const glm::vec3& p_position) noexcept : m_position{p_position} {}
		constexpr operator glm::vec3&() noexcept             { return m_position; }
		constexpr operator const glm::vec3&() const noexcept { return m_position; }
		constexpr bool operator==(const Point& p_other) const noexcept { return m_position == p_other.m_position; }
		constexpr bool operator!=(const Point& p_other) const noexcept { return m_position != p_other.m_position; }
	};
}