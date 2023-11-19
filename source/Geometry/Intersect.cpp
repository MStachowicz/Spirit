#include "Intersect.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

#include "Logger.hpp"

#include <glm/glm.hpp>
#include <limits>

// This intersections source file is composed of header definitions as well as cpp-static-functions that are used as helpers for them.
namespace Geometry
{
	static constexpr float Epsilon        = std::numeric_limits<float>::epsilon();
	// Enabling this adds robustness checks that account for the floating-point margin of error.
	static constexpr bool  Use_Epsilon_Test = true;

// ==============================================================================================================================
// UTILITIY FUNCTIONS
// ==============================================================================================================================

	float triple_product(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w)
	{
		// [uvw] = [vwu] = [wuv] = −[uwv] = −[vuw] = −[wvu]
		return glm::dot(glm::cross(u, v), w); // [uvw]
	}
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
	static bool coplanar_tri_tri(const glm::vec3& N, const Triangle& triangle_1, const Triangle& triangle_2)
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

		// test all edges of triangle_1 against the edges of triangle_2
		if (edge_against_tri_edges(triangle_1.m_point_1, triangle_1.m_point_2, triangle_2.m_point_1, triangle_2.m_point_2, triangle_2.m_point_3, i0, i1)
		|| edge_against_tri_edges(triangle_1.m_point_2, triangle_1.m_point_3, triangle_2.m_point_1, triangle_2.m_point_2, triangle_2.m_point_3, i0, i1)
		|| edge_against_tri_edges(triangle_1.m_point_3, triangle_1.m_point_1, triangle_2.m_point_1, triangle_2.m_point_2, triangle_2.m_point_3, i0, i1))
			return true;

		// finally, test if triangle_1 is totally contained in triangle_2 or vice versa
		if (point_in_tri(triangle_1.m_point_1, triangle_2.m_point_1, triangle_2.m_point_2, triangle_2.m_point_3, i0, i1) || point_in_tri(triangle_2.m_point_1, triangle_1.m_point_1, triangle_1.m_point_2, triangle_1.m_point_3, i0, i1))
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
// ==============================================================================================================================
// END UTILITIY FUNCTIONS
// ==============================================================================================================================

	glm::vec3 closest_point(const Line& line, const glm::vec3& point)
	{
		const glm::vec3 ab = line.m_point_2 - line.m_point_1;
		// Project point onto line computing the parameterized position d(t) = P + t * AB
		const float t = glm::dot(point - line.m_point_1, ab) / glm::dot(ab, ab);
		return line.m_point_1 + (ab * t);
	}
	glm::vec3 closest_point(const LineSegment& line, const glm::vec3& point)
	{
		const glm::vec3 ab = line.m_end - line.m_start;
		// Project point onto line computing the parameterized position d(t) = P + t * AB
		const float t = glm::dot(point - line.m_start, ab) / glm::dot(ab, ab);

		// If outside segment, clamp t (and therefore d) to the closest endpoint
		     if (t < 0.f) return line.m_start;
		else if (t > 1.f) return line.m_end;
		else              return line.m_start + (ab * t);
	}
	glm::vec3 closest_point(const Ray& ray, const glm::vec3& point)
	{
		const glm::vec3& ab = ray.m_direction;
		// Project point onto line computing the parameterized position d(t) = P + t * AB
		const float t = glm::dot(point - ray.m_start, ab) / glm::dot(ab, ab);

		// If outside segment, clamp t (and therefore d) to the closest endpoint
		if (t < 0.f) return ray.m_start;
		else         return ray.m_start + (ab * t);

	}




	std::optional<Point> get_intersection(const AABB& AABB, const Point& point)
	{
		if (intersecting(AABB, point)) return point;
		else                           return std::nullopt;
	}
	std::optional<Geometry::Point> get_intersection(const AABB& AABB, const Ray& ray, float* distance_along_ray)
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
		float nearestExist  = MAX;

		for (int i = 0; i < 3; i++)
		{
			if (std::abs(ray.m_direction[i]) < EPSILON)
			{
				// Ray is parallel to slab. No hit if origin not within slab
				if (ray.m_start[i] < AABB.mMin[i] || ray.m_start[i] > AABB.mMax[i])
					return std::nullopt;
			}
			else
			{
				// Compute intersection values along ray with near and far plane of slab on i axis
				const float ood = 1.0f / ray.m_direction[i];
				float entry = (AABB.mMin[i] - ray.m_start[i]) * ood;
				float exit = (AABB.mMax[i] - ray.m_start[i]) * ood;

				if (entry > exit) // Make entry be intersection with near plane and exit with far plane
					std::swap(entry, exit);

				if (entry > farthestEntry)
					farthestEntry = entry;
				if (exit < nearestExist)
					nearestExist = exit;

				// Exit with no collision as soon as slab intersection becomes empty
				if (farthestEntry > nearestExist)
					return std::nullopt;
			}
		}

		// Special case, if a bad Ray is passed in e.g. 0.0,0.0,0.0
		// All Ray components are < EPSILON we can fall through here and return true.
		if (farthestEntry == -MAX || nearestExist == MAX)
			return std::nullopt;

		if (distance_along_ray)
			*distance_along_ray = farthestEntry;

		// Ray intersects all 3 slabs. Return point intersection_point and length_along_ray
		return Geometry::Point(ray.m_start + (ray.m_direction * farthestEntry));
	}
	std::optional<Point> get_intersection(const Cone& cone, const Point& point)
	{
		if (intersecting(cone, point))
			return point;
		else
			return std::nullopt;
	}
	std::optional<Point> get_intersection(const Cylinder& cylinder, const Point& point)
	{
		if (intersecting(cylinder, point)) return point;
		else                               return std::nullopt;
	}
	std::optional<Geometry::Point> get_intersection(const Line& line, const Triangle& triangle)
	{
		// Identical to the above function but uses u, v, w to determine the intersection point to return.

		const glm::vec3 pq = line.m_point_2 - line.m_point_1;
		const glm::vec3 pa = triangle.m_point_1 - line.m_point_1;
		const glm::vec3 pb = triangle.m_point_2 - line.m_point_1;
		const glm::vec3 pc = triangle.m_point_3 - line.m_point_1;

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
			return Geometry::Point{(u * triangle.m_point_1) + (v * triangle.m_point_2) + (w * triangle.m_point_3)};
		}
		else
			return std::nullopt;
	}
	std::optional<Geometry::Line> get_intersection(const Plane& plane_1, const Plane& plane_2)
	{
		// Compute direction of intersection line
		glm::vec3 direction = glm::cross(plane_1.m_normal, plane_2.m_normal);

		// If d is (near) zero, the planes are parallel (and separated)
		// or coincident, so they’re not considered intersecting
		const float denom = glm::dot(direction, direction);
		if (denom < Epsilon)
			return std::nullopt;

		// Compute point on intersection line
		glm::vec3 point_on_intersection_line = glm::cross(plane_1.m_distance * plane_2.m_normal - plane_2.m_distance * plane_1.m_normal, direction) / denom;
		return Geometry::Line{point_on_intersection_line, direction, false};
	}
	std::optional<Geometry::Point> get_intersection(const Plane& plane_1, const Plane& plane_2, const Plane& plane_3)
	{
		const glm::vec3 u = glm::cross(plane_2.m_normal, plane_3.m_normal);
		const float denom = glm::dot(plane_1.m_normal, u);

		if (std::abs(denom) < Epsilon)
			return std::nullopt;
		else
			return Geometry::Point{plane_1.m_distance * u + glm::cross(plane_1.m_normal, plane_3.m_distance * plane_2.m_normal - plane_2.m_distance * plane_3.m_normal) / denom};
	}
	std::optional<Geometry::LineSegment> get_intersection(const Sphere& sphere_1, const Sphere& sphere_2)
	{
		// Returns the line segment that is the intersection of two spheres.
		auto distance_between_centers = glm::distance(sphere_1.m_center, sphere_2.m_center);
		auto radius_sum               = sphere_1.m_radius + sphere_2.m_radius;
		auto overlap_distance         = radius_sum - distance_between_centers;

		if (overlap_distance >= 0.0f) // Allow for spheres touching
		{
			auto mid_point    = (sphere_1.m_center + sphere_2.m_center) / 2.0f;
			auto direction    = glm::normalize(sphere_2.m_center - sphere_1.m_center);
			auto half_overlap = (overlap_distance / 2.0f) * direction;
			auto start_point  = mid_point - half_overlap;
			auto end_point    = mid_point + half_overlap;
			return LineSegment{start_point, end_point};
		}
		else
			return std::nullopt; // No intersection, the spheres are separate.
	}
	bool intersecting(const AABB& AABB_1, const AABB& AABB_2)
	{
		// Reference: Real-Time Collision Detection (Christer Ericson)
		// Exit with no intersection if separated along an axis, overlapping on all axes means AABBs are intersecting
		if (AABB_1.mMax[0] < AABB_2.mMin[0] || AABB_1.mMin[0] > AABB_2.mMax[0]
		 || AABB_1.mMax[1] < AABB_2.mMin[1] || AABB_1.mMin[1] > AABB_2.mMax[1]
		 || AABB_1.mMax[2] < AABB_2.mMin[2] || AABB_1.mMin[2] > AABB_2.mMax[2])
			return false;
		else
			return true;
	}
	bool intersecting(const AABB& AABB, const Point& point)
	{// Is point inside or on the surface of AABB?
		return (point.m_position.x >= AABB.mMin.x && point.m_position.x <= AABB.mMax.x
		     && point.m_position.y >= AABB.mMin.y && point.m_position.y <= AABB.mMax.y
		     && point.m_position.z >= AABB.mMin.z && point.m_position.z <= AABB.mMax.z);
	}
	bool intersecting(const AABB& AABB, const Ray& ray)
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
			if (std::abs(ray.m_direction[i]) < EPSILON)
			{
				// Ray is parallel to slab. No hit if origin not within slab
				if (ray.m_start[i] < AABB.mMin[i] || ray.m_start[i] > AABB.mMax[i])
					return false;
			}
			else
			{
				// Compute intersection values along ray with near and far plane of slab on i axis
				const float ood = 1.0f / ray.m_direction[i];
				float entry = (AABB.mMin[i] - ray.m_start[i]) * ood;
				float exit = (AABB.mMax[i] - ray.m_start[i]) * ood;

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

		// Ray intersects all 3 slabs.
		return true;
	}
	bool intersecting(const Cone& cone, const Point& point)
	{
		auto cone_height = glm::distance(cone.m_base, cone.m_top);

		if (cone_height == 0.0f) // 0 height cone has no volume, so point cannot be inside it.
			return false;

		auto cone_direction = glm::normalize(cone.m_top - cone.m_base);
		// Project tip_to_p onto the cone axis to get the point's distance along the axis
		auto point_distance_along_axis = glm::dot(point.m_position - cone.m_base, cone_direction);

		// Is the point orthogonally within the bounds of the cone's axis.
		if (point_distance_along_axis >= 0.f && point_distance_along_axis <= cone_height)
		{
			// The distance from the point to the cone's axis.
			auto orthogonal_distance = glm::distance(point.m_position, cone.m_base + (cone_direction * point_distance_along_axis));
			// The radius of the cone at the point's distance along the axis.
			auto cone_radius = cone.m_base_radius * ((cone_height - point_distance_along_axis) / cone_height);
			// The point is inside the cone if the orthogonal distance is less than the cone's radius at that point along it's axis.
			return orthogonal_distance <= cone_radius;
		}
		else
			return false; // Outside the cone
	}
	bool intersecting(const Cylinder& cylinder, const Point& point)
	{
		auto cylinder_height = glm::distance(cylinder.m_base, cylinder.m_top);

		if (cylinder_height == 0.0f) // 0 height cylinder has no volume, so point cannot be inside it.
			return false;

		auto cylinder_direction = glm::normalize(cylinder.m_top - cylinder.m_base);
		// Project tip_to_p onto the cylinder axis to get the point's distance along the axis
		auto point_distance_along_axis = glm::dot(point.m_position - cylinder.m_base, cylinder_direction);

		// Is the point orthogonally within the bounds of the cylinder's axis.
		if (point_distance_along_axis >= 0.f && point_distance_along_axis <= cylinder_height)
		{
			// The distance from the point to the cylinder's axis.
			auto orthogonal_distance = glm::distance(point.m_position, cylinder.m_base + (cylinder_direction * point_distance_along_axis));
			// The point is inside the cylinder if the orthogonal distance is less than the cylinder's radius.
			return orthogonal_distance <= cylinder.m_radius;
		}
		else
			return false; // Outside the cylinder
	}
	bool intersecting(const Line& line, const Triangle& triangle)
	{
		// Below works for a double-sided triangle (both CW or CCW depending on which side it is viewed),
		// line passes on the inside if all three scalar triple products (u,v,w) have the same sign (ignoring zeroes).
		// The code does not handle the case when line is in the same plane as triangle.

		const glm::vec3 pq = line.m_point_2 - line.m_point_1;
		const glm::vec3 pa = triangle.m_point_1 - line.m_point_1;
		const glm::vec3 pb = triangle.m_point_2 - line.m_point_1;
		const glm::vec3 pc = triangle.m_point_3 - line.m_point_1;

		const glm::vec3 m = glm::cross(pq, pc); // m allows us to avoid an extra cross product below.
		const auto u      = glm::dot(pb, m);    // triple_product(pq, pc, pb);
		const auto v      = -glm::dot(pa, m);   // triple_product(pq, pa, pc);
		const auto w      = triple_product(pq, pb, pa);

		if (u == 0 && v == 0 && w == 0)
			ASSERT(false, "[INTERSECT] Line is in the plane of the triangle. This isn't handled yet (intersect_line_line).");

		return (u <= 0.f && v <= 0.f && w <= 0.f) || (u >= 0.f && v >= 0.f && w >= 0.f); // have the same sign (ignoring zeroes)
	}
	bool intersecting(const Plane& plane_1, const Plane& plane_2)
	{
		// If the dot product is equal to zero, the planes are parallel and do not intersect
		if (glm::dot(plane_1.m_normal, plane_2.m_normal) == 0.0f)
			return false;
		else
			return true;
	}
	bool intersecting(const Sphere& sphere_1, const Sphere& sphere_2)
	{
		// Returns true if the spheres are intersecting.
		auto distance_between_centers = glm::distance(sphere_1.m_center, sphere_2.m_center);
		auto radius_sum               = sphere_1.m_radius + sphere_2.m_radius;
		return distance_between_centers <= radius_sum;
	}
	bool intersecting(const Triangle& triangle_1, const Triangle& triangle_2, bool test_co_planar)
	{
		// Uses the Möller-Trumbore intersection algorithm to perform triangle-triangle collision detection. Adapted from:
		// https://github.com/erich666/jgt-code/blob/master/Volume_08/Number_1/Shen2003/tri_tri_test/include/Moller97.c
		// https://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf

		// compute plane of triangle_1
		glm::vec3 E1 = triangle_1.m_point_2 - triangle_1.m_point_1;
		glm::vec3 E2 = triangle_1.m_point_3 - triangle_1.m_point_1;
		glm::vec3 N1 = glm::cross(E1, E2); // Normal of triangle_1
		float d1 = -glm::dot(N1, triangle_1.m_point_1);
		// plane equation 1: N1.X+d1=0

		// Put triangle_2 into plane equation 1 to compute signed distances to the plane
		float du0 = glm::dot(N1, triangle_2.m_point_1) + d1;
		float du1 = glm::dot(N1, triangle_2.m_point_2) + d1;
		float du2 = glm::dot(N1, triangle_2.m_point_3) + d1;

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

		// compute plane of triangle_2
		E1 = triangle_2.m_point_2 - triangle_2.m_point_1;
		E2 = triangle_2.m_point_3 - triangle_2.m_point_1;
		glm::vec3 N2 = glm::cross(E1, E2); // Tr Normal
		float d2 = -glm::dot(N2, triangle_2.m_point_1);
		// plane equation 2: N2.X+d2=0

		// put triangle_1 into plane equation of triangle_2
		float dv0 = glm::dot(N2, triangle_1.m_point_1) + d2;
		float dv1 = glm::dot(N2, triangle_1.m_point_2) + d2;
		float dv2 = glm::dot(N2, triangle_1.m_point_3) + d2;

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
		float vp0 = triangle_1.m_point_1[index];
		float vp1 = triangle_1.m_point_2[index];
		float vp2 = triangle_1.m_point_3[index];

		float up0 = triangle_2.m_point_1[index];
		float up1 = triangle_2.m_point_2[index];
		float up2 = triangle_2.m_point_3[index];

		glm::vec2 isect1;
		glm::vec2 isect2;

		// compute interval for triangle_1 and triangle_2. If the interval check comes back false
		// the triangles are coplanar and we can early out by checking for collision between coplanar triangles.
		if (!compute_intervals(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, isect1[0], isect1[1]))
		{
			if (test_co_planar)
				return coplanar_tri_tri(N1, triangle_1, triangle_2);
			else
				return false;
		}
		if (!compute_intervals(up0, up1, up2, du0, du1, du2, du0du1, du0du2, isect2[0], isect2[1]))
		{
			if (test_co_planar)
				return coplanar_tri_tri(N1, triangle_1, triangle_2);
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

} // namespace Geometry