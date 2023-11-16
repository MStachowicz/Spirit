#pragma once

#include "Geometry/Line.hpp"
#include "glm/vec3.hpp"
#include "Utility/Logger.hpp"
#include <optional>

namespace Geometry
{
	class AABB;
	class Cone;
	class Cuboid;
	class Cylinder;
	class LineSegment;
	class Plane;
	class Quad;
	class Ray;
	class Sphere;
	class Triangle;

	// Returns the scalar triple product of u, v and w. Also referred to as the box product.
	// Geometrically, the value of the scalar triple product corresponds to the (signed) volume of a parallelepiped formed by the three independent vectors u, v, and w.
	// The special notation [uvw] is often used to denote a triple product. [uvw] == (u x v)·w
	// Real-Time Collision Detection (Christer Ericson) - 3.3.7 The Scalar Triple Product pg 44
	float triple_product(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w);



// ==============================================================================================================================
// Intersection types (Types of intersection based on the shapes being intersected)
// ==============================================================================================================================

	// Returned when a Line, LineSegment or Ray are intersecting at a given point.
	struct LineIntersection
	{
		glm::vec3 intersection_point; // The point where the Line/LineSegment/Ray intersects the plane.
		float length_along_ray;       // The length along the ray where the intersection point is located.
	};
	// Returned when two planes intersect.
	struct PlanePlaneIntersection
	{
		Line line; // The line along which the two planes intersect.
	};
	struct PointIntersection
	{
		glm::vec3 point; // The point where the intersection occurs.

		constexpr operator glm::vec3&() noexcept             { return point; }
		constexpr operator const glm::vec3&() const noexcept { return point; }
	};


//==============================================================================================================================
// get_intersection functions = Where and how are we intersecting
//==============================================================================================================================

	// AABB functions
	//==============================================================================================================================
	inline std::optional<bool>      get_intersection(const AABB& AABB_1, const AABB& AABB_2)             { LOG_WARN("[INTERSECT] Not implemented AABB v AABB"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Cone& cone)               { LOG_WARN("[INTERSECT] Not implemented AABB v Cone"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Cuboid& cuboid)           { LOG_WARN("[INTERSECT] Not implemented AABB v Cuboid"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Cylinder& cylinder)       { LOG_WARN("[INTERSECT] Not implemented AABB v Cylinder"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Line& line)               { LOG_WARN("[INTERSECT] Not implemented AABB v Line"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const LineSegment& lineSegment) { LOG_WARN("[INTERSECT] Not implemented AABB v LineSegment"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Plane& plane)             { LOG_WARN("[INTERSECT] Not implemented AABB v Plane"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Quad& quad)               { LOG_WARN("[INTERSECT] Not implemented AABB v Quad"); return std::nullopt; } // #TODO
	std::optional<LineIntersection> get_intersection(const AABB& AABB,   const Ray& ray);                // IMPLEMENTED
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented AABB v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool>      get_intersection(const AABB& AABB,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented AABB v Triangle"); return std::nullopt; } // #TODO

	// Cone functions
	//==============================================================================================================================
	inline std::optional<bool> get_intersection(const Cone& cone,   const AABB& AABB)               { return get_intersection(AABB, cone); }
	inline std::optional<bool> get_intersection(const Cone& cone_1, const Cone& cone_2)             { LOG_WARN("[INTERSECT] Not implemented Cone v Cone"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Cuboid& cuboid)           { LOG_WARN("[INTERSECT] Not implemented Cone v Cuboid"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Cylinder& cylinder)       { LOG_WARN("[INTERSECT] Not implemented Cone v Cylinder"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Line& line)               { LOG_WARN("[INTERSECT] Not implemented Cone v Line"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const LineSegment& lineSegment) { LOG_WARN("[INTERSECT] Not implemented Cone v LineSegment"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Plane& plane)             { LOG_WARN("[INTERSECT] Not implemented Cone v Plane"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Quad& quad)               { LOG_WARN("[INTERSECT] Not implemented Cone v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Ray& ray)                 { LOG_WARN("[INTERSECT] Not implemented Cone v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented Cone v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cone& cone,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented Cone v Triangle"); return std::nullopt; } // #TODO

	// Cuboid functions
	//==============================================================================================================================
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const AABB& AABB)               { return get_intersection(AABB, cuboid); }
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Cone& cone)               { return get_intersection(cone, cuboid); }
	inline std::optional<bool> get_intersection(const Cuboid& cuboid_1, const Cuboid& cuboid_2)         { LOG_WARN("[INTERSECT] Not implemented Cuboid v Cuboid"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Cylinder& cylinder)       { LOG_WARN("[INTERSECT] Not implemented Cuboid v Cylinder"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Line& line)               { LOG_WARN("[INTERSECT] Not implemented Cuboid v Line"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const LineSegment& lineSegment) { LOG_WARN("[INTERSECT] Not implemented Cuboid v LineSegment"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Plane& plane)             { LOG_WARN("[INTERSECT] Not implemented Cuboid v Plane"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Quad& quad)               { LOG_WARN("[INTERSECT] Not implemented Cuboid v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Ray& ray)                 { LOG_WARN("[INTERSECT] Not implemented Cuboid v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented Cuboid v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cuboid& cuboid,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented Cuboid v Triangle"); return std::nullopt; } // #TODO

	// Cylinder functions
	//==============================================================================================================================
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const AABB& AABB)               { return get_intersection(AABB, cylinder); }
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Cone& cone)               { return get_intersection(cone, cylinder); }
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Cuboid& cuboid)           { return get_intersection(cuboid, cylinder); }
	inline std::optional<bool> get_intersection(const Cylinder& cylinder_1, const Cylinder& cylinder_2)     { LOG_WARN("[INTERSECT] Not implemented Cylinder v Cylinder"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Line& line)               { LOG_WARN("[INTERSECT] Not implemented Cylinder v Line"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const LineSegment& lineSegment) { LOG_WARN("[INTERSECT] Not implemented Cylinder v LineSegment"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Plane& plane)             { LOG_WARN("[INTERSECT] Not implemented Cylinder v Plane"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Quad& quad)               { LOG_WARN("[INTERSECT] Not implemented Cylinder v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Ray& ray)                 { LOG_WARN("[INTERSECT] Not implemented Cylinder v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented Cylinder v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Cylinder& cylinder,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented Cylinder v Triangle"); return std::nullopt; } // #TODO

	// Line functions
	//==============================================================================================================================
	inline std::optional<bool>       get_intersection(const Line& line,   const AABB& AABB)               { return get_intersection(AABB, line); }
	inline std::optional<bool>       get_intersection(const Line& line,   const Cone& cone)               { return get_intersection(cone, line); }
	inline std::optional<bool>       get_intersection(const Line& line,   const Cuboid& cuboid)           { return get_intersection(cuboid, line); }
	inline std::optional<bool>       get_intersection(const Line& line,   const Cylinder& cylinder)       { return get_intersection(cylinder, line); }
	inline std::optional<bool>       get_intersection(const Line& line_1, const Line& line_2)             { LOG_WARN("[INTERSECT] Not implemented Line v Line"); return std::nullopt; } // #TODO
	inline std::optional<bool>       get_intersection(const Line& line,   const LineSegment& lineSegment) { LOG_WARN("[INTERSECT] Not implemented Line v LineSegment"); return std::nullopt; } // #TODO
	inline std::optional<bool>       get_intersection(const Line& line,   const Plane& plane)             { LOG_WARN("[INTERSECT] Not implemented Line v Plane"); return std::nullopt; } // #TODO
	inline std::optional<bool>       get_intersection(const Line& line,   const Quad& quad)               { LOG_WARN("[INTERSECT] Not implemented Line v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool>       get_intersection(const Line& line,   const Ray& ray)                 { LOG_WARN("[INTERSECT] Not implemented Line v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool>       get_intersection(const Line& line,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented Line v Sphere"); return std::nullopt; } // #TODO
	std::optional<PointIntersection> get_intersection(const Line& line,   const Triangle& triangle);       // IMPLEMENTED

	// LineSegment functions
	//==============================================================================================================================
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const AABB& AABB)                 { return get_intersection(AABB, lineSegment); }
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Cone& cone)                 { return get_intersection(cone, lineSegment); }
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Cuboid& cuboid)             { return get_intersection(cuboid, lineSegment); }
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Cylinder& cylinder)         { return get_intersection(cylinder, lineSegment); }
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Line& line)                 { return get_intersection(line, lineSegment); }
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment_1, const LineSegment& lineSegment_2) { LOG_WARN("[INTERSECT] Not implemented LineSegment v LineSegment"); return std::nullopt; } // #TODO
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Plane& plane)               { LOG_WARN("[INTERSECT] Not implemented LineSegment v Plane"); return std::nullopt; } // #TODO
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Quad& quad)                 { LOG_WARN("[INTERSECT] Not implemented LineSegment v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Ray& ray)                   { LOG_WARN("[INTERSECT] Not implemented LineSegment v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool>              get_intersection(const LineSegment& lineSegment,   const Sphere& sphere)             { LOG_WARN("[INTERSECT] Not implemented LineSegment v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<PointIntersection> get_intersection(const LineSegment& lineSegment,   const Triangle& triangle)         { LOG_WARN("[INTERSECT] Not implemented LineSegment v Triangle"); return std::nullopt; } // #TODO

	// Plane functions
	//==============================================================================================================================
	inline std::optional<bool>            get_intersection(const Plane& plane,   const AABB& AABB)                            { return get_intersection(AABB, plane); }
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Cone& cone)                            { return get_intersection(cone, plane); }
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Cuboid& cuboid)                        { return get_intersection(cuboid, plane); }
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Cylinder& cylinder)                    { return get_intersection(cylinder, plane); }
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Line& line)                            { return get_intersection(line, plane); }
	inline std::optional<bool>            get_intersection(const Plane& plane,   const LineSegment& lineSegment)              { return get_intersection(lineSegment, plane); }
	std::optional<PlanePlaneIntersection> get_intersection(const Plane& plane_1, const Plane& plane_2);                       // IMPLEMENTED
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Quad& quad)                            { LOG_WARN("[INTERSECT] Not implemented Plane v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Ray& ray)                              { LOG_WARN("[INTERSECT] Not implemented Plane v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Sphere& sphere)                        { LOG_WARN("[INTERSECT] Not implemented Plane v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool>            get_intersection(const Plane& plane,   const Triangle& triangle)                    { LOG_WARN("[INTERSECT] Not implemented Plane v Triangle"); return std::nullopt; } // #TODO
	std::optional<PointIntersection>      get_intersection(const Plane& plane_1, const Plane& plane_2, const Plane& plane_3); // IMPLEMENTED

	// Quad functions
	//==============================================================================================================================
	inline std::optional<bool> get_intersection(const Quad& quad,   const AABB& AABB)               { return get_intersection(AABB, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad,   const Cone& cone)               { return get_intersection(cone, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad,   const Cuboid& cuboid)           { return get_intersection(cuboid, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad,   const Cylinder& cylinder)       { return get_intersection(cylinder, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad,   const Line& line)               { return get_intersection(line, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad,   const LineSegment& lineSegment) { return get_intersection(lineSegment, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad,   const Plane& plane)             { return get_intersection(plane, quad); }
	inline std::optional<bool> get_intersection(const Quad& quad_1, const Quad& quad_2)             { LOG_WARN("[INTERSECT] Not implemented Quad v Quad"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Quad& quad,   const Ray& ray)                 { LOG_WARN("[INTERSECT] Not implemented Quad v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Quad& quad,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented Quad v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Quad& quad,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented Quad v Triangle"); return std::nullopt; } // #TODO

	// Ray functions
	//==============================================================================================================================
	inline std::optional<LineIntersection>  get_intersection(const Ray& ray,   const AABB& AABB)               { return get_intersection(AABB, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Cone& cone)               { return get_intersection(cone, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Cuboid& cuboid)           { return get_intersection(cuboid, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Cylinder& cylinder)       { return get_intersection(cylinder, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Line& line)               { return get_intersection(line, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const LineSegment& lineSegment) { return get_intersection(lineSegment, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Plane& plane)             { return get_intersection(plane, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Quad& quad)               { return get_intersection(quad, ray); }
	inline std::optional<bool>              get_intersection(const Ray& ray_1, const Ray& ray_2)               { LOG_WARN("[INTERSECT] Not implemented Ray v Ray"); return std::nullopt; } // #TODO
	inline std::optional<bool>              get_intersection(const Ray& ray,   const Sphere& sphere)           { LOG_WARN("[INTERSECT] Not implemented Ray v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<PointIntersection> get_intersection(const Ray& ray,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented Ray v Triangle"); return std::nullopt; } // #TODO

	// Sphere functions
	//==============================================================================================================================
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const AABB& AABB)               { return get_intersection(AABB, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Cone& cone)               { return get_intersection(cone, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Cuboid& cuboid)           { return get_intersection(cuboid, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Cylinder& cylinder)       { return get_intersection(cylinder, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Line& line)               { return get_intersection(line, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const LineSegment& lineSegment) { return get_intersection(lineSegment, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Plane& plane)             { return get_intersection(plane, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Quad& quad)               { return get_intersection(quad, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Ray& ray)                 { return get_intersection(ray, sphere); }
	inline std::optional<bool> get_intersection(const Sphere& sphere_1, const Sphere& sphere_2)         { LOG_WARN("[INTERSECT] Not implemented Sphere v Sphere"); return std::nullopt; } // #TODO
	inline std::optional<bool> get_intersection(const Sphere& sphere,   const Triangle& triangle)       { LOG_WARN("[INTERSECT] Not implemented Sphere v Triangle"); return std::nullopt; } // #TODO

	// Triangle functions
	//==============================================================================================================================
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const AABB& AABB)                                        { return get_intersection(AABB, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const Cone& cone)                                        { return get_intersection(cone, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const Cuboid& cuboid)                                    { return get_intersection(cuboid, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const Cylinder& cylinder)                                { return get_intersection(cylinder, triangle); }
	inline std::optional<PointIntersection> get_intersection(const Triangle& triangle,   const Line& line)                                        { return get_intersection(line, triangle); }
	inline std::optional<PointIntersection> get_intersection(const Triangle& triangle,   const LineSegment& lineSegment)                          { return get_intersection(lineSegment, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const Plane& plane)                                      { return get_intersection(plane, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const Quad& quad)                                        { return get_intersection(quad, triangle); }
	inline std::optional<PointIntersection> get_intersection(const Triangle& triangle,   const Ray& ray)                                          { return get_intersection(ray, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle,   const Sphere& sphere)                                    { return get_intersection(sphere, triangle); }
	inline std::optional<bool>              get_intersection(const Triangle& triangle_1, const Triangle& triangle_2, bool test_co_planar = true)  { LOG_WARN("[INTERSECT] Not implemented Triangle v Triangle"); return std::nullopt; } // #TODO
//==============================================================================================================================
// end get_intersection functions
//==============================================================================================================================



//==============================================================================================================================
// intersecting functions = Are we intersecting? - Where not implemented, these are wrappers for get_intersection functions
//==============================================================================================================================

	// AABB functions
	//==============================================================================================================================
	       bool intersecting(const AABB& AABB_1, const AABB& AABB_2);            // IMPLEMENTED
	inline bool intersecting(const AABB& AABB,   const Cone& cone)               { return get_intersection(AABB, cone).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const Cuboid& cuboid)           { return get_intersection(AABB, cuboid).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const Cylinder& cylinder)       { return get_intersection(AABB, cylinder).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const Line& line)               { return get_intersection(AABB, line).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const LineSegment& lineSegment) { return get_intersection(AABB, lineSegment).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const Plane& plane)             { return get_intersection(AABB, plane).has_value(); }       // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const Quad& quad)               { return get_intersection(AABB, quad).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	       bool intersecting(const AABB& AABB,   const Ray& ray);                // IMPLEMENTED
	inline bool intersecting(const AABB& AABB,   const Sphere& sphere)           { return get_intersection(AABB, sphere).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const AABB& AABB,   const Triangle& triangle)       { return get_intersection(AABB, triangle).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Cone functions
	//==============================================================================================================================
	inline bool intersecting(const Cone& cone,   const AABB& AABB)               { return intersecting(AABB, cone); }
	inline bool intersecting(const Cone& cone_1, const Cone& cone_2)             { return get_intersection(cone_1, cone_2).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Cuboid& cuboid)           { return get_intersection(cone, cuboid).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Cylinder& cylinder)       { return get_intersection(cone, cylinder).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Line& line)               { return get_intersection(cone, line).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const LineSegment& lineSegment) { return get_intersection(cone, lineSegment).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Plane& plane)             { return get_intersection(cone, plane).has_value(); }       // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Quad& quad)               { return get_intersection(cone, quad).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Ray& ray)                 { return get_intersection(cone, ray).has_value(); }         // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Sphere& sphere)           { return get_intersection(cone, sphere).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cone& cone,   const Triangle& triangle)       { return get_intersection(cone, triangle).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Cuboid functions
	//==============================================================================================================================
	inline bool intersecting(const Cuboid& cuboid,   const AABB& AABB)               { return intersecting(AABB, cuboid); }
	inline bool intersecting(const Cuboid& cuboid,   const Cone& cone)               { return intersecting(cone, cuboid); }
	inline bool intersecting(const Cuboid& cuboid_1, const Cuboid& cuboid_2)         { return get_intersection(cuboid_1, cuboid_2).has_value(); }  // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Cylinder& cylinder)       { return get_intersection(cuboid, cylinder).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Line& line)               { return get_intersection(cuboid, line).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const LineSegment& lineSegment) { return get_intersection(cuboid, lineSegment).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Plane& plane)             { return get_intersection(cuboid, plane).has_value(); }       // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Quad& quad)               { return get_intersection(cuboid, quad).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Ray& ray)                 { return get_intersection(cuboid, ray).has_value(); }         // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Sphere& sphere)           { return get_intersection(cuboid, sphere).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cuboid& cuboid,   const Triangle& triangle)       { return get_intersection(cuboid, triangle).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Cylinder functions
	//==============================================================================================================================
	inline bool intersecting(const Cylinder& cylinder,   const AABB& AABB)                { return intersecting(AABB, cylinder); }
	inline bool intersecting(const Cylinder& cylinder,   const Cone& cone)                { return intersecting(cone, cylinder); }
	inline bool intersecting(const Cylinder& cylinder,   const Cuboid& cuboid)            { return intersecting(cuboid, cylinder); }
	inline bool intersecting(const Cylinder& cylinder_1, const Cylinder& cylinder_2)      { return get_intersection(cylinder_1, cylinder_2).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const Line& line)                { return get_intersection(cylinder, line).has_value(); }         // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const LineSegment& lineSegment)  { return get_intersection(cylinder, lineSegment).has_value(); }  // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const Plane& plane)              { return get_intersection(cylinder, plane).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const Quad& quad)                { return get_intersection(cylinder, quad).has_value(); }         // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const Ray& ray)                  { return get_intersection(cylinder, ray).has_value(); }          // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const Sphere& sphere)            { return get_intersection(cylinder, sphere).has_value(); }       // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Cylinder& cylinder,   const Triangle& triangle)        { return get_intersection(cylinder, triangle).has_value(); }     // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Line functions
	//==============================================================================================================================
	inline bool intersecting(const Line& line,   const AABB& AABB)                { return intersecting(AABB, line); }
	inline bool intersecting(const Line& line,   const Cone& cone)                { return intersecting(cone, line); }
	inline bool intersecting(const Line& line,   const Cuboid& cuboid)            { return intersecting(cuboid, line); }
	inline bool intersecting(const Line& line,   const Cylinder& cylinder)        { return intersecting(cylinder, line); }
	inline bool intersecting(const Line& line_1, const Line& line_2)              { return get_intersection(line_1, line_2).has_value(); }    // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Line& line,   const LineSegment& lineSegment)  { return get_intersection(line, lineSegment).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Line& line,   const Plane& plane)              { return get_intersection(line, plane).has_value(); }       // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Line& line,   const Quad& quad)                { return get_intersection(line, quad).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Line& line,   const Ray& ray)                  { return get_intersection(line, ray).has_value(); }         // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Line& line,   const Sphere& sphere)            { return get_intersection(line, sphere).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	       bool intersecting(const Line& line,   const Triangle& triangle);       // IMPLEMENTED

	// LineSegment functions
	//==============================================================================================================================
	inline bool intersecting(const LineSegment& lineSegment,   const AABB& AABB)                 { return intersecting(AABB, lineSegment); }
	inline bool intersecting(const LineSegment& lineSegment,   const Cone& cone)                 { return intersecting(cone, lineSegment); }
	inline bool intersecting(const LineSegment& lineSegment,   const Cuboid& cuboid)             { return intersecting(cuboid, lineSegment); }
	inline bool intersecting(const LineSegment& lineSegment,   const Cylinder& cylinder)         { return intersecting(cylinder, lineSegment); }
	inline bool intersecting(const LineSegment& lineSegment,   const Line& line)                 { return intersecting(line, lineSegment); }
	inline bool intersecting(const LineSegment& lineSegment_1, const LineSegment& lineSegment_2) { return get_intersection(lineSegment_1, lineSegment_2).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const LineSegment& lineSegment,   const Plane& plane)               { return get_intersection(lineSegment, plane).has_value(); }           // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const LineSegment& lineSegment,   const Quad& quad)                 { return get_intersection(lineSegment, quad).has_value(); }            // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const LineSegment& lineSegment,   const Ray& ray)                   { return get_intersection(lineSegment, ray).has_value(); }             // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const LineSegment& lineSegment,   const Sphere& sphere)             { return get_intersection(lineSegment, sphere).has_value(); }          // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const LineSegment& lineSegment,   const Triangle& triangle)         { return get_intersection(lineSegment, triangle).has_value(); }        // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Plane functions
	//==============================================================================================================================
	inline bool intersecting(const Plane& plane,   const AABB& AABB)                           { return intersecting(AABB, plane); }
	inline bool intersecting(const Plane& plane,   const Cone& cone)                           { return intersecting(cone, plane); }
	inline bool intersecting(const Plane& plane,   const Cuboid& cuboid)                       { return intersecting(cuboid, plane); }
	inline bool intersecting(const Plane& plane,   const Cylinder& cylinder)                   { return intersecting(cylinder, plane); }
	inline bool intersecting(const Plane& plane,   const Line& line)                           { return intersecting(line, plane); }
	inline bool intersecting(const Plane& plane,   const LineSegment& lineSegment)             { return intersecting(lineSegment, plane); }
	       bool intersecting(const Plane& plane_1, const Plane& plane_2);                      // IMPLEMENTED
	inline bool intersecting(const Plane& plane,   const Quad& quad)                           { return get_intersection(plane, quad).has_value(); }     // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Plane& plane,   const Ray& ray)                             { return get_intersection(plane, ray).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Plane& plane,   const Sphere& sphere)                       { return get_intersection(plane, sphere).has_value(); }   // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Plane& plane,   const Triangle& triangle)                   { return get_intersection(plane, triangle).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Plane& plane_1, const Plane& plane_2, const Plane& plane_3) { return get_intersection(plane_1, plane_2).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Quad functions
	//==============================================================================================================================
	inline bool intersecting(const Quad& quad,   const AABB& AABB)               { return intersecting(AABB, quad); }
	inline bool intersecting(const Quad& quad,   const Cone& cone)               { return intersecting(cone, quad); }
	inline bool intersecting(const Quad& quad,   const Cuboid& cuboid)           { return intersecting(cuboid, quad); }
	inline bool intersecting(const Quad& quad,   const Cylinder& cylinder)       { return intersecting(cylinder, quad); }
	inline bool intersecting(const Quad& quad,   const Line& line)               { return intersecting(line, quad); }
	inline bool intersecting(const Quad& quad,   const LineSegment& lineSegment) { return intersecting(lineSegment, quad); }
	inline bool intersecting(const Quad& quad,   const Plane& plane)             { return intersecting(plane, quad); }
	inline bool intersecting(const Quad& quad_1, const Quad& quad_2)             { return get_intersection(quad_1, quad_2).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Quad& quad,   const Ray& ray)                 { return get_intersection(quad, ray).has_value(); }      // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Quad& quad,   const Sphere& sphere)           { return get_intersection(quad, sphere).has_value(); }   // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Quad& quad,   const Triangle& triangle)       { return get_intersection(quad, triangle).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Ray functions
	//==============================================================================================================================
	inline bool intersecting(const Ray& ray,   const AABB& AABB)               { return intersecting(AABB, ray); }
	inline bool intersecting(const Ray& ray,   const Cone& cone)               { return intersecting(cone, ray); }
	inline bool intersecting(const Ray& ray,   const Cuboid& cuboid)           { return intersecting(cuboid, ray); }
	inline bool intersecting(const Ray& ray,   const Cylinder& cylinder)       { return intersecting(cylinder, ray); }
	inline bool intersecting(const Ray& ray,   const Line& line)               { return intersecting(line, ray); }
	inline bool intersecting(const Ray& ray,   const LineSegment& lineSegment) { return intersecting(lineSegment, ray); }
	inline bool intersecting(const Ray& ray,   const Plane& plane)             { return intersecting(plane, ray); }
	inline bool intersecting(const Ray& ray,   const Quad& quad)               { return intersecting(quad, ray); }
	inline bool intersecting(const Ray& ray_1, const Ray& ray_2)               { return get_intersection(ray_1, ray_2).has_value(); }  // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Ray& ray,   const Sphere& sphere)           { return get_intersection(ray, sphere).has_value(); }   // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Ray& ray,   const Triangle& triangle)       { return get_intersection(ray, triangle).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Sphere functions
	//==============================================================================================================================
	inline bool intersecting(const Sphere& sphere,   const AABB& AABB)               { return intersecting(AABB, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Cone& cone)               { return intersecting(cone, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Cuboid& cuboid)           { return intersecting(cuboid, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Cylinder& cylinder)       { return intersecting(cylinder, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Line& line)               { return intersecting(line, sphere); }
	inline bool intersecting(const Sphere& sphere,   const LineSegment& lineSegment) { return intersecting(lineSegment, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Plane& plane)             { return intersecting(plane, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Quad& quad)               { return intersecting(quad, sphere); }
	inline bool intersecting(const Sphere& sphere,   const Ray& ray)                 { return intersecting(ray, sphere); }
	inline bool intersecting(const Sphere& sphere_1, const Sphere& sphere_2)         { return get_intersection(sphere_1, sphere_2).has_value(); } // Expensive get_intersection call for lack of bespoke intersection function #TODO
	inline bool intersecting(const Sphere& sphere,   const Triangle& triangle)       { return get_intersection(sphere, triangle).has_value(); }   // Expensive get_intersection call for lack of bespoke intersection function #TODO

	// Triangle functions
	//==============================================================================================================================
	inline bool intersecting(const Triangle& triangle,   const AABB& AABB)                                        { return intersecting(AABB, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Cone& cone)                                        { return intersecting(cone, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Cuboid& cuboid)                                    { return intersecting(cuboid, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Cylinder& cylinder)                                { return intersecting(cylinder, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Line& line)                                        { return intersecting(line, triangle); }
	inline bool intersecting(const Triangle& triangle,   const LineSegment& lineSegment)                          { return intersecting(lineSegment, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Plane& plane)                                      { return intersecting(plane, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Quad& quad)                                        { return intersecting(quad, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Ray& ray)                                          { return intersecting(ray, triangle); }
	inline bool intersecting(const Triangle& triangle,   const Sphere& sphere)                                    { return intersecting(sphere, triangle); }
	       bool intersecting(const Triangle& triangle_1, const Triangle& triangle_2, bool test_co_planar = true); // IMPLEMENTED
//==============================================================================================================================
// end intersecting functions
//==============================================================================================================================

} // namespace Geometry