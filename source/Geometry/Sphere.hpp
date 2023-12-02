#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
	class Sphere
	{
	public:
		Sphere(const glm::vec3& p_position, const float& p_radius)
			: m_center{p_position}
			, m_radius{p_radius}
		{}

		glm::vec3 m_center;
		float m_radius;
	};
}