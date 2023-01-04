#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    struct AABB;
    struct Ray;
    struct Plane;

    // Information about the point of contact between objects.
    struct Collision
    {
        glm::vec3 mPoint;
        glm::vec3 mNormal;
    };

    bool intersect(const Plane& pPlane1, const Plane& pPlane2);
    bool intersect(const AABB& pAABB, const AABB& pOtherAABB);
    bool intersect(const AABB& pAABB, const Ray& pRay, glm::vec3* pIntersectionPoint = nullptr, float* pLengthAlongRay = nullptr);
}