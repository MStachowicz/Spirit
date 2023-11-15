#pragma once

#include "Geometry/Line.hpp"
#include "glm/vec3.hpp"

#include <optional>

namespace Geometry
{
	class AABB;
	class Ray;
	class Plane;
	class Triangle;
	class LineSegment;

	// Returns the scalar triple product of u, v and w. Also referred to as the box product.
	// Geometrically, the value of the scalar triple product corresponds to the (signed) volume of a parallelepiped formed by the three independent vectors u, v, and w.
	// The special notation [uvw] is often used to denote a triple product. [uvw] == (u x v)·w
	// Real-Time Collision Detection (Christer Ericson) - 3.3.7 The Scalar Triple Product pg 44
	float triple_product(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w);




	// ==============================================================================================================================
	// Intersection types (Types of intersection based on the shapes being intersected)
	// ==============================================================================================================================

	// Returned when a Line, LineSegment or Ray are intersecting at a given point.
	struct LineIntersection
	{
		glm::vec3 intersection_point; // The point where the Line/LineSegment/Ray intersects the plane.
		float length_along_ray;       // The length along the ray where the intersection point is located.
	};
	// Returned when two planes intersect.
	struct PlanePlaneIntersection
	{
		Line line; // The line along which the two planes intersect.
	};
	struct PointIntersection
	{
		glm::vec3 point; // The point where the intersection occurs.

		constexpr operator glm::vec3&() noexcept             { return point; }
		constexpr operator const glm::vec3&() const noexcept { return point; }
	};




	// ==============================================================================================================================
	// get_intersection functions (Where/how are we intersecting)
	// ==============================================================================================================================

	std::optional<LineIntersection> get_intersection(const AABB& p_AABB, const Ray& p_ray);
	// Check if the planes intersect. If they do, returns line intersection.
	std::optional<PlanePlaneIntersection> get_intersection(const Plane& p_plane_1, const Plane& p_plane_2);
	// Intersect 3 planes and return the single point they collide at.
	std::optional<PointIntersection> get_intersection(const Plane& p_plane, const Plane& p_plane_2, const Plane& p_plane_3);
	// Intersect a Ray with a Triangle and return the point of entry of the ray into the triangle.
	std::optional<PointIntersection> get_intersection(const Ray& p_ray, const Triangle& p_triangle);
	std::optional<PointIntersection> get_intersection(const LineSegment& p_line_segment, const Triangle& p_triangle);
	std::optional<PointIntersection> get_intersection(const Line& p_line, const Triangle& p_triangle);




	// ==============================================================================================================================
	// intersecting functions (Are we intersecting)
	// ==============================================================================================================================

	bool intersecting(const Plane& p_plane_1, const Plane& p_plane_2);
	bool intersecting(const AABB& p_AABB, const AABB& p_AABB_2);
	bool intersecting(const Line& p_line, const Triangle& p_triangle);
	bool intersecting(const Triangle& p_triangle_1, const Triangle& p_triangle_2, bool p_test_co_planar = true);
	// Returning the intersection point is free, call get_intersection if required instead.
	bool intersecting(const AABB& p_AABB, const Ray& p_ray);
	bool intersecting(const Ray& p_ray, const Triangle& p_triangle);
	bool intersecting(const LineSegment& p_line_segment, const Triangle& p_triangle);
} // namespace Geometry