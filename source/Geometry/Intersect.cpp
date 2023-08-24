#include "Intersect.hpp"

#include "AABB.hpp"
#include "Line.hpp"
#include "Plane.hpp"
#include "Ray.hpp"
#include "Triangle.hpp"

#include "Logger.hpp"

#include <glm/glm.hpp>
#include <limits>

// This intersections source file is composed of header definitions as well as cpp-static-functions that are used as helpers for them.
namespace Geometry
{
    static constexpr float Epsilon        = std::numeric_limits<float>::epsilon();
    // Enabling this adds robustness checks that account for the floating-point margin of error.
    static constexpr bool  Use_Epsilon_Test = true;

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
    static bool coplanar_tri_tri(const glm::vec3& N, const Triangle& p_triangle_1, const Triangle& p_triangle_2)
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

        // test all edges of p_triangle_1 against the edges of p_triangle_2
        if (edge_against_tri_edges(p_triangle_1.m_point_1, p_triangle_1.m_point_2, p_triangle_2.m_point_1, p_triangle_2.m_point_2, p_triangle_2.m_point_3, i0, i1)
        || edge_against_tri_edges(p_triangle_1.m_point_2, p_triangle_1.m_point_3, p_triangle_2.m_point_1, p_triangle_2.m_point_2, p_triangle_2.m_point_3, i0, i1)
        || edge_against_tri_edges(p_triangle_1.m_point_3, p_triangle_1.m_point_1, p_triangle_2.m_point_1, p_triangle_2.m_point_2, p_triangle_2.m_point_3, i0, i1))
            return true;

        // finally, test if p_triangle_1 is totally contained in p_triangle_2 or vice versa
        if (point_in_tri(p_triangle_1.m_point_1, p_triangle_2.m_point_1, p_triangle_2.m_point_2, p_triangle_2.m_point_3, i0, i1) || point_in_tri(p_triangle_2.m_point_1, p_triangle_1.m_point_1, p_triangle_1.m_point_2, p_triangle_1.m_point_3, i0, i1))
            return true;

        return false;
    }
    static void isect(const float& VV0, const float& VV1, const float& VV2, const float& D0, const float& D1, const float& D2, float& isect0, float& isect1)
    {
        isect0 = VV0 + (VV1 - VV0) * D0 / (D0 - D1);
        isect1 = VV0 + (VV2 - VV0) * D0 / (D0 - D2);
    }
    // Compute the intervals of two triangles and set the intersections to isect0 and isect1.
    // Returns false if the triangles are coplanar and the assignment didnt happen.
    static bool compute_intervals(const float& VV0, const float& VV1, const float& VV2, const float& D0, const float& D1, const float& D2, const float& D0D1, const float& D0D2, float& isect0, float& isect1)
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


    bool intersect(const AABB& p_AABB_1, const AABB& p_AABB_2)
    {
        // Reference: Real-Time Collision Detection (Christer Ericson)
        // Exit with no intersection if separated along an axis, overlapping on all axes means AABBs are intersecting
        if (p_AABB_1.mMax[0] < p_AABB_2.mMin[0] || p_AABB_1.mMin[0] > p_AABB_2.mMax[0]
         || p_AABB_1.mMax[1] < p_AABB_2.mMin[1] || p_AABB_1.mMin[1] > p_AABB_2.mMax[1]
         || p_AABB_1.mMax[2] < p_AABB_2.mMin[2] || p_AABB_1.mMin[2] > p_AABB_2.mMax[2])
            return false;
        else
            return true;
    }

    bool intersect(const AABB& p_AABB_1, const Ray& p_ray, glm::vec3* p_intersection_point/*= nullptr*/, float* p_length_along_ray/*= nullptr*/)
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
            if (std::abs(p_ray.m_direction[i]) < EPSILON)
            {
                // Ray is parallel to slab. No hit if origin not within slab
                if (p_ray.m_start[i] < p_AABB_1.mMin[i] || p_ray.m_start[i] > p_AABB_1.mMax[i])
                    return false;
            }
            else
            {
                // Compute intersection values along p_ray with near and far plane of slab on i axis
                const float ood = 1.0f / p_ray.m_direction[i];
                float entry = (p_AABB_1.mMin[i] - p_ray.m_start[i]) * ood;
                float exit = (p_AABB_1.mMax[i] - p_ray.m_start[i]) * ood;

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

        // Ray intersects all 3 slabs. Return point p_intersection_point and p_length_along_ray
        if (p_intersection_point) *p_intersection_point = p_ray.m_start + p_ray.m_direction * farthestEntry;
        if (p_length_along_ray)    *p_length_along_ray    = farthestEntry;
        return true;
    }

    bool intersect(const Plane& p_plane_1, const Plane& p_plane_2)
    {
        // If the dot product is equal to zero, the planes are parallel and do not intersect
        if (glm::dot(p_plane_1.m_normal, p_plane_2.m_normal) == 0.0f)
            return false;
        else
            return true;
    }


    bool intersect_triangle_triangle_static(const Triangle& p_triangle_1, const Triangle& p_triangle_2, bool p_test_co_planar)
    {
        // Uses the Möller-Trumbore intersection algorithm to perform triangle-triangle collision detection. Adapted from:
        // https://github.com/erich666/jgt-code/blob/master/Volume_08/Number_1/Shen2003/tri_tri_test/include/Moller97.c
        // https://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf

        // compute plane of p_triangle_1
        glm::vec3 E1 = p_triangle_1.m_point_2 - p_triangle_1.m_point_1;
        glm::vec3 E2 = p_triangle_1.m_point_3 - p_triangle_1.m_point_1;
        glm::vec3 N1 = glm::cross(E1, E2); // Normal of p_triangle_1
        float d1 = -glm::dot(N1, p_triangle_1.m_point_1);
        // plane equation 1: N1.X+d1=0

        // Put p_triangle_2 into plane equation 1 to compute signed distances to the plane
        float du0 = glm::dot(N1, p_triangle_2.m_point_1) + d1;
        float du1 = glm::dot(N1, p_triangle_2.m_point_2) + d1;
        float du2 = glm::dot(N1, p_triangle_2.m_point_3) + d1;

        // coplanarity robustness check
        if constexpr (Use_Epsilon_Test)
        {
            if (std::abs(du0) < Epsilon) du0 = 0.0;
            if (std::abs(du1) < Epsilon) du1 = 0.0;
            if (std::abs(du2) < Epsilon) du2 = 0.0;
        }
        float du0du1 = du0 * du1;
        float du0du2 = du0 * du2;

        if (du0du1 > 0.0f && du0du2 > 0.0f) // same sign on all of them + not equal 0 = no intersecton
            return false;

        // compute plane of p_triangle_2
        E1 = p_triangle_2.m_point_2 - p_triangle_2.m_point_1;
        E2 = p_triangle_2.m_point_3 - p_triangle_2.m_point_1;
        glm::vec3 N2 = glm::cross(E1, E2); // Tr Normal
        float d2 = -glm::dot(N2, p_triangle_2.m_point_1);
        // plane equation 2: N2.X+d2=0

        // put p_triangle_1 into plane equation of p_triangle_2
        float dv0 = glm::dot(N2, p_triangle_1.m_point_1) + d2;
        float dv1 = glm::dot(N2, p_triangle_1.m_point_2) + d2;
        float dv2 = glm::dot(N2, p_triangle_1.m_point_3) + d2;

        if constexpr (Use_Epsilon_Test)
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
        float vp0 = p_triangle_1.m_point_1[index];
        float vp1 = p_triangle_1.m_point_2[index];
        float vp2 = p_triangle_1.m_point_3[index];

        float up0 = p_triangle_2.m_point_1[index];
        float up1 = p_triangle_2.m_point_2[index];
        float up2 = p_triangle_2.m_point_3[index];

        glm::vec2 isect1;
        glm::vec2 isect2;

        // compute interval for p_triangle_1 and p_triangle_2. If the interval check comes back false
        // the triangles are coplanar and we can early out by checking for collision between coplanar triangles.
        if (!compute_intervals(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, isect1[0], isect1[1]))
        {
            if (p_test_co_planar)
                return coplanar_tri_tri(N1, p_triangle_1, p_triangle_2);
            else
                return false;
        }
        if (!compute_intervals(up0, up1, up2, du0, du1, du2, du0du1, du0du2, isect2[0], isect2[1]))
        {
            if (p_test_co_planar)
                return coplanar_tri_tri(N1, p_triangle_1, p_triangle_2);
            else
                return false;
        }

        // Sort so components of isect1 and isect2 are in ascending order.
        if (isect1[0] > isect1[1])
            std::swap(isect1[0], isect1[1]);
        if (isect2[0] > isect2[1])
            std::swap(isect2[0], isect2[1]);

        if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
            return false;
        else
            return true;
    }

    bool intersect_plane_plane_static(const Plane& p_plane_1, const Plane& p_plane_2, glm::vec3& out_direction, glm::vec3& out_point_on_intersection_line)
    {
        // Compute direction of intersection line
        out_direction = glm::cross(p_plane_1.m_normal, p_plane_2.m_normal);

        // If d is (near) zero, the planes are parallel (and separated)
        // or coincident, so they’re not considered intersecting
        const float denom = glm::dot(out_direction, out_direction);
        if (denom < Epsilon)
            return false;

        // Compute point on intersection line
        out_point_on_intersection_line = glm::cross(p_plane_1.m_distance * p_plane_2.m_normal - p_plane_2.m_distance * p_plane_1.m_normal, out_direction) / denom;
        return true;
    }

    bool intersect_plane_plane_plane(const Plane& p_plane, const Plane& p_plane_2, const Plane& p_plane_3, glm::vec3& out_collision_point)
    {
        const glm::vec3 u = glm::cross(p_plane_2.m_normal, p_plane_3.m_normal);
        const float denom = glm::dot(p_plane.m_normal, u);

        if (std::abs(denom) < Epsilon)
            return false;
        else
        {
            out_collision_point = (p_plane.m_distance * u + glm::cross(p_plane.m_normal, p_plane_3.m_distance * p_plane_2.m_normal - p_plane_2.m_distance * p_plane_3.m_normal)) / denom;
            return true;
        }
    }

    bool intersect_line_triangle(const Line& p_line, const Triangle& p_triangle)
    {
        // Below works for a double-sided triangle (both CW or CCW depending on which side it is viewed),
        // p_line passes on the inside if all three scalar triple products (u,v,w) have the same sign (ignoring zeroes).
        // The code does not handle the case when p_line is in the same plane as p_triangle.

        const glm::vec3 pq = p_line.m_point_2 - p_line.m_point_1;
        const glm::vec3 pa = p_triangle.m_point_1 - p_line.m_point_1;
        const glm::vec3 pb = p_triangle.m_point_2 - p_line.m_point_1;
        const glm::vec3 pc = p_triangle.m_point_3 - p_line.m_point_1;

        const glm::vec3 m = glm::cross(pq, pc); // m allows us to avoid an extra cross product below.
        const auto u      = glm::dot(pb, m);    // triple_product(pq, pc, pb);
        const auto v      = -glm::dot(pa, m);   // triple_product(pq, pa, pc);
        const auto w      = triple_product(pq, pb, pa);

        if (u == 0 && v == 0 && w == 0)
            ASSERT(false, "[INTERSECT] Line is in the plane of the triangle. This isn't handled yet (intersect_line_line).");

        return (u <= 0.f && v <= 0.f && w <= 0.f) || (u >= 0.f && v >= 0.f && w >= 0.f); // have the same sign (ignoring zeroes)
    }
    bool intersect_line_triangle(const Line& p_line, const Triangle& p_triangle, glm::vec3& out_intersection_point)
    {
        // Identical to the above function but uses u, v, w to determine the intersection point to return.

        const glm::vec3 pq = p_line.m_point_2 - p_line.m_point_1;
        const glm::vec3 pa = p_triangle.m_point_1 - p_line.m_point_1;
        const glm::vec3 pb = p_triangle.m_point_2 - p_line.m_point_1;
        const glm::vec3 pc = p_triangle.m_point_3 - p_line.m_point_1;

        const glm::vec3 m = glm::cross(pq, pc); // m allows us to avoid an extra cross product below.
        auto u      = glm::dot(pb, m);    // triple_product(pq, pc, pb);
        auto v      = -glm::dot(pa, m);   // triple_product(pq, pa, pc);
        auto w      = triple_product(pq, pb, pa);

        if (u == 0 && v == 0 && w == 0)
            ASSERT(false, "[INTERSECT] Line is in the plane of the triangle. This isn't handled yet (intersect_line_line).");

        if ((u <= 0.f && v <= 0.f && w <= 0.f) || (u >= 0.f && v >= 0.f && w >= 0.f)) // have the same sign (ignoring zeroes)
        {
            // Compute the barycentric coordinates (u, v, w) determining out_intersection_point,
            // r = (u * a) + (v * b) + (w * c)
            const float denom = 1.0f / (u + v + w);
            u *= denom;
            v *= denom;
            w *= denom; // w = 1.0f - u - v;
            out_intersection_point = (u * p_triangle.m_point_1) + (v * p_triangle.m_point_2) + (w * p_triangle.m_point_3);
            return true;
        }
        else
            return false;
    }
    bool intersect_ray_triangle(const Ray& p_ray, const Triangle& p_triangle)
    {
        //#TODO - Collision detection pg 190 5.3.6
        ASSERT(false, "Not implemented 'intersect_ray_triangle'");
        return false;
    }
    bool intersect_ray_triangle(const Ray& p_ray, const Triangle& p_triangle, glm::vec3& out_collision_point)
    {
        //#TODO - Collision detection pg 190 5.3.6
        ASSERT(false, "Not implemented 'intersect_ray_triangle'");
        return false;
    }
    bool intersect_line_segment_triangle(const LineSegment& p_line_segment, const Triangle& p_triangle)
    {
        //#TODO - Collision detection pg 190 5.3.6
        ASSERT(false, "Not implemented 'intersect_line_segment_triangle'");
        return false;
    }
    bool intersect_line_segment_triangle(const LineSegment& p_line_segment, const Triangle& p_triangle, glm::vec3& out_collision_point)
    {
        //#TODO - Collision detection pg 190 5.3.6
        ASSERT(false, "Not implemented 'intersect_line_segment_triangle'");
        return false;
    }

    float triple_product(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w)
    {
        // [uvw] = [vwu] = [wuv] = −[uwv] = −[vuw] = −[wvu]
        return glm::dot(glm::cross(u, v), w); // [uvw]
    }
} // namespace Geometry