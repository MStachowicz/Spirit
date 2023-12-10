#pragma once

#include "glm/vec3.hpp"
#include "glm/fwd.hpp"

namespace Geometry
{
	// A cone is shape that tapers from a circular base centered at m_base to a point m_top.
	// Cone is assumed to be right-circular meaning the base is always a circle.
	class Cone
	{
	public:
		constexpr Cone(const glm::vec3& p_base, const glm::vec3& p_top, float p_radius) noexcept
			: m_base{p_base}
			, m_top{p_top}
			, m_base_radius{p_radius}
		{}

		// Transforms the cone by the given model matrix and scale.
		// The scaling in x and z axis must be equal because the cone is a circular cone.
		void transform(const glm::mat4& p_model, const glm::vec3& p_scale);
		void draw_UI() const;

		glm::vec3 m_base;
		glm::vec3 m_top;
		float m_base_radius;
	};
}