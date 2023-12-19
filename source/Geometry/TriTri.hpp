// An implementation of 'Faster Triangle-Triangle Intersection Tests' by Olivier Devillers and Philippe Guigue.
// Reference: Olivier Devillers, Philippe Guigue. Faster Triangle-Triangle Intersection Tests. RR-4488, INRIA. 2002. ffinria-00072100f
// Adapted:   Eric Haines: https://github.com/erich666/jgt-code/blob/master/Volume_08/Number_1/Guigue2003/tri_tri_intersect.c
// This implementation matches the Haines work but is adapted to glm and C++.
#pragma once

#include "glm/fwd.hpp"

namespace Geometry
{
	static constexpr bool USE_EPSILON_TEST_TRI_TRI = true; // Set to true to use coplanarity robustness checks

	// Three-dimensional Triangle-Triangle intersection test.
	//@param p1,q1,r1 The vertices of triangle 1.
	//@param p2,q2,r2 The vertices of triangle 2.
	//@return Whether the triangles overlap.
	bool tri_tri_is_intersecting(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                             const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2);

	// Three-dimensional Triangle-Triangle get-intersection test.
	// Computes the segment of intersection of the two triangles if it exists.
	//@param p1,q1,r1 The vertices of triangle 1.
	//@param p2,q2,r2 The vertices of triangle 2.
	//@param coplanar Whether the triangles are coplanar.
	//@param l_source,l_target The start and end point of the line segment of intersection. Only valid if the triangles intersect and are not coplanar.
	//@return Whether the triangles intersect or are coplanar.
	bool tri_tri_get_intersection(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                              const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                              bool& coplanar, glm::vec3& l_source, glm::vec3& l_target);
} // namespace Geometry