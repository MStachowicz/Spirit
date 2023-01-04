#include "Intersect.hpp"

#include "AABB.hpp"
#include "Plane.hpp"
#include "Ray.hpp"
#include "Triangle.hpp"

#include <glm/glm.hpp>
#include <limits>

namespace Geometry
{
    bool intersect(const AABB& pAABB, const AABB& pOtherAABB)
    {
        // Reference: Real-Time Collision Detection (Christer Ericson)
        // Exit with no intersection if separated along an axis, overlapping on all axes means AABBs are intersecting
        if (pAABB.mMax[0] < pOtherAABB.mMin[0] || pAABB.mMin[0] > pOtherAABB.mMax[0]
         || pAABB.mMax[1] < pOtherAABB.mMin[1] || pAABB.mMin[1] > pOtherAABB.mMax[1]
         || pAABB.mMax[2] < pOtherAABB.mMin[2] || pAABB.mMin[2] > pOtherAABB.mMax[2])
            return false;
        else
            return true;
    }

    bool intersect(const AABB& pAABB, const Ray& pRay, glm::vec3* pIntersectionPoint/*= nullptr*/, float* pLengthAlongRay/*= nullptr*/)
    {
        // Adapted from: Real-Time Collision Detection (Christer Ericson) - 5.3.3 Intersecting Ray or Segment Against Box pg 180

        // Defining a slab as a space between a pair of parallel planes, the AABB volume can be seen as the intersection of 3 such slabs.
        // With these 3 slabs at right-angles to each other, a ray intersects the AABB if the intersections between the ray and the slabs all overlap.
        // A test for intersection only needs to keep track of
        // 1. Farthest of all entries into a slab
        // 2. Nearest of all exits  out of a slab
        // If the farthest entry ever becomes farther than the nearest exit, the ray cannot be intersecting.

        // To avoid a division by zero when the ray is parallel to a slab, substituting a test for the ray origin being contained in the slab.

        static constexpr auto EPSILON = std::numeric_limits<float>::epsilon();
        static constexpr auto MAX     = std::numeric_limits<float>::max();

        float farthestEntry = -MAX;
        float nearestExist = MAX;

        for (int i = 0; i < 3; i++)
        {
            if (std::abs(pRay.mDirection[i]) < EPSILON)
            {
                // Ray is parallel to slab. No hit if origin not within slab
                if (pRay.mStart[i] < pAABB.mMin[i] || pRay.mStart[i] > pAABB.mMax[i])
                    return false;
            }
            else
            {
                // Compute intersection values along pRay with near and far plane of slab on i axis
                const float ood = 1.0f / pRay.mDirection[i];
                float entry = (pAABB.mMin[i] - pRay.mStart[i]) * ood;
                float exit = (pAABB.mMax[i] - pRay.mStart[i]) * ood;

                if (entry > exit) // Make entry be intersection with near plane and exit with far plane
                    std::swap(entry, exit);

                if (entry > farthestEntry)
                    farthestEntry = entry;
                if (exit < nearestExist)
                    nearestExist = exit;

                // Exit with no collision as soon as slab intersection becomes empty
                if (farthestEntry > nearestExist)
                    return false;
            }
        }

        // Special case, if a bad Ray is passed in e.g. 0.0,0.0,0.0
        // All Ray components are < EPSILON we can fall through here and return true.
        if (farthestEntry == -MAX || nearestExist == MAX)
            return false;

        // Ray intersects all 3 slabs. Return point pIntersectionPoint and pLengthAlongRay
        if (pIntersectionPoint) *pIntersectionPoint = pRay.mStart + pRay.mDirection * farthestEntry;
        if (pLengthAlongRay)    *pLengthAlongRay    = farthestEntry;
        return true;
    }

    bool intersect(const Plane& pPlane1, const Plane& pPlane2)
    {
        // If the dot product is equal to zero, the planes are parallel and do not intersect
        if (glm::dot(pPlane1.mNormal, pPlane2.mNormal) == 0.0f)
            return false;
        else
            return true;
    }

    std::optional<Collision> getCollision(const Triangle& pTriangle1, const Triangle& pTriangle2)
    {
        // Uses the Möller-Trumbore intersection algorithm to perform triangle-triangle collision detection

        glm::vec3 e1 = pTriangle1.mPoint2 - pTriangle1.mPoint1;
        glm::vec3 e2 = pTriangle1.mPoint3 - pTriangle1.mPoint1;
        glm::vec3 s1 = glm::cross(pTriangle2.mPoint2 - pTriangle2.mPoint1, pTriangle2.mPoint3 - pTriangle2.mPoint1);

        const float divisor = glm::dot(s1, e1);
        if (divisor == 0.0f)
            return std::nullopt;

        const float invDivisor = 1.0f / divisor;
        glm::vec3 d      = pTriangle1.mPoint1 - pTriangle2.mPoint1;
        float b1         = glm::dot(d, s1) * invDivisor;

        if (b1 < 0.0f || b1 > 1.0f)
            return std::nullopt;

        glm::vec3 s2 = glm::cross(d, e1);
        float b2     = glm::dot(pTriangle2.mPoint3 - pTriangle2.mPoint1, s2) * invDivisor;
        if (b2 < 0.0f || b1 + b2 > 1.0f)
            return std::nullopt;

        float t = glm::dot(pTriangle2.mPoint2 - pTriangle2.mPoint1, s2) * invDivisor;
        if (t < 0.0f)
            return std::nullopt;

        // Calculate point of intersection and normal at point of intersection
        Collision collision;
        collision.mPoint = pTriangle1.mPoint1 + e1 * b1 + e2 * b2;
        collision.mNormal = glm::normalize(glm::cross(e1, e2));
        return collision;
    }
} // namespace Geometry