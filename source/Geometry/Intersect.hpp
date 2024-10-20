#pragma once

#include "Geometry/Line.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Constants.hpp"
#include "Utility/Logger.hpp"

#include "glm/vec3.hpp"
#include <optional>

namespace Geometry
{
	class AABB;
	class Cone;
	class Cuboid;
	class Cylinder;
	class Plane;
	class Quad;
	class Ray;
	class Sphere;
	class Triangle;

	// Returns the scalar triple product of u, v and w. Also referred to as the box product.
	// Geometrically, the value of the scalar triple product corresponds to the (signed) volume of a parallelepiped formed by the three independent vectors u, v, and w.
	// The special notation [uvw] is often used to denote a triple product. [uvw] == (u x v)·w
	// Real-Time Collision Detection (Christer Ericson) - 3.3.7 The Scalar Triple Product pg 44
	float triple_product(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w);

//==============================================================================================================================
// Point inside
//==============================================================================================================================
	bool point_inside(const AABB& AABB,               const glm::vec3& point);
	bool point_inside(const Cone& cone,               const glm::vec3& point);
	bool point_inside(const Cylinder& cylinder,       const glm::vec3& point);
	bool point_inside(const Line& line,               const glm::vec3& point);
	bool point_inside(const LineSegment& lineSegment, const glm::vec3& point);
	bool point_inside(const Ray& ray,                 const glm::vec3& point);

//==============================================================================================================================
// Closest point functions
//==============================================================================================================================
	// Get the closest point along line to the point
	//@param line The line to find the closest point on
	//@param point The point to find the closest point to
	//@return The closest point on line to point
	glm::vec3 closest_point(const Line& line, const glm::vec3& point);
	// Get the closest point along line segment to the point
	//@param line The line segment to find the closest point on
	//@param point The point to find the closest point to
	//@return The closest point on line segment to point
	glm::vec3 closest_point(const LineSegment& line, const glm::vec3& point);
	// Get the closest point along ray to the point
	//@param ray The ray to find the closest point on
	//@param point The point to find the closest point to
	//@return The closest point on ray to point
	glm::vec3 closest_point(const Ray& ray, const glm::vec3& point);
	// Get the closest point on a triangle to the point
	//@param triangle The triangle to find the closest point on
	//@param point The point to find the closest point to
	//@return The closest point on triangle to point
	glm::vec3 closest_point(const Triangle& triangle, const glm::vec3& point);

//==============================================================================================================================
// distance functions
//==============================================================================================================================
	// Get the distance from the point to the line squared
	//@param line The line to find the distance to
	//@param point The point to find the distance from
	//@return The distance from point to line squared
	float distance_squared(const LineSegment& line, const glm::vec3& point);
	// Get the distance from the point to the line
	//@param line The line to find the distance to
	//@param point The point to find the distance from
	//@return The distance from point to line
	float distance(const LineSegment& line, const glm::vec3& point);
	// Get the distance from the point to the plane.
	// The distance is signed as the plane has a normal direction. If the point is on the opposite side of the plane to the normal, the distance will be negative.
	//@param plane The plane to find the distance to
	//@param point The point to find the distance from
	//@return The signed distance from point to plane
	float distance(const Plane& plane, const glm::vec3& point);

//==============================================================================================================================
// get_intersection functions: Return the point of intersection between two shapes from the perspective of shape A.
//==============================================================================================================================
	std::optional<glm::vec3> get_intersection(const Ray& ray, const Plane& plane);
	std::optional<glm::vec3> get_intersection(const AABB& AABB, const Ray& ray, float* distance_along_ray = nullptr);
	std::optional<glm::vec3> get_intersection(const Line& line, const Triangle& triangle);
	//@returns If the planes are parallel, std::nullopt is returned. If there is an intersection, the Line of intersection is returned.
	std::optional<Line> get_intersection(const Plane& plane_1, const Plane& plane_2);
	std::optional<glm::vec3> get_intersection(const Plane& plane, const Sphere& sphere);
	//@returns The point of intersection between the three planes. If the planes are parallel, std::nullopt is returned.
	std::optional<glm::vec3> get_intersection(const Plane& plane_1, const Plane& plane_2, const Plane& plane_3);
	std::optional<glm::vec3> get_intersection(const Sphere& sphere_1, const Sphere& sphere_2);
	// Computes the LineSegment of intersection of the two triangles if it exists.
	//@param triangle_1,triangle_2 The triangles to test for intersection.
	//@param is_coplanar Optional out param set to true if the triangles are coplanar.
	//@return The LineSegment of intersection if it exists, otherwise std::nullopt. If is_coplanar is true, the returned LineSegment is degerate.
	std::optional<LineSegment> triangle_triangle(const Triangle& triangle_1, const Triangle& triangle_2, bool* is_coplanar = nullptr);

//==============================================================================================================================
// intersecting functions: Return if the geometries are intersecting.
//==============================================================================================================================
	bool intersecting(const AABB& AABB_1,         const AABB& AABB_2);
	bool intersecting(const AABB& AABB,           const Ray& ray);
	bool intersecting(const Line& line,           const Triangle& triangle);
	bool intersecting(const Plane& plane_1,       const Plane& plane_2);
	bool intersecting(const Plane& plane,         const Sphere& sphere);
	bool intersecting(const Plane& plane,         const glm::vec3& point, float tolerance = TOLERANCE);
	bool intersecting(const Sphere& sphere_1,     const Sphere& sphere_2);
	bool intersecting(const Triangle& triangle_1, const Triangle& triangle_2);

} // namespace Geometry