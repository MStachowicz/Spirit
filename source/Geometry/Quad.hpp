#pragma once

#include "Triangle.hpp"

#include "glm/vec3.hpp"
#include "glm/fwd.hpp"

#include <array>

namespace Geometry
{
	class Plane;
	class Triangle;

	// A quadrilateral. Four-sided polygon, having four edges (sides) and four corners (vertices).
	// Quad is a 2-dimensional shape.
	class Quad
	{
	public:
		glm::vec3 m_top_left;
		glm::vec3 m_top_right;
		glm::vec3 m_bottom_left;
		glm::vec3 m_bottom_right;

		// Construct a quad from its 4 corner points.
		constexpr Quad(const glm::vec3& p_top_left, const glm::vec3& p_top_right, const glm::vec3& p_bottom_left, const glm::vec3& p_bottom_right) noexcept
			: m_top_left{p_top_left}
			, m_top_right{p_top_right}
			, m_bottom_left{p_bottom_left}
			, m_bottom_right{p_bottom_right}
		{}
		// Construct a unit quad at p_point facing p_normal (counter-clockwise winding).
		Quad(const glm::vec3& p_point, const glm::vec3& p_normal) noexcept;
		// Construct a quad at the centroid position of p_triangle scaled to encompass it.
		Quad(const Triangle& p_triangle) noexcept;
		// Construct a unit quad inside the plane centered at the closest point of the plane to the origin.
		Quad(const Plane& p_plane) noexcept;

		void transform(const glm::mat4& p_transform);
		void draw_UI() const;

		// Uniformly scale the Quad by p_scale factor from its center.
		void scale(const float p_scale);

		glm::vec3 center() const;
		// Get a pair of triangles that represent this quad.
		std::array<Triangle, 2> get_triangles() const;
	};
}