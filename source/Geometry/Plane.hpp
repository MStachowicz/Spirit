#pragma once

#include "Constants.hpp"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Geometry
{
	// A 2-dimensional surface extending indefinitely.
	// The equation of a plane in 3D: ax + by + cz + d = 0
	// where 'a' 'b' 'c' and 'd' are constants defining the plane and  'x' 'y' and 'z' are coordinates of a point on the plane.
	// e.g. a plane with normal [1,0,0] and d = 5 can be represented as 'x + 5 = 0' representing a plane that is 5 units to the left of the origin along the x-axis.
	class Plane
	{
	public:
		glm::vec3 m_normal; // Unit length normal of the plane.
		float m_distance;   // The distance of the plane from the origin along m_normal. Represents the 'd' in the plane equation.

		// Constructs a plane from a world space position and direction intialising the normal of the plane to p_direction.
		Plane(const glm::vec3& p_point, const glm::vec3& p_direction) noexcept;
		// Construct a plane from a plane equation.
		//@param p_equation Equation of a plane in the form 'x + y + z + w = 0'
		Plane(const glm::vec4& p_equation) noexcept;
		// @param p_point The point to check if it lies on the plane.
		// @param tolerance The tolerance to use when comparing the point to the plane.
		// @return True if the point lies on the plane, false otherwise.
		bool point_on_plane(const glm::vec3& p_point, const float tolerance = TOLERANCE) const;

		void normalise();
	};
}