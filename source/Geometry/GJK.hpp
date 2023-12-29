#pragma once

#include "glm/vec3.hpp"
#include "glm/fwd.hpp"

#include <array>
#include <vector>
#include <initializer_list>
#include <stdexcept>

namespace GJK
{
	// Simplex is a set of points that define a convex shape.
	// The size of the simplex determines the shape.
	// 0 = empty, 1 = point, 2 = line, 3 = triangle, 4 = tetrahedron
	struct Simplex
	{
		std::array<glm::vec3, 4> points;
		int size;

		Simplex()
			: points({glm::vec3{0.f}})
			, size(0)
		{}
		Simplex(std::initializer_list<glm::vec3> list)
			: points()
			, size(static_cast<int>(list.size()))
		{
			if (list.size() > 4) throw std::runtime_error("[GJK] Simplex can only hold up to 4 points.");

			std::copy(list.begin(), list.end(), points.begin());
		}
		Simplex& operator=(std::initializer_list<glm::vec3> list)
		{
			if (list.size() > 4) throw std::runtime_error("[GJK] Simplex can only hold up to 4 points.");

			size = 0;
			for (auto& point : list)
				points[size++] = point;

			return *this;
		}
		void push_front(const glm::vec3& point)
		{
			points = {point, points[0], points[1], points[2]};
			size   = std::min(size + 1, 4);
		}
		glm::vec3& operator[](int index) { return points[index]; }
		const glm::vec3& operator[](int index) const { return points[index]; }
	};

	// Is a and b in the same direction?
	//@param a,b: The direction vectors to compare. These don't have to be normalized since we only care about direction.
	//@return True if a and b are in the same direction, false otherwise.
	bool same_direction(const glm::vec3& a, const glm::vec3& b);

	// Given a convex shape defined by a set of points, find the furthest point in p_direction.
	// This is a brute force implementation of O(n) complexity.
	//@param p_direction The direction to search in. Doesn't have to be normalized since we only care about direction.
	//@param p_points The object-space point set that defines convex shape.
	glm::vec3 support_point(const glm::vec3& p_direction, const std::vector<glm::vec3>& p_points);

	// Given two convex shapes defined by a set of points in object space, find the furthest point in p_direction and return the difference.
	// By providing the transform and orientation of each convex shape, we can find the furthest point in world space.
	// This is a brute force implementation of O(2n) complexity.
	//@param p_direction: The direction to search in world space. Doesn't have to be normalized since we only care about direction.
	//@param p_points_1,p_points_2: The object-space point set that defines the convex shapes in object space.
	//@param p_transform_1,p_transform_2: The object->world space transform of the convex shapes.
	//@param p_orientation_1,p_orientation_2 The orientation of the convex shapes.
	//@return The difference between the furthest point of the convex shapes and the furthest point of the second convex shape in world space.
	glm::vec3 support_point(const glm::vec3& p_direction,
	                        const std::vector<glm::vec3>& p_points_1, const glm::mat4& p_transform_1, const glm::quat& p_orientation_1,
	                        const std::vector<glm::vec3>& p_points_2, const glm::mat4& p_transform_2, const glm::quat& p_orientation_2);

	// Performs an iteration of the GJK algorithm on p_simplex.
	// The p_simplex and p_direction are updated in place.
	//@param p_simplex The simplex to update.
	//@param p_direction The direction to update to the new direction towards the origin from the closest feature on p_simplex.
	//@returns True if the origin is contained in the simplex, false otherwise (with the updated p_simplex and p_direction).
	bool do_simplex(Simplex& p_simplex, glm::vec3& p_direction);

	// Given two convex shapes defined by a set of points in object space, and their transforms and orientations, determine if they intersect.
	//@param p_points_1,p_points_2: The object-space point set that defines the convex shapes in object space.
	//@param p_transform_1,p_transform_2: The object->world space transform of the convex shapes.
	//@param p_orientation_1,p_orientation_2 The orientation of the convex shapes.
	//@param p_initial_direction The initial direction to search in. Defaults to (1,0,0) arbitrarily. A good initial direction is the vector between the two shapes in world space.
	//@return True if the two convex shapes intersect, false otherwise.
	bool intersecting(const std::vector<glm::vec3>& p_points_1, const glm::mat4& p_transform_1, const glm::quat& p_orientation_1,
	                  const std::vector<glm::vec3>& p_points_2, const glm::mat4& p_transform_2, const glm::quat& p_orientation_2,
	                  const glm::vec3& p_initial_direction = glm::vec3(1.f, 0.f, 0.f));
} // namespace GJK