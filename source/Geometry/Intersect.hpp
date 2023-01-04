#pragma once

#include "glm/vec3.hpp"

#include <optional>

// The functions in this file can be categorised in two ways.
// 1. Intersection detection - Whether two objects 1 and 2 are overlaping
// 2. Intersection finding   - finding the point of contact
namespace Geometry
{
    struct AABB;
    struct Ray;
    struct Plane;
    struct Triangle;

    // Information about the point of contact between objects.
    struct Collision
    {
        glm::vec3 mPoint;
        glm::vec3 mNormal;
    };

    bool intersect(const Plane& pPlane1, const Plane& pPlane2);
    bool intersect(const AABB& pAABB, const AABB& pOtherAABB);
    bool intersect(const AABB& pAABB, const Ray& pRay, glm::vec3* pIntersectionPoint = nullptr, float* pLengthAlongRay = nullptr);

    std::optional<Collision> getCollision(const Triangle& pTriangle1, const Triangle& pTriangle2);
}