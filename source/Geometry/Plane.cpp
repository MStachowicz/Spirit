#include "Plane.hpp"
#include "Intersect.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
	Plane::Plane(const glm::vec3& p_point, const glm::vec3& p_direction) noexcept
		: m_normal{glm::normalize(p_direction)}
		, m_distance{glm::dot(m_normal, p_point)}
	{}

	Plane::Plane(const glm::vec4& p_equation) noexcept
		: m_normal{glm::vec3(p_equation.x, p_equation.y, p_equation.z)}
		, m_distance{p_equation.w}
	{}

	bool Plane::point_on_plane(const glm::vec3 & p_point, float tolerance) const { return intersecting(*this, p_point, tolerance); }

	void Plane::normalise()
	{
		// Normalise the plane p_equation
		// In order to normalize a plane equation we divide the coefficients x y z by the magnitude of the normal vector n.
		// We also need to divide the constant d by the magnitude of the normal vector n
		const float magnitude = glm::length(m_normal);
		m_normal   /= magnitude;
		m_distance /= magnitude;
	}
}