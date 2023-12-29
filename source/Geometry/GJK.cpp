#include "GJK.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <stdexcept>

namespace GJK
{
	bool same_direction(const glm::vec3& A, const glm::vec3& B)
	{
		return glm::dot(A, B) > 0.f;
	}

	glm::vec3 support_point(const glm::vec3& p_direction, const std::vector<glm::vec3>& p_points)
	{
		if (p_points.size() == 0)
			throw std::runtime_error("[GJK] Empty point set in support_point func.");

		glm::vec3 furthest_point = p_points[0];
		float furthest_distance  = glm::dot(p_direction, furthest_point);

		for (const glm::vec3& point : p_points)
		{
			float distance = glm::dot(p_direction, point);

			if (distance > furthest_distance)
			{
				furthest_distance = distance;
				furthest_point    = point;
			}
		}

		return furthest_point;
	}

	glm::vec3 support_point(const glm::vec3& p_direction,
	                        const std::vector<glm::vec3>& p_points_1, const glm::mat4& p_transform_1, const glm::quat& p_orientation_1,
	                        const std::vector<glm::vec3>& p_points_2, const glm::mat4& p_transform_2, const glm::quat& p_orientation_2)
	{
		// We transform p_direction into each shape's object-space orientation to get the support points in object space.
		// We then transform the support points into world space and return the difference.
		// This allows us to transform just the two points rather than the entire point set.

		const auto shape_1_object_space_dir =   glm::inverse(p_orientation_1) * p_direction;
		const auto shape_2_object_space_dir = -(glm::inverse(p_orientation_2) * p_direction);

		const auto mesh_1_support_point_object_space = support_point(shape_1_object_space_dir, p_points_1);
		const auto mesh_1_support_point_world_space  = p_transform_1 * glm::vec4(mesh_1_support_point_object_space, 1.f);
		const auto mesh_2_support_point_object_space = support_point(shape_2_object_space_dir, p_points_2);
		const auto mesh_2_support_point_world_space  = p_transform_2 * glm::vec4(mesh_2_support_point_object_space, 1.f);

		return mesh_1_support_point_world_space - mesh_2_support_point_world_space;
	}

	bool do_simplex(Simplex& p_simplex, glm::vec3& p_direction)
	{
		// Purpose of the do_simplex function is to iteratively construct a simplex that encloses the origin.
		// In each case, the closest feature to the origin is determined and the simplex is re-formed.
		// The points get added, swapped and removed from the simplex as needed to converge.

		auto do_line = [&p_simplex, &p_direction]()
		{
			// Line case has 3 total regions defined by the features:
			// 1 edge, 2 vertices.

			// We cull out the regions that are impossible by virtue of having just added point A.
			// This impossible region is beyond vertex B because the origin is in the direction of A.
			// This leaves us with 2 regions.

            //           |             | X X X X X
            //           |             | X X X X X
            //     2     |      1      | X X X X X
            //           |             | X X X X X
            // - - - - - A - - - - - - B X X X X X
			//           |             | X X X X X
			//     2     |      1      | X X X X X
			//           |             | X X X X X
			//           |             | X X X X X

			// Using the diagram above, we can determine which region we are in by comparing the direction of the line AO with the features.
			// For visualisation, it helps to consider each numbered region above as the origin O and the line AO as the line from A to the numbers.
			// Note the algorithm below is checking the line in 3D, so region 1 is around the LineSegment AB and region 2 is the area beyond vertex A in direction BA.

			const glm::vec3& A = p_simplex[0];
			const glm::vec3& B = p_simplex[1];

			glm::vec3 AB = B - A;
			glm::vec3 AO = -A;

			if (same_direction(AB, AO)) // If origin is in the direction of AB we are in region 1.
			{
				p_direction = glm::cross(glm::cross(AB, AO), AB); // Return the direction perpendicular to AB in the direction of the origin.
			}
			else // The origin is in region 2, beyond vertex A.
			{// We revert to a point case A, returning the direction as A to origin.
				p_simplex   = {A};
				p_direction = AO;
			}
		};
		auto do_triangle = [&]()
		{
			// Triangle case has 8 total regions defined by the features:
			// 2 faces (front + back), 3 edges, 3 vertices.

			// We cull out the regions that are impossible by virtue of having just added point A.
			// This impossible region is along the line BC because the origin is in the direction of A.
			// This leaves us with 4 regions.

            //             \X X X/ X X X X X X
            //              \X X/ X X X X X X
            //               \X/ X X X X X X X
            //                C X X X X X X X
            //               / \ X X X X X X X
            //              /   \ X X X X X X
            //    1        /  4  \ X X X X X X
            //            /       \ X X X X X
            //_ _ _ _ _ _A_ _ _ _ _B_X_X_X_X_X
            //          /           \ X X X X
            //    2    /      3      \ X X X X
            //        /               \ X X X
            //       /                 \ X X X

			// Using the diagram above, we can determine which region we are in by comparing the direction of the line AO with the features.
			// For visualisation, it helps to consider each numbered region above as the origin O and the line AO as the line from A to the numbers.
			// Note the algorithm below is checking the triangle in 3D, so the lines can be seen as planes and region 4 as the area behind or in front of the triangle.

			const glm::vec3& A = p_simplex[0];
			const glm::vec3& B = p_simplex[1];
			const glm::vec3& C = p_simplex[2];

			glm::vec3 AB = B - A; // Edge AB
			glm::vec3 AC = C - A; // Edge AC
			glm::vec3 AO = -A;    // Direction to origin from A

			glm::vec3 ABC = glm::cross(AB, AC); // Normal of the triangle

			if (same_direction(glm::cross(ABC, AC), AO)) // If the line perpendicular to AC is in the same direction as origin, we can be in region 1 or 2.
			{
				if (same_direction(AC, AO)) // If AC is in the same direction as AO we are in region 1.
				{
					p_simplex   = {A, C};
					p_direction = glm::cross(glm::cross(AC, AO), AC);
				}
				else // Otherwise we know we are in region 2 or 3.
				{    // We revert to a line case AB knowing the point C is not in the direction of the origin.
					p_simplex = {A, B};
					do_line();
					return;
				}
			}
			else // If the origin was not in the direction of the perpendicular to AC, we can be in regions 3 or 4.
			{
				if (same_direction(glm::cross(AB, ABC), AO)) // If the perpendicular to AB is in the same direction as AO we are in region 2 or 3.
				{// As above, we revert to line case AB knowing the point C is not in the direction of the origin.
					p_simplex = {A, B};
					do_line();
					return;
				}
				else // By process of elimination, if we are not in region 1, 2 or 3, we must be in region 4.
				{    // This case forms a tetrahedron on the next iteration.
					if (same_direction(ABC, AO)) // If AO is in the same direction as ABC, we are in front of the triangle.
					{
						p_direction = ABC;
					}
					else // Otherwise we are behind the triangle.
					{
						p_simplex   = {A, C, B};
						p_direction = -ABC;
					}
				}
			}
		};
		auto do_tetrahedron = [&]()
		{
			// Tetrahedron is defined by the features:
			// 4 faces, 6 edges, 4 vertices.

			// We cull out the regions that are impossible by virtue of having just added point A.
			// This is any area below the triangle BCD because the origin is in the direction of A.
			// This leaves us with 3 regions which are all faces/triangles.

			const glm::vec3& A = p_simplex[0];
			const glm::vec3& B = p_simplex[1];
			const glm::vec3& C = p_simplex[2];
			const glm::vec3& D = p_simplex[3];

			glm::vec3 AB = B - A;
			glm::vec3 AC = C - A;
			glm::vec3 AD = D - A;
			glm::vec3 AO = -A;

			glm::vec3 ABC = glm::cross(AB, AC);
			glm::vec3 ACD = glm::cross(AC, AD);
			glm::vec3 ADB = glm::cross(AD, AB);

			if (same_direction(ABC, AO)) // If the origin is in the direction of triangle ABC normal, check the triangle case.
			{
				p_simplex = {A, B, C};
				do_triangle();
				return false;
			}
			else if (same_direction(ACD, AO)) // If the origin is in the direction of triangle ACD normal, check the triangle case.
			{
				p_simplex = {A, C, D};
				do_triangle();
				return false;
			}
			else if (same_direction(ADB, AO)) // If the origin is in the direction of triangle ADB normal, check the triangle case.
			{
				p_simplex = {A, D, B};
				do_triangle();
				return false;
			}
			else // By process of elimination, if none of the faces of the tetrahedron are in the direction of the origin, we must be inside the tetrahedron.
				return true;
		};

		switch (p_simplex.size)
		{
			case 2: do_line();     return false;
			case 3: do_triangle(); return false;
			case 4:  return do_tetrahedron();
			default: throw std::runtime_error("[GJK] Invalid simplex size in do_simplex func.");
		}
	}

	bool intersecting(const std::vector<glm::vec3>& p_points_1, const glm::mat4& p_transform_1, const glm::quat& p_orientation_1,
	                  const std::vector<glm::vec3>& p_points_2, const glm::mat4& p_transform_2, const glm::quat& p_orientation_2,
	                  const glm::vec3& p_initial_direction)
	{
		glm::vec3 direction = p_initial_direction;
		Simplex simplex = {support_point(direction,
		                                 p_points_1, p_transform_1, p_orientation_1,
		                                 p_points_2, p_transform_2, p_orientation_2)};
		direction = -simplex[0]; // AO, search in the direction of the origin. Reversed direction to point towards the origin.

		while (true) // Main GJK loop. Converge on A simplex that encloses the origin.
		{
			auto new_support_point = support_point(direction,
			                                       p_points_1, p_transform_1, p_orientation_1,
			                                       p_points_2, p_transform_2, p_orientation_2);

			// If the new support point is not past the origin then its impossible to enclose the origin.
			if (glm::dot(new_support_point, direction) <= 0.f)
				return false;

			// Shift the simplex points along to retain A as the most recently added support point as do_simplex expects.
			simplex.push_front(new_support_point);

			if (do_simplex(simplex, direction))
				return true;
		}
	}
} // namespace GJK