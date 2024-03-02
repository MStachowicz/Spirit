#include "GJK.hpp"
#include "Intersect.hpp"
#include "Triangle.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <algorithm>
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

	// Tests if the reverse of an edge already exists in the list and if so, removes it.
	void add_if_unique_edge(std::vector<std::pair<unsigned int, unsigned int>>& edges, const std::vector<unsigned int>& faces, unsigned int a, unsigned int b)
	{
		// By virtue of winding order, if a neighbouring face shares an edge, it will be in reverse order.
		// We only want to store the edges that we are going to save because every edge gets removed first, then we repair.

		auto reverse = std::find(edges.begin(), edges.end(), std::make_pair(faces[b], faces[a]));

		if (reverse != edges.end())
			edges.erase(reverse);
		else
			edges.emplace_back(faces[a], faces[b]);
	}

	// Returns a list of face normals and the index of the closest face.
	//@param polytope: The list of points that define the polytope.
	//@param faces: The list of indices that define the faces of the polytope.
	//@returns A pair containing the list of face normals (xyz pos + w distance) and the index of the closest face.
	std::pair<std::vector<glm::vec4>, size_t> get_face_normals(const std::vector<glm::vec3>& polytope, const std::vector<unsigned int>& faces)
	{
		std::vector<glm::vec4> face_normals;
		face_normals.reserve(faces.size() / 3);
		unsigned int min_triangle   = 0;
		float min_distance = std::numeric_limits<float>::max();

		for (unsigned int i = 0; i < static_cast<unsigned int>(faces.size()); i += 3)
		{
			glm::vec3 a = polytope[faces[i]];
			glm::vec3 b = polytope[faces[i + 1]];
			glm::vec3 c = polytope[faces[i + 2]];

			glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
			float distance   = glm::dot(normal, a);

			if (distance < 0)
			{
				normal   *= -1;
				distance *= -1;
			}
			face_normals.emplace_back(normal, distance);

			if (distance < min_distance)
			{
				min_triangle = i / 3u;
				min_distance = distance;
			}
		}

		return {face_normals, min_triangle};
	}

	CollisionPoint EPA(const Simplex& p_simplex,
	                   const std::vector<glm::vec3>& p_points_1, const glm::mat4& p_transform_1, const glm::quat& p_orientation_1,
	                   const std::vector<glm::vec3>& p_points_2, const glm::mat4& p_transform_2, const glm::quat& p_orientation_2)
	{
		if (p_simplex.size != 4)
			throw std::runtime_error("[GJK] Invalid simplex size in EPA function. EPA expects incoming simplex to be a tetrahedron.");

		std::vector<glm::vec3> polytope = {p_simplex[0], p_simplex[1], p_simplex[2], p_simplex[3]};
		std::vector<unsigned int> faces = {
		    0, 1, 2,
		    0, 3, 1,
		    0, 2, 3,
		    1, 3, 2};
		auto [face_normals, min_face] = get_face_normals(polytope, faces);

		glm::vec3 min_normal = face_normals[min_face];
		float min_distance   = std::numeric_limits<float>::max();

		while (min_distance == std::numeric_limits<float>::max())
		{
			min_normal   = face_normals[min_face];
			min_distance = face_normals[min_face].w;

			glm::vec3 support = support_point(min_normal,
			                                  p_points_1, p_transform_1, p_orientation_1,
			                                  p_points_2, p_transform_2, p_orientation_2);
			float s_distance  = dot(min_normal, support);

			if (std::abs(s_distance - min_distance) > 0.001f)
			{
				min_distance = std::numeric_limits<float>::max();

				// When expanding the polytope, we cannot just add a vertex, we need to repair the faces as well.
				// When two faces result in the same support point being added, duplicate faces end up inside the polytope and cause incorrect results.
				// We have to remove every face that is pointing in the direction of the support point relative to the triangle.
				// To repair afterwards, we keep track of unique_edges and use those along with the support point's index to make new faces.
				std::vector<std::pair<unsigned int, unsigned int>> unique_edges;
				for (unsigned int i = 0; i < face_normals.size(); i++)
				{
					// This line compares the shortest distance of two parallel planes and the origin,
					// passing by a forced point (support vertex vs point of current face).
					// If the distance of the support point is lower, then the result would be a concave polytope, which is undesired.
					// if ( normals[i].dot(support) > normals[i].dot(polytope[faces[i*3]]))
					// Check same_direction relative to the triangle, not just the face normal.

					if (same_direction(face_normals[i], support - polytope[faces[i * 3]]))
					{
						unsigned int face_index = i * 3;

						add_if_unique_edge(unique_edges, faces, face_index, face_index + 1);
						add_if_unique_edge(unique_edges, faces, face_index + 1, face_index + 2);
						add_if_unique_edge(unique_edges, faces, face_index + 2, face_index);

						faces[face_index + 2] = faces.back();
						faces.pop_back();
						faces[face_index + 1] = faces.back();
						faces.pop_back();
						faces[face_index] = faces.back();
						faces.pop_back();

						face_normals[i] = face_normals.back(); // pop-erase
						face_normals.pop_back();

						i--;
					}
				}
				if (unique_edges.size() == 0)
					break;

				// Now that we have a list of unique_edges, we can add the new_faces to a list and add the supporting point to the polytope.
				// Storing the new_faces in their own list allows us to calculate only the normals of these new_faces.
				std::vector<unsigned int> new_faces;
				for (auto [edge_index_1, edge_index_2] : unique_edges)
				{
					new_faces.push_back(edge_index_1);
					new_faces.push_back(edge_index_2);
					new_faces.push_back(static_cast<unsigned int>(polytope.size()));
				}
				polytope.push_back(support);
				auto [new_normals, new_min_face] = get_face_normals(polytope, new_faces);

				// After calculating the new normals, we need to find the new closest face.
				// We only iterate over the old normals, and compare the closest one to the closest face of the new normals.
				// Then we can add these new faces and normals to the end of faces and face_normals respectively.
				float new_min_distance = std::numeric_limits<float>::max();
				for (size_t i = 0; i < face_normals.size(); i++)
				{
					if (face_normals[i].w < new_min_distance)
					{
						new_min_distance = face_normals[i].w;
						min_face         = i;
					}
				}
				if (new_normals[new_min_face].w < new_min_distance)
				{
					min_face = new_min_face + face_normals.size();
				}

				faces.insert(faces.end(), new_faces.begin(), new_faces.end());
				face_normals.insert(face_normals.end(), new_normals.begin(), new_normals.end());
			}
		}

		if (min_distance == std::numeric_limits<float>::max())
			throw std::runtime_error("[GJK] EPA failed to find a collision point. This should never happen.");


		// The closest face of the polytope to the origin of the Minkowski difference is the face that represents the deepest penetration.
		// The normal of this face is the collision normal, and its distance to the origin is the penetration depth.
		// The collision point is the point on the face closest to the origin.

		auto min_face_a = polytope[faces[min_face * 3]]; // * 3 because faces is a list of indices, not points.
		auto min_face_b = polytope[faces[min_face * 3 + 1]];
		auto min_face_c = polytope[faces[min_face * 3 + 2]];
		auto closest_point = Geometry::closest_point(Geometry::Triangle(min_face_a, min_face_b, min_face_c), glm::vec3(0.f));
		// Once a supporting point isn't found further from the closest face,
		// We return that face's normal and its distance in a CollisionPoint.
		CollisionPoint point;
		point.normal            = min_normal;
		point.A                 = glm::inverse(p_transform_1) * glm::vec4(closest_point + point.normal * min_distance, 1.f);
		point.B                 = glm::inverse(p_transform_2) * glm::vec4(closest_point - point.normal * min_distance, 1.f);
		point.penetration_depth = min_distance + 0.001f;
		return point;
	}
} // namespace GJK