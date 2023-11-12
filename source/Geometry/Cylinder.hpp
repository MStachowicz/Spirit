#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
	class Cylinder
	{
	public:
		Cylinder(const glm::vec3& p_base, const glm::vec3& p_top, float p_radius)
			: m_base{p_base}
			, m_top{p_top}
			, m_radius{p_radius}
		{}

		glm::vec3 m_base;
		glm::vec3 m_top;
		float m_radius;
	};
}