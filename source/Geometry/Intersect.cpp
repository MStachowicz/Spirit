#include "Intersect.hpp"

#include "AABB.hpp"

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
}