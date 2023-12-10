#pragma once

#include "Utility.hpp"

#include "Component/Mesh.hpp"
#include "Geometry/Shape.hpp"
#include "Geometry/LineSegment.hpp"
#include "OpenGL/GLState.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/quaternion.hpp"

#include <array>
#include <vector>
#include <numbers>
#include <utility>
#include <type_traits>

namespace Utility
{
	template <typename VertexType = Data::Vertex, OpenGL::PrimitiveMode primitive_mode = OpenGL::PrimitiveMode::Triangles, bool build_collision_shape = false>
	requires Data::is_valid_mesh_vert<VertexType>
	class MeshBuilder
	{
		std::vector<VertexType> data;
		glm::vec4 current_colour;
		std::vector<Geometry::Shape> shapes;

	public:
		MeshBuilder() noexcept
			: data{}
			, current_colour{glm::vec4{1.f}}
			, shapes{}
		{}
		void reserve(size_t size)
		{
			data.reserve(size);
		}
		void clear()
		{
			data.clear();
			shapes.clear();
		}
		bool empty() const
		{
			return data.empty();
		}
		void set_colour(const glm::vec4& colour)
		{
			static_assert(Data::has_colour_member<VertexType>, "VertexType must have a colour member.");
			current_colour = colour;
		}
		void set_colour(const glm::vec3& colour)
		{
			static_assert(Data::has_colour_member<VertexType>, "VertexType must have a colour member.");
			current_colour = glm::vec4(colour, 1.f);
		}
		[[nodiscard]] Data::Mesh get_mesh()
		{
			return Data::Mesh{data, primitive_mode, shapes};
		}

	private:
		// Helpers for MeshBuilder::add_ functions. These perform the actual adding of vertices to the data vector.
		// Impl versions do not add to the shapes vector. Only the publicly accessible versions do.
		// This allows add_ functions to use the impl versions without adding extra shapes to the shapes vetor.

		// Add a vertex to the mesh. _impl version (doesn't add to shapes vector).
		template <typename Vertex>
		void add_vertex_impl(Vertex&& v)
		{
			static_assert(primitive_mode == OpenGL::PrimitiveMode::Points, "add_vertex requires MeshBuilder PrimitiveMode to be Points.");
			static_assert(std::is_same_v<std::decay_t<Vertex>, VertexType>, "Vertex type must match the MeshBuilder VertexType.");
			static_assert(!Data::has_normal_member<VertexType>, "add_vertex doesnt support normal data. Remove normal from Vertex.");
			static_assert(!Data::has_UV_member<VertexType>, "add_vertex doesnt support UV data. Remove UV from VertexType.");

			if constexpr (Data::has_colour_member<VertexType>)
				v.colour = current_colour;

			data.emplace_back(std::forward<Vertex>(v));
		}
		// Add a line to the mesh. _impl version (doesn't add to shapes vector).
		template <typename Vertex, typename Vertex2>
		void add_line_impl(Vertex&& v1, Vertex2&& v2)
		{
			static_assert(primitive_mode == OpenGL::PrimitiveMode::Lines, "add_line requires MeshBuilder PrimitiveMode to be Lines.");
			static_assert(!Data::has_normal_member<VertexType>, "add_line doesnt support normal data. Remove the normal from VertexType.");
			static_assert(!Data::has_UV_member<VertexType>, "add_line doesnt support UV data. Remove UV from VertexType.");

			if constexpr (std::is_same_v<std::decay_t<Vertex>, glm::vec3> && std::is_same_v<std::decay_t<Vertex2>, glm::vec3>)
			{// vec3 overload, apply the position to the vertices and call recursively with them.
				VertexType v1_t;
				VertexType v2_t;
				v1_t.position = v1;
				v2_t.position = v2;
				add_line_impl(std::forward<VertexType>(v1_t), std::forward<VertexType>(v2_t));
			}
			else if constexpr (std::is_same_v<std::decay_t<Vertex>, VertexType> && std::is_same_v<std::decay_t<Vertex2>, VertexType>)
			{
				if constexpr (Data::has_colour_member<VertexType>)
				{
					v1.colour = current_colour;
					v2.colour = current_colour;
				}
				data.emplace_back(std::forward<Vertex>(v1));
				data.emplace_back(std::forward<Vertex2>(v2));
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_line for this combo of VertexType params."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		// Add triangle to the mesh. _impl version (doesn't add to shapes vector).
		template <typename Vertex, typename Vertex2, typename Vertex3>
		void add_triangle_impl(Vertex&& v1, Vertex2&& v2, Vertex3&& v3)
		{
			static_assert(primitive_mode == OpenGL::PrimitiveMode::Triangles, "add_triangle requires MeshBuilder PrimitiveMode to be Triangles.");

			if constexpr (std::is_same_v<std::decay_t<Vertex>, glm::vec3> && std::is_same_v<std::decay_t<Vertex2>, glm::vec3> && std::is_same_v<std::decay_t<Vertex3>, glm::vec3>)
			{// vec3 overload, apply the position to the VertexType and call add_triangle recursively with them.
				VertexType v1_t;
				VertexType v2_t;
				VertexType v3_t;
				v1_t.position = v1;
				v2_t.position = v2;
				v3_t.position = v3;
				add_triangle_impl(std::forward<VertexType>(v1_t), std::forward<VertexType>(v2_t), std::forward<VertexType>(v3_t));
			}
			else if constexpr (std::is_same_v<std::decay_t<Vertex>, VertexType> && std::is_same_v<std::decay_t<Vertex2>, VertexType> && std::is_same_v<std::decay_t<Vertex3>, VertexType>)
			{
				if constexpr (Data::has_normal_member<VertexType>)
				{
					const auto edge1       = v2.position - v1.position;
					const auto edge2       = v3.position - v1.position;
					const auto calc_normal = glm::cross(edge1, edge2);
					add_triangle_impl(std::forward<Vertex>(v1), std::forward<Vertex2>(v2), std::forward<Vertex3>(v3), calc_normal);
				}
				else
				{
					if constexpr (Data::has_colour_member<VertexType>)
					{
						v1.colour = current_colour;
						v2.colour = current_colour;
						v3.colour = current_colour;
					}

					data.emplace_back(std::forward<Vertex>(v1));
					data.emplace_back(std::forward<Vertex2>(v2));
					data.emplace_back(std::forward<Vertex3>(v3));
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_triangle for this combo of VertexType params."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		// Add triangle to the mesh with a pre-defined normal. _impl version (doesn't add to shapes vector).
		template <typename Vertex, typename Vertex2, typename Vertex3>
		void add_triangle_impl(Vertex&& v1, Vertex2&& v2, Vertex3&& v3, const glm::vec3& normal)
		{
			static_assert(primitive_mode == OpenGL::PrimitiveMode::Triangles, "add_triangle requires MeshBuilder PrimitiveMode to be Triangles.");
			static_assert(Data::has_normal_member<VertexType>, "VertexType must have a normal member. Call non-normal overload of add_triangle or remove normal data from VertexType.");

			if constexpr (std::is_same_v<std::decay_t<Vertex>, VertexType> && std::is_same_v<std::decay_t<Vertex2>, VertexType> && std::is_same_v<std::decay_t<Vertex3>, VertexType>)
			{
				if constexpr (Data::has_colour_member<VertexType>)
				{
					v1.colour = current_colour;
					v2.colour = current_colour;
					v3.colour = current_colour;
				}

				v1.normal = normal;
				v2.normal = normal;
				v3.normal = normal;

				data.emplace_back(std::forward<Vertex>(v1));
				data.emplace_back(std::forward<Vertex2>(v2));
				data.emplace_back(std::forward<Vertex3>(v3));
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_triangle for this combo of VertexType params."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		// Add a quad to the mesh. _impl version (doesn't add to shapes vector).
		void add_quad_impl(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right)
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles || primitive_mode == OpenGL::PrimitiveMode::Lines)
			{
				VertexType top_left_v;
				VertexType bottom_left_v;
				VertexType bottom_right_v;
				VertexType top_right_v;

				top_left_v.position     = top_left;
				bottom_left_v.position  = bottom_left;
				bottom_right_v.position = bottom_right;
				top_right_v.position    = top_right;

				if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
				{
					if constexpr (Data::has_UV_member<VertexType>)
					{
						top_left_v.uv     = glm::vec2(0.f, 1.f);
						bottom_left_v.uv  = glm::vec2(0.f, 0.f);
						bottom_right_v.uv = glm::vec2(1.f, 0.f);
						top_right_v.uv    = glm::vec2(1.f, 1.f);
					}

					if constexpr (Data::has_normal_member<VertexType>)
					{
						const auto normal = glm::normalize(glm::cross(bottom_left - top_left, top_right - top_left));
						add_triangle_impl(top_left_v, bottom_left_v, bottom_right_v, normal);
						add_triangle_impl(top_left_v, bottom_right_v, top_right_v, normal);
					}
					else
					{
						add_triangle_impl(top_left_v, bottom_left_v, bottom_right_v);
						add_triangle_impl(top_left_v, bottom_right_v, top_right_v);
					}
				}
				else if constexpr (primitive_mode == OpenGL::PrimitiveMode::Lines)
				{
					add_line_impl(top_left_v, bottom_left_v);
					add_line_impl(bottom_left_v, bottom_right_v);
					add_line_impl(bottom_right_v, top_right_v);
					add_line_impl(top_right_v, top_left_v);
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_quad for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		// Add a circle to the mesh. _impl version (doesn't add to shapes vector).
		void add_circle_impl(const glm::vec3& center, float radius, size_t segments, const glm::vec3& normal = {0.f, 1.f, 0.f})
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto points_and_UVs = get_circle_points(center, radius, segments, normal);

				for (size_t i = 0; i < segments; ++i)
				{
					VertexType v1;
					VertexType v2;
					VertexType v3;

					v1.position = points_and_UVs[(i + 1) % segments].first;
					v2.position = center;
					v3.position = points_and_UVs[i].first;

					if constexpr (Data::has_UV_member<VertexType>)
					{
						v1.uv = glm::vec2(0.5f) - (points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f));
						v2.uv = glm::vec2{0.5f, 0.5f};
						v3.uv = glm::vec2(0.5f) - (points_and_UVs[i].second * glm::vec2(0.5f, -0.5f));
					}

					if constexpr (Data::has_normal_member<VertexType>)
						add_triangle_impl(v1, v2, v3, normal);
					else
						add_triangle_impl(v1, v2, v3);
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_circle for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}

	public:
		template <typename Vertex>
		void add_vertex(Vertex&& v)
		{
			static_assert(build_collision_shape == false, "Vertex mesh doesnt support collisions. Use Geometry::point_inside functions instead.");
			add_vertex_impl(std::forward<Vertex>(v));
		}
		template <typename Vertex, typename Vertex2>
		void add_line(Vertex&& v1, Vertex2&& v2)
		{
			add_line_impl(std::forward<Vertex>(v1), std::forward<Vertex2>(v2));
			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::LineSegment{v1.position, v2.position});
		}
		void add_line(const Geometry::LineSegment& line)
		{
			add_line(line.m_start, line.m_end);
		}
		template <typename Vertex, typename Vertex2, typename Vertex3>
		void add_triangle(Vertex&& v1, Vertex2&& v2, Vertex3&& v3)
		{
			add_triangle_impl(std::forward<Vertex>(v1), std::forward<Vertex2>(v2), std::forward<Vertex3>(v3));
			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::Triangle{v1.position, v2.position, v3.position});
		}
		template <typename Vertex, typename Vertex2, typename Vertex3>
		void add_triangle(Vertex&& v1, Vertex2&& v2, Vertex3&& v3, const glm::vec3& normal)
		{
			add_triangle_impl(std::forward<Vertex>(v1), std::forward<Vertex2>(v2), std::forward<Vertex3>(v3), normal);
			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::Triangle{v1.position, v2.position, v3.position});
		}
		void add_triangle(const Geometry::Triangle& triangle)
		{
			add_triangle(triangle.m_point_1, triangle.m_point_2, triangle.m_point_3);
		}
		void add_quad(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right)
		{
			add_quad_impl(top_left, top_right, bottom_left, bottom_right);

			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::Quad{top_left, top_right, bottom_left, bottom_right});
		}
		void add_quad(const Geometry::Quad& quad)
		{
			add_quad(quad.m_point_1, quad.m_point_2, quad.m_point_3, quad.m_point_4);
		}
		void add_cone(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments = 16)
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto top_to_base    = glm::normalize(base - top);
				const auto points_and_UVs = get_circle_points(base, radius, segments, top_to_base);

				for (size_t i = 0; i < segments; ++i)
				{
					VertexType v1;
					VertexType v2;
					VertexType v3;

					v1.position = points_and_UVs[i].first;
					v2.position = top;
					v3.position = points_and_UVs[(i + 1) % segments].first;

					if constexpr (Data::has_UV_member<VertexType>)
					{
						v1.uv = glm::vec2(0.5f) - (points_and_UVs[i].second * glm::vec2(0.5f, -0.5f));
						v2.uv = glm::vec2{0.5f, 0.5f};
						v3.uv = glm::vec2(0.5f) - (points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f));
					}

					add_triangle_impl(v1, v2, v3);
				}

				//#TODO #OPTIMIZATION Reuse the circle points from points_and_UVs for the base circle like cylinder.
				add_circle_impl(base, radius, segments, top_to_base);
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_cone for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)

			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::Cone{base, top, radius});
		}
		void add_cone(const Geometry::Cone& cone, size_t segments = 16)
		{
			add_cone(cone.m_base, cone.m_top, cone.m_base_radius, segments);
		}
		void add_cylinder(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments = 16)
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto base_to_top                = top - base;
				const auto base_to_top_dir            = glm::normalize(base_to_top);
				const auto top_to_base_dir            = -base_to_top_dir;
				const auto base_circle_points_and_UVs = get_circle_points(base, radius, segments, top_to_base_dir);

				VertexType base_vertex_center;
				VertexType top_vertex_center;
				base_vertex_center.position = base;
				top_vertex_center.position = top;

				if constexpr (Data::has_UV_member<VertexType>)
				{
					base_vertex_center.uv = glm::vec2{0.5f, 0.5f};
					top_vertex_center.uv  = glm::vec2{0.5f, 0.5f};
				}
				if constexpr (Data::has_normal_member<VertexType>)
				{
					base_vertex_center.normal = top_to_base_dir;
					top_vertex_center.normal  = base_to_top_dir;
				}

				for (size_t i = 0; i < segments; ++i)
				{
					VertexType base_vertex_1;
					VertexType base_vertex_2;
					VertexType top_vertex_1;
					VertexType top_vertex_2;

					base_vertex_1.position = base_circle_points_and_UVs[i].first;
					base_vertex_2.position = base_circle_points_and_UVs[(i + 1) % segments].first;
					top_vertex_1.position  = base_circle_points_and_UVs[i].first + base_to_top;
					top_vertex_2.position  = base_circle_points_and_UVs[(i + 1) % segments].first + base_to_top;

					if constexpr (Data::has_UV_member<VertexType>)
					{
						base_vertex_1.uv = glm::vec2(0.5f) - (base_circle_points_and_UVs[i].second * glm::vec2(0.5f, -0.5f));
						base_vertex_2.uv = glm::vec2{0.5f, 0.5f};
						top_vertex_1.uv  = glm::vec2{0.5f, 0.5f};
						top_vertex_2.uv  = glm::vec2{0.5f, 0.5f};
					}

					{// Add the triangles for the side of the cylinder. Go in triangle pairs forming quads along the side.
						add_triangle_impl(base_vertex_1, top_vertex_1, base_vertex_2);
						add_triangle_impl(base_vertex_2, top_vertex_1, top_vertex_2);
					}
					{// Add the triangles for the circles at the base and top of the cylinder.
					 // Though identical to MeshBuilder::add_circle, we dont want to call get_circle_points more than once so we reuse the base_circle_points.
					 	if constexpr (Data::has_normal_member<VertexType>)
						{
							add_triangle_impl(base_vertex_2, base_vertex_center, base_vertex_1, top_to_base_dir);
							add_triangle_impl(top_vertex_1, top_vertex_center, top_vertex_2, base_to_top_dir);
						}
						else
						{
							add_triangle_impl(base_vertex_2, base_vertex_center, base_vertex_1);
							add_triangle_impl(top_vertex_1, top_vertex_center, top_vertex_2);
						}
					}
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_cylinder for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)

			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::Cylinder{base, top, radius});
		}
		void add_cylinder(const Geometry::Cylinder& cylinder, size_t segments = 16)
		{
			add_cylinder(cylinder.m_base, cylinder.m_top, cylinder.m_radius, segments);
		}
		void add_arrow(const glm::vec3& base, const glm::vec3& top, size_t segments = 16)
		{
			const auto base_to_top        = top - base;
			const auto direction          = glm::normalize(base_to_top);
			const auto length             = glm::length(base_to_top);
			const auto base_radius        = length * 0.05f;
			const auto cone_radius        = length * 0.2f;
			const auto body_to_head_ratio = 0.75f;
			const auto body_top           = base + direction * length * body_to_head_ratio;

			add_cylinder(base, body_top, base_radius, segments);
			add_cone(body_top, top, cone_radius, segments);
		}
		void add_icosphere(const glm::vec3& center, float radius, size_t subdivisions)
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				constexpr auto standard_points = get_icosahedron_points();
				auto points = std::vector<glm::vec3>(standard_points.begin(), standard_points.end());

				for (size_t i = 0; i < subdivisions; i++)
				{
					std::vector<glm::vec3> new_points;
					new_points.reserve(points.size() * 4);

					// Going through each triangle, subdivide it into 4 triangles.
					for (size_t j = 0; j < points.size(); j += 3)
					{
						const auto a = (points[j]     + points[j + 1]) / 2.f;
						const auto b = (points[j + 1] + points[j + 2]) / 2.f;
						const auto c = (points[j + 2] + points[j])     / 2.f;

						// T1
						new_points.emplace_back(points[j]);
						new_points.emplace_back(a);
						new_points.emplace_back(c);
						// T2
						new_points.emplace_back(points[j + 1]);
						new_points.emplace_back(b);
						new_points.emplace_back(a);
						// T3
						new_points.emplace_back(points[j + 2]);
						new_points.emplace_back(c);
						new_points.emplace_back(b);
						// T4
						new_points.emplace_back(a);
						new_points.emplace_back(b);
						new_points.emplace_back(c);
					}
					points = std::move(new_points);
				}

				for (size_t i = 0; i < points.size(); i += 3)
				{
					points[i]     = ((points[i]     / glm::length(points[i]    )) * radius) + center;
					points[i + 1] = ((points[i + 1] / glm::length(points[i + 1])) * radius) + center;
					points[i + 2] = ((points[i + 2] / glm::length(points[i + 2])) * radius) + center;

					add_triangle_impl(points[i], points[i + 1], points[i + 2]);
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_icosphere for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)

			// A low subdivision icosphere only approximates a sphere. While we add a sphere here, below a certain subdivision level we could push just the faces.
			if constexpr (build_collision_shape)
				shapes.emplace_back(Geometry::Sphere{center, radius});
		}
		void add_sphere(const Geometry::Sphere sphere, size_t subdivisions)
		{
			add_icosphere(sphere.m_center, sphere.m_radius, subdivisions);
		}
		void add_cuboid(const Geometry::Cuboid& cuboid)
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles || primitive_mode == OpenGL::PrimitiveMode::Lines)
			{
				const auto vertices = cuboid.get_vertices();
				add_quad_impl(vertices[3], vertices[1], vertices[2], vertices[0]); // Top
				add_quad_impl(vertices[6], vertices[4], vertices[7], vertices[5]); // Bottom
				add_quad_impl(vertices[3], vertices[2], vertices[7], vertices[6]); // Left
				add_quad_impl(vertices[0], vertices[1], vertices[4], vertices[5]); // Right
				add_quad_impl(vertices[2], vertices[0], vertices[6], vertices[4]); // Front
				add_quad_impl(vertices[1], vertices[3], vertices[5], vertices[7]); // Back
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_cuboid for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)

			if constexpr (build_collision_shape)
				shapes.emplace_back(cuboid);
		}

	private: // Helpers for MeshBuilder::add_ functions

		// Get the points and UVs for a circle.
		//@param center Center of the circle.
		//@param radius Radius of the circle.
		//@param segments Number of segments to use to make the circle.
		//@param normal Direction the circle is facing. Decides the winding order.
		//@return A vector of pairs of points and UVs.
		[[nodiscard]] static std::vector<std::pair<glm::vec3, glm::vec2>> get_circle_points(const glm::vec3& center, float radius, size_t segments, const glm::vec3& normal = {0.f, 1.f, 0.f})
		{
			std::vector<std::pair<glm::vec3, glm::vec2>> points_and_UVs;
			points_and_UVs.reserve(segments);
			const auto rotation = Utility::get_rotation(glm::vec3{0.f, 1.f, 0.f}, normal); // Get the rotation quaternion to rotate the circle to the correct orientation.
			const float angle_step = 2.0f * std::numbers::pi_v<float> / segments;

			for (size_t i = 0; i < segments; ++i)
			{
				const auto angle = (float)i * angle_step;
				const auto point = center + (rotation * glm::vec3(radius * glm::sin(angle), 0.f, radius * glm::cos(angle)));
				const auto uv    = glm::vec2{glm::sin(angle), glm::cos(angle)};
				points_and_UVs.emplace_back(std::make_pair(point, uv));
			}

			return points_and_UVs;
		}
		// Get a pair of arrays defining a standard icosahedron. A platonic solid with 20 faces, 30 edges and 12 vertices.
		// Use get_icosahedron_points to return a flat list of points.
		//@return A pair of arrays, first=points, second=indices.
		[[nodiscard]] static consteval std::pair<std::array<glm::vec3, 12>, std::array<unsigned int, 60>> get_icosahedron_points_and_indices()
		{
			// create 12 vertices of a icosahedron
			constexpr auto t = std::numbers::phi_v<float>; // golden ratio
			return {
				{glm::vec3(-1.f, t, 0.f),
				 glm::vec3(1.f, t, 0.f),
				 glm::vec3(-1.f, -t, 0.f),
				 glm::vec3(1.f, -t, 0.f),

				 glm::vec3(0.f, -1.f, t),
				 glm::vec3(0.f, 1.f, t),
				 glm::vec3(0.f, -1.f, -t),
				 glm::vec3(0.f, 1.f, -t),

				 glm::vec3(t, 0.f, -1.f),
				 glm::vec3(t, 0.f, 1.f),
				 glm::vec3(-t, 0.f, -1.f),
				 glm::vec3(-t, 0.f, 1.f)},

				{0, 11, 5,
				 0, 5, 1,
				 0, 1, 7,
				 0, 7, 10,
				 0, 10, 11,

				 1, 5, 9,
				 5, 11, 4,
				 11, 10, 2,
				 10, 7, 6,
				 7, 1, 8,

				 3, 9, 4,
				 3, 4, 2,
				 3, 2, 6,
				 3, 6, 8,
				 3, 8, 9,

				 4, 9, 5,
				 2, 4, 11,
				 6, 2, 10,
				 8, 6, 7,
				 9, 8, 1}};
		}
		// Get an array defining a standard icosahedron. A platonic solid with 20 faces, 30 edges and 12 vertices.
		// Uses get_icosahedron_points_and_indices to return a list of unique points and indices.
		//@return An array defining the vertex positions of the icosahedron
		[[nodiscard]] static consteval std::array<glm::vec3, 60> get_icosahedron_points()
		{
			constexpr auto points  = get_icosahedron_points_and_indices().first;
			constexpr auto indices = get_icosahedron_points_and_indices().second;

			std::array<glm::vec3, 60> points_flat = {};
			for (size_t i = 0; i < indices.size(); i++)
				points_flat[i] = points[indices[i]];

			return points_flat;
		}
	};
}