#include "Intersect.hpp"

#include "AABB.hpp"
#include "Plane.hpp"
#include "Ray.hpp"
#include "Triangle.hpp"

#include <glm/glm.hpp>
#include <limits>

// This intersections source file is composed of header definitions as well as cpp-static-functions that are used as helpers for them.
namespace Geometry
{
    static constexpr float Epsilon        = std::numeric_limits<float>::epsilon();
    // Enabling this adds robustness checks that account for the floating-point margin of error.
    static constexpr bool  UseEpsilonTest = true;

    // This edge to edge test is based on Franlin Antonio's gem: Faster Line Segment Intersection - Graphics Gems III pp. 199-202
    static bool edge_edge_test(const glm::vec3& V0, const glm::vec3& U0, const glm::vec3& U1, float& Ax, float& Ay, int& i0, int& i1)
    {
        float Bx = U0[i0] - U1[i0];
        float By = U0[i1] - U1[i1];
        float Cx = V0[i0] - U0[i0];
        float Cy = V0[i1] - U0[i1];
        float f  = Ay * Bx - Ax * By;
        float d  = By * Cx - Bx * Cy;
        if ((f > 0 && d >= 0 && d <= f) || (f < 0 && d <= 0 && d >= f))
        {
            float e = Ax * Cy - Ay * Cx;
            if (f > 0)
            {
                if (e >= 0 && e <= f)
                    return true;
            }
            else
            {
                if (e <= 0 && e >= f)
                    return true;
            }
        }
        return false;
    }
    static bool edge_against_tri_edges(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& U0, const glm::vec3& U1, const glm::vec3& U2, int& i0, int& i1)
    {
        float Ax = V1[i0] - V0[i0];
        float Ay = V1[i1] - V0[i1];
        // test edge U0,U1 against V0,V1 OR
        // test edge U1,U2 against V0,V1 OR
        // test edge U2,U1 against V0,V1
        if (edge_edge_test(V0, U0, U1, Ax, Ay, i0, i1) || edge_edge_test(V0, U1, U2, Ax, Ay, i0, i1) || edge_edge_test(V0, U2, U0, Ax, Ay, i0, i1))
            return true;
        else
            return false;

    }
    static bool point_in_tri(const glm::vec3& V0, const glm::vec3& U0, const glm::vec3& U1, const glm::vec3& U2, int i0, int i1)
    {
        float a, b, c, d0, d1, d2;
        /* is T1 completly inside T2? */
        /* check if V0 is inside tri(U0,U1,U2) */
        a  = U1[i1] - U0[i1];
        b  = -(U1[i0] - U0[i0]);
        c  = -a * U0[i0] - b * U0[i1];
        d0 = a * V0[i0] + b * V0[i1] + c;

        a  = U2[i1] - U1[i1];
        b  = -(U2[i0] - U1[i0]);
        c  = -a * U1[i0] - b * U1[i1];
        d1 = a * V0[i0] + b * V0[i1] + c;

        a  = U0[i1] - U2[i1];
        b  = -(U0[i0] - U2[i0]);
        c  = -a * U2[i0] - b * U2[i1];
        d2 = a * V0[i0] + b * V0[i1] + c;

        if (d0 * d1 > 0.0)
        {
            if (d0 * d2 > 0.0)
                return true;
        }
        return false;
    }
    static bool coplanar_tri_tri(const glm::vec3& N, const Triangle& pTriangle1, const Triangle& pTriangle2)
    {
        int i0, i1;
        const glm::vec3 A = glm::abs(N);
        // first project onto an axis-aligned plane, that maximizes the area of the triangles, compute indices: i0,i1.

        if (A[0] > A[1])
        {
            if (A[0] > A[2])
            {
                i0 = 1; // A[0] is greatest
                i1 = 2;
            }
            else
            {
                i0 = 0; // A[2] is greatest
                i1 = 1;
            }
        }
        else // A[0]<=A[1]
        {
            if (A[2] > A[1])
            {
                i0 = 0; // A[2] is greatest
                i1 = 1;
            }
            else
            {
                i0 = 0; // A[1] is greatest
                i1 = 2;
            }
        }

        // test all edges of triangle 1 against the edges of triangle 2
        if (edge_against_tri_edges(pTriangle1.mPoint1, pTriangle1.mPoint2, pTriangle2.mPoint1, pTriangle2.mPoint2, pTriangle2.mPoint3, i0, i1)
        || edge_against_tri_edges(pTriangle1.mPoint2, pTriangle1.mPoint3, pTriangle2.mPoint1, pTriangle2.mPoint2, pTriangle2.mPoint3, i0, i1)
        || edge_against_tri_edges(pTriangle1.mPoint3, pTriangle1.mPoint1, pTriangle2.mPoint1, pTriangle2.mPoint2, pTriangle2.mPoint3, i0, i1))
            return true;

        // finally, test if triangle 1 is totally contained in triangle 2 or vice versa
        if (point_in_tri(pTriangle1.mPoint1, pTriangle2.mPoint1, pTriangle2.mPoint2, pTriangle2.mPoint3, i0, i1) || point_in_tri(pTriangle2.mPoint1, pTriangle1.mPoint1, pTriangle1.mPoint2, pTriangle1.mPoint3, i0, i1))
            return true;

        return false;
    }
    static void isect(float& VV0, float& VV1, float& VV2, float& D0, float& D1, float& D2, float& isect0, float& isect1)
    {
        isect0 = VV0 + (VV1 - VV0) * D0 / (D0 - D1);
        isect1 = VV0 + (VV2 - VV0) * D0 / (D0 - D2);
    }
    // Compute the intervals of two triangles and set the intersections to isect0 and isect1. Returns false if the triangles are coplanar and the assignment didnt happen.
    static bool compute_intervals(float& VV0, float& VV1, float& VV2, float& D0, float& D1, float& D2, float& D0D1, float& D0D2, float& isect0, float& isect1)
    {
        if (D0D1 > 0.0f)
            isect(VV2, VV0, VV1, D2, D0, D1, isect0, isect1); // here we know that D0D2<=0.0, that is D0, D1 are on the same side, D2 on the other or on the plane
        else if (D0D2 > 0.0f)
            isect(VV1, VV0, VV2, D1, D0, D2, isect0, isect1); // here we know that d0d1<=0.0
        else if (D1 * D2 > 0.0f || D0 != 0.0f)
            isect(VV0, VV1, VV2, D0, D1, D2, isect0, isect1); // here we know that d0d1<=0.0 or that D0!=0.0
        else if (D1 != 0.0f)
            isect(VV1, VV0, VV2, D1, D0, D2, isect0, isect1);
        else if (D2 != 0.0f)
            isect(VV2, VV0, VV1, D2, D0, D1, isect0, isect1);
        else // triangles are coplanar
            return false; // coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);

        return true;
    }


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

    bool intersect_triangle_triangle_static(const Triangle& pTriangle1, const Triangle& pTriangle2)
    {
        // Uses the Möller-Trumbore intersection algorithm to perform triangle-triangle collision detection. Adapted from:
        // https://github.com/erich666/jgt-code/blob/master/Volume_08/Number_1/Shen2003/tri_tri_test/include/Moller97.c

        // compute plane of pTriangle1
        glm::vec3 E1 = pTriangle1.mPoint2 - pTriangle1.mPoint1;
        glm::vec3 E2 = pTriangle1.mPoint3 - pTriangle1.mPoint1;
        glm::vec3 N1 = glm::cross(E1, E2); // Normal of pTriangle1
        float d1 = -glm::dot(N1, pTriangle1.mPoint1);
        // plane equation 1: N1.X+d1=0

        // Put pTriangle2 into plane equation 1 to compute signed distances to the plane
        float du0 = glm::dot(N1, pTriangle2.mPoint1) + d1;
        float du1 = glm::dot(N1, pTriangle2.mPoint2) + d1;
        float du2 = glm::dot(N1, pTriangle2.mPoint3) + d1;

        // coplanarity robustness check
        if constexpr (UseEpsilonTest)
        {
            if (std::abs(du0) < Epsilon) du0 = 0.0;
            if (std::abs(du1) < Epsilon) du1 = 0.0;
            if (std::abs(du2) < Epsilon) du2 = 0.0;
        }
        float du0du1 = du0 * du1;
        float du0du2 = du0 * du2;

        if (du0du1 > 0.0f && du0du2 > 0.0f) // same sign on all of them + not equal 0 = no intersecton
            return false;

        // compute plane of pTriangle2
        E1 = pTriangle2.mPoint2 - pTriangle2.mPoint1;
        E2 = pTriangle2.mPoint3 - pTriangle2.mPoint1;
        glm::vec3 N2 = glm::cross(E1, E2);
        float d2 = -glm::dot(N2, pTriangle2.mPoint1);
        // plane equation 2: N2.X+d2=0

        // put pTriangle1 into plane equation of pTriangle2
        float dv0 = glm::dot(N2, pTriangle1.mPoint1) + d2;
        float dv1 = glm::dot(N2, pTriangle1.mPoint2) + d2;
        float dv2 = glm::dot(N2, pTriangle1.mPoint3) + d2;

        if constexpr (UseEpsilonTest)
        {
            if (std::abs(dv0) < Epsilon) dv0 = 0.0;
            if (std::abs(dv1) < Epsilon) dv1 = 0.0;
            if (std::abs(dv2) < Epsilon) dv2 = 0.0;
        }

        float dv0dv1 = dv0 * dv1;
        float dv0dv2 = dv0 * dv2;

        // Same sign on all of them and not equal to 0 then no intersection occurs
        if (dv0dv1 > 0.0f && dv0dv2 > 0.0f)
            return false;

        // compute direction of intersection line
        glm::vec3 D = glm::cross(N1, N2);

        // compute and index to the largest component of D
        float max = std::abs(D[0]);
        float b   = std::abs(D[1]);
        float c   = std::abs(D[2]);
        int index = 0;

        if (b > max) max = b, index = 1;
        if (c > max) max = c, index = 2;

        // this is the simplified projection onto L
        float vp0 = pTriangle1.mPoint1[index];
        float vp1 = pTriangle1.mPoint2[index];
        float vp2 = pTriangle1.mPoint3[index];

        float up0 = pTriangle2.mPoint1[index];
        float up1 = pTriangle2.mPoint2[index];
        float up2 = pTriangle2.mPoint3[index];

        glm::vec2 isect1;
        glm::vec2 isect2;

        // compute interval for triangle 1 and triangle 2. If the interval check comes back false
        // the triangles are coplanar and we can early out by checking for collision between coplanar triangles.
        if (!compute_intervals(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, isect1[0], isect1[1]))
            return coplanar_tri_tri(N1, pTriangle1, pTriangle2);
        if (!compute_intervals(up0, up1, up2, du0, du1, du2, du0du1, du0du2, isect2[0], isect2[1]))
            return coplanar_tri_tri(N1, pTriangle1, pTriangle2);

        // Sort so isect 1 and 2 are in ascending order by index
        if (isect1[0] > isect1[1])
            std::swap(isect1[0], isect1[1]);
        if (isect2[0] > isect2[1])
            std::swap(isect2[0], isect2[1]);

        if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
            return false;
        else
            return true;
    }
} // namespace Geometry