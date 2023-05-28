#pragma once

#include "glm/vec3.hpp"

#include <optional>

// The functions in this file can be categorised in two ways.
// 1. Interference detection functions: whether two (static) objects are overlapping at their given positions and orientations.
// 2. Closest point functions: Finding the closest point on object 1 to object 2.
// 3. Intersection finding: Finding the points of contact.
namespace Geometry
{
    class AABB;
    class Ray;
    class Plane;
    class Triangle;
    class Line;

    // Returns the scalar triple product of u, v and w. Also referred to as the box product.
    // Geometrically, the value of the scalar triple product corresponds to the (signed) volume of a parallelepiped formed by the three independent vectors u, v, and w.
    // The special notation [uvw] is often used to denote a triple product. [uvw] == (u x v)·w
    // Real-Time Collision Detection (Christer Ericson) - 3.3.7 The Scalar Triple Product pg 44
    float triple_product(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w);

    // Interference detection ==============================================================================================================================
    bool intersect(const Plane& p_plane_1, const Plane& p_plane_2);
    bool intersect(const AABB& p_AABB, const AABB& p_AABB_2);
    bool intersect(const AABB& p_AABB, const Ray& p_ray, glm::vec3* p_intersection_point = nullptr, float* p_length_along_ray = nullptr);
    bool intersect_triangle_triangle_static(const Triangle& p_triangle_1, const Triangle& p_triangle_2, bool p_test_co_planar = true);
    // Check if the planes intersect. If they do, returns information about the intersection line, its out_direction and a out_point_on_intersection_line
    bool intersect_plane_plane_static(const Plane& p_plane_1, const Plane& p_plane_2, glm::vec3& out_direction, glm::vec3& out_point_on_intersection_line);
    bool intersect_line_triangle(const Line& p_line, const Triangle& p_triangle);

    // Interference finding ==============================================================================================================================
    bool intersect_line_triangle(const Line& p_line, const Triangle& p_triangle, glm::vec3& out_collision_point);

} // namespace Geometry