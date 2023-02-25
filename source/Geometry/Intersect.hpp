#pragma once

#include "glm/vec3.hpp"

// The functions in this file can be categorised in two ways.
// 1. Interference detection functions: whether two (static) objects are overlapping at their given positions and orientations.
// 2. Closest point functions: Finding the closest point on object 1 to object 2.
// 3. Intersection finding: Finding the points of contact.
namespace Geometry
{
    struct AABB;
    struct Ray;
    struct Plane;
    struct Triangle;

    // Interference detection
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool intersect(const Plane& pPlane1, const Plane& pPlane2); // Plane v Plane
    bool intersect(const AABB& pAABB, const AABB& pOtherAABB);
    bool intersect(const AABB& pAABB, const Ray& pRay, glm::vec3* pIntersectionPoint = nullptr, float* pLengthAlongRay = nullptr);
    bool intersect_triangle_triangle_static(const Triangle& pTriangle1, const Triangle& pTriangle2, bool pTestCoPlaner = true);
}