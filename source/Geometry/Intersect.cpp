#include "Intersect.hpp"

#include "AABB.hpp"
#include "Ray.hpp"

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

        // Ray intersects all 3 slabs. Return point pIntersectionPoint and pLengthAlongRay
        if (pIntersectionPoint) *pIntersectionPoint = pRay.mStart + pRay.mDirection * farthestEntry;
        if (pLengthAlongRay)    *pLengthAlongRay    = farthestEntry;
        return true;
    }
} // namespace Geometry