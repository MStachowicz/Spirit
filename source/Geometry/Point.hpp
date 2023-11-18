#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
	class Point
	{
	public:
		constexpr Point(const glm::vec3& p_position) noexcept : m_position{p_position} {}

		glm::vec3 m_position;
	};
}