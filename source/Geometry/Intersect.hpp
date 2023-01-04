#include "glm/fwd.hpp"

namespace Geometry
{
    struct AABB;
    struct Ray;
    struct Plane;

    bool intersect(const Plane& pPlane1, const Plane& pPlane2);

    bool intersect(const AABB& pAABB, const AABB& pOtherAABB);
    bool intersect(const AABB& pAABB, const Ray& pRay, glm::vec3* pIntersectionPoint = nullptr, float* pLengthAlongRay = nullptr);
}