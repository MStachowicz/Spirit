#include "Intersect.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Cone.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/TriTri.hpp"
#include "Geometry/Triangle.hpp"

#include "Utility/Logger.hpp"

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

// ==============================================================================================================================
// END UTILITIY FUNCTIONS
// ==============================================================================================================================


// ==============================================================================================================================
// POINT_INSIDE + CLOSEST_POINT FUNCTIONS
// ==============================================================================================================================

	bool point_inside(const AABB& AABB, const glm::vec3& point)
	{
		return (point.x >= AABB.m_min.x && point.x <= AABB.m_max.x
		     && point.y >= AABB.m_min.y && point.y <= AABB.m_max.y
		     && point.z >= AABB.m_min.z && point.z <= AABB.m_max.z);
	}
	bool point_inside(const Cone& cone, const glm::vec3& point)
	{
		auto cone_height = glm::distance(cone.m_base, cone.m_top);

		if (cone_height == 0.0f) // 0 height cone has no volume, so point cannot be inside it.
			return false;

		auto cone_direction = glm::normalize(cone.m_top - cone.m_base);
		// Project tip_to_p onto the cone axis to get the point's distance along the axis
		auto point_distance_along_axis = glm::dot(point - cone.m_base, cone_direction);

		// Is the point orthogonally within the bounds of the cone's axis.
		if (point_distance_along_axis >= 0.f && point_distance_along_axis <= cone_height)
		{
			// The distance from the point to the cone's axis.
			auto orthogonal_distance = glm::distance(point, cone.m_base + (cone_direction * point_distance_along_axis));
			// The radius of the cone at the point's distance along the axis.
			auto cone_radius = cone.m_base_radius * ((cone_height - point_distance_along_axis) / cone_height);
			// The point is inside the cone if the orthogonal distance is less than the cone's radius at that point along it's axis.
			return orthogonal_distance <= cone_radius;
		}
		else
			return false; // Outside the cone
	}
	bool point_inside(const Cylinder& cylinder, const glm::vec3& point)
	{
		auto cylinder_height = glm::distance(cylinder.m_base, cylinder.m_top);

		if (cylinder_height == 0.0f) // 0 height cylinder has no volume, so point cannot be inside it.
			return false;

		auto cylinder_direction = glm::normalize(cylinder.m_top - cylinder.m_base);
		// Project tip_to_p onto the cylinder axis to get the point's distance along the axis
		auto point_distance_along_axis = glm::dot(point - cylinder.m_base, cylinder_direction);

		// Is the point orthogonally within the bounds of the cylinder's axis.
		if (point_distance_along_axis >= 0.f && point_distance_along_axis <= cylinder_height)
		{
			// The distance from the point to the cylinder's axis.
			auto orthogonal_distance = glm::distance(point, cylinder.m_base + (cylinder_direction * point_distance_along_axis));
			// The point is inside the cylinder if the orthogonal distance is less than the cylinder's radius.
			return orthogonal_distance <= cylinder.m_radius;
		}
		else
			return false; // Outside the cylinder
	}
	bool point_inside(const Line& line, const glm::vec3& point)
	{
		// Calculate vectors along the line and to the point
		glm::vec3 AB = line.m_point_2    - line.m_point_1;
		glm::vec3 AP = point  - line.m_point_1;

		// Calculate the cross product of the two vectors (area of parallelogram with AB and AP as sides)
		glm::vec3 cross_AB_AP = glm::cross(AB, AP);

		// If the cross product is (0, 0, 0), the point is on the line
		return cross_AB_AP == glm::vec3(0.f);
	}
	bool point_inside(const LineSegment& lineSegment, const glm::vec3& point)
	{
		// Calculate vectors along the lineSegment and to the point
		glm::vec3 AB = lineSegment.m_end - lineSegment.m_start;
		glm::vec3 AP = point  - lineSegment.m_start;

		// Calculate the cross product of the two vectors (area of parallelogram with AB and AP as sides)
		glm::vec3 cross_AB_AP = glm::cross(AB, AP);

		// If the cross product is (0, 0, 0), the point is on the line
		if (cross_AB_AP != glm::vec3(0.0f))
			return false;

		// Check if the point is within the line segment
		float dot_AB_AP = glm::dot(AB, AP);
		return dot_AB_AP >= 0.0f && dot_AB_AP <= glm::dot(AB, AB);
	}
	bool point_inside(const Ray& ray, const glm::vec3& point)
	{
		// Calculate vectors along the ray and to the point
		glm::vec3 AB = ray.m_direction;
		glm::vec3 AP = point - ray.m_start;

		// Calculate the cross product of the two vectors (area of parallelogram with AB and AP as sides)
		glm::vec3 cross_AB_AP = glm::cross(AB, AP);

		// If the cross product is (0, 0, 0), the point is along the ray direction
		if (cross_AB_AP != glm::vec3(0.0f))
			return false;

		// Check if the point is ahead of or on the ray start
		float dot_AB_AP = glm::dot(AB, AP);
		return dot_AB_AP >= 0.0f;
	}

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
	glm::vec3 barycentric(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& p)
	{
		glm::vec3 v0 = b - a;
		glm::vec3 v1 = c - a;
		glm::vec3 v2 = p - a;

		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;

		auto v = (d11 * d20 - d01 * d21) / denom;
		auto w = (d00 * d21 - d01 * d20) / denom;
		auto u = 1.0f - v - w;

		return glm::vec3(u, v, w);
	}
	glm::vec3 closest_point(const Triangle& triangle, const glm::vec3& point)
	{
		const glm::vec3& a = triangle.m_point_1;
		const glm::vec3& b = triangle.m_point_2;
		const glm::vec3& c = triangle.m_point_3;
		const glm::vec3& p = point;

		// Check if P in vertex region outside A
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ap = p - a;
		float d1     = glm::dot(ab, ap);
		float d2     = glm::dot(ac, ap);
		if (d1 <= 0.0f && d2 <= 0.0f)
			return a; // barycentric coordinates (1, 0, 0)

		// Check if P in vertex region outside B
		glm::vec3 bp = p - b;
		float d3     = glm::dot(ab, bp);
		float d4     = glm::dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3)
			return b; // barycentric coordinates (0, 1, 0)

		// Check if P in edge region of AB, if so return projection of P onto AB
		float vc = d1 * d4 - d3 * d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			float v = d1 / (d1 - d3);
			return a + v * ab; // barycentric coordinates (1-v, v, 0)
		}

		// Check if P in vertex region outside C
		glm::vec3 cp = p - c;
		float d5     = glm::dot(ab, cp);
		float d6     = glm::dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6)
			return c; // barycentric coordinates (0, 0, 1)

		// Check if P in edge region of AC, if so return projection of P onto AC
		float vb = d5 * d2 - d1 * d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			float w = d2 / (d2 - d6);
			return a + w * ac; // barycentric coordinates (1-w, 0, w)
		}

		// Check if P in edge region of BC, if so return projection of P onto BC
		float va = d3 * d6 - d5 * d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			return b + w * (c - b); // barycentric coordinates (0, 1-w, w)
		}

		// P inside face region. Compute Q through its barycentric coordinates (u, v, w)
		float denom = 1.0f / (va + vb + vc);
		float v     = vb * denom;
		float w     = vc * denom;
		return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w
	}
	float distance_squared(const LineSegment& line, const glm::vec3& point)
	{
		auto AB = line.m_end - line.m_start;
		auto AP = point      - line.m_start;
		float dot_AP_AB = glm::dot(AP, AB);

		// Handle cases where c projects outside AB
		if (dot_AP_AB <= 0.0f)
			return glm::dot(AP, AP);

		float f = glm::dot(AB, AB);
		if (dot_AP_AB >= f)
		{
			auto bc = point - line.m_end;
			return glm::dot(bc, bc);
		}

		// Handle cases where c projects onto AB
		return glm::dot(AP, AP) - dot_AP_AB * dot_AP_AB / f;
	}
	float distance(const LineSegment& line, const glm::vec3& point)
	{
		return std::sqrt(distance_squared(line, point));
	}
	float distance(const Plane& plane, const glm::vec3& point)
	{
		return glm::dot(plane.m_normal, point) - plane.m_distance;
	}

// ==============================================================================================================================
// END POINT_INSIDE FUNCTIONS
// ==============================================================================================================================

	std::optional<glm::vec3> get_intersection(const Ray& ray, const Plane& plane)
	{
		// Compute the dot product between the ray direction and the plane normal
		float dot_direction_normal = glm::dot(ray.m_direction, plane.m_normal);

		// If the dot product is zero, the ray is parallel to the plane
		if (std::abs(dot_direction_normal) < Epsilon)
			return std::nullopt;

		// Compute the distance from the ray origin to the plane
		float distance = (plane.m_distance - glm::dot(ray.m_start, plane.m_normal)) / dot_direction_normal;

		// If the distance is negative, the plane is behind the ray
		if (distance < 0.0f)
			return std::nullopt;

		// Compute the intersection point
		return ray.m_start + (ray.m_direction * distance);

	}

	std::optional<glm::vec3> get_intersection(const AABB& AABB, const Ray& ray, float* distance_along_ray)
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
				if (ray.m_start[i] < AABB.m_min[i] || ray.m_start[i] > AABB.m_max[i])
					return std::nullopt;
			}
			else
			{
				// Compute intersection values along ray with near and far plane of slab on i axis
				const float ood = 1.0f / ray.m_direction[i];
				float entry = (AABB.m_min[i] - ray.m_start[i]) * ood;
				float exit = (AABB.m_max[i] - ray.m_start[i]) * ood;

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

		// Ray intersects all 3 slabs. Return point intersection_point and length_along_ray
		if (distance_along_ray)
			*distance_along_ray = farthestEntry;

		return ray.m_start + (ray.m_direction * farthestEntry);
	}
	std::optional<glm::vec3> get_intersection(const Line& line, const Triangle& triangle)
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

			return (u * triangle.m_point_1) + (v * triangle.m_point_2) + (w * triangle.m_point_3);
		}
		else
			return std::nullopt;
	}
	std::optional<Line> get_intersection(const Plane& plane_1, const Plane& plane_2)
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
		return Line(point_on_intersection_line, direction);
	}
	std::optional<glm::vec3> get_intersection(const Plane& plane, const Sphere& sphere)
	{
		// Compute the distance from the sphere center to the plane
		float distance = glm::dot(plane.m_normal, sphere.m_center) - plane.m_distance;

		// If the absolute value of the distance is less than the sphere radius, then the sphere is colliding with the plane
		if (std::abs(distance) < sphere.m_radius)
		{
			// The normal of the contact is the normal of the plane, but it must be reversed if the sphere is behind the plane
			glm::vec3 normal = plane.m_normal;
			if (distance < 0)
				normal = -normal;

			// The position of the contact is point on the sphere surface towards the plane
			return sphere.m_center - normal * sphere.m_radius;
		}
		else
			return std::nullopt;
	}
	std::optional<glm::vec3> get_intersection(const Plane& plane_1, const Plane& plane_2, const Plane& plane_3)
	{
		const glm::vec3 u = glm::cross(plane_2.m_normal, plane_3.m_normal);
		const float denom = glm::dot(plane_1.m_normal, u);

		if (std::abs(denom) < Epsilon)
			return std::nullopt;
		else
			return plane_1.m_distance * u + glm::cross(plane_1.m_normal, plane_3.m_distance * plane_2.m_normal - plane_2.m_distance * plane_3.m_normal) / denom;
	}
	std::optional<glm::vec3> get_intersection(const Sphere& sphere_1, const Sphere& sphere_2)
	{
		// Compute the displacement, or the distance between the two spheres
		glm::vec3 displacement = sphere_1.m_center - sphere_2.m_center;
		float distance = glm::length(displacement);

		// If the distance is less than the sum of the two radii, then the spheres are colliding
		if (distance <= sphere_1.m_radius + sphere_2.m_radius)
		{
			// The normal of the contact is the normalized displacement
			// If the spheres are in the same position (distance == 0.f), the normal is arbitrary, we choose up here.
			glm::vec3 normal = distance == 0.f ? glm::vec3(0.f, 1.f, 0.f) : displacement / distance;
			// The position of the contact is point on the surface of sphere_1 towards sphere_2
			return sphere_1.m_center - normal * sphere_1.m_radius;
		}
		else
			return std::nullopt;
	}
	std::optional<glm::vec3> get_intersection(const Triangle& triangle_1, const Triangle& triangle_2)
	{
		if (triangle_1.is_degenerate() || triangle_2.is_degenerate())
			return std::nullopt;

		auto intersection_segment = triangle_triangle(triangle_1, triangle_2);
		if (intersection_segment)
		{// Halfway point on the line segment
			return intersection_segment->m_start + ((intersection_segment->m_end - intersection_segment->m_start) * 0.5f);
		}
		else
			return std::nullopt;
	}
	std::optional<LineSegment> triangle_triangle(const Triangle& triangle_1, const Triangle& triangle_2, bool* is_coplanar)
	{
		bool coplanar           = false;
		LineSegment lineSegment = LineSegment(glm::vec3(0.f), glm::vec3(0.f));
		bool intersecting       = tri_tri_get_intersection(triangle_1.m_point_1, triangle_1.m_point_2, triangle_1.m_point_3, triangle_2.m_point_1, triangle_2.m_point_2, triangle_2.m_point_3,
		                                                   coplanar, lineSegment.m_start, lineSegment.m_end);

		if (coplanar && is_coplanar)
			*is_coplanar = true;

		if (intersecting)
			return lineSegment; // If coplanar, the line
		else
			return std::nullopt;
	}
	bool intersecting(const AABB& AABB_1, const AABB& AABB_2)
	{
		// Reference: Real-Time Collision Detection (Christer Ericson)
		// Exit with no intersection if separated along an axis, overlapping on all axes means AABBs are intersecting
		if (AABB_1.m_max[0] < AABB_2.m_min[0] || AABB_1.m_min[0] > AABB_2.m_max[0]
		 || AABB_1.m_max[1] < AABB_2.m_min[1] || AABB_1.m_min[1] > AABB_2.m_max[1]
		 || AABB_1.m_max[2] < AABB_2.m_min[2] || AABB_1.m_min[2] > AABB_2.m_max[2])
			return false;
		else
			return true;
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
				if (ray.m_start[i] < AABB.m_min[i] || ray.m_start[i] > AABB.m_max[i])
					return false;
			}
			else
			{
				// Compute intersection values along ray with near and far plane of slab on i axis
				const float ood = 1.0f / ray.m_direction[i];
				float entry = (AABB.m_min[i] - ray.m_start[i]) * ood;
				float exit = (AABB.m_max[i] - ray.m_start[i]) * ood;

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
	bool intersecting(const Plane& plane, const Sphere& sphere)
	{
		// For a normalized plane (|p.n| = 1), evaluating the plane equation for a point gives the signed distance of the point to the plane
		float dist = glm::dot(sphere.m_center, plane.m_normal) - plane.m_distance;
		// If sphere center within +/-radius from plane, plane intersects sphere
		return std::abs(dist) <= sphere.m_radius;
	}
	bool intersecting(const Sphere& sphere_1, const Sphere& sphere_2)
	{
		// Returns true if the spheres are intersecting.
		auto distance_between_centers = glm::distance(sphere_1.m_center, sphere_2.m_center);
		auto radius_sum               = sphere_1.m_radius + sphere_2.m_radius;
		return distance_between_centers <= radius_sum;
	}
	bool intersecting(const Triangle& triangle_1, const Triangle& triangle_2)
	{
		if (triangle_1.is_degenerate() || triangle_2.is_degenerate())
			return false;

		return tri_tri_is_intersecting(triangle_1.m_point_1, triangle_1.m_point_2, triangle_1.m_point_3,
		                               triangle_2.m_point_1, triangle_2.m_point_2, triangle_2.m_point_3);
	}
} // namespace Geometry