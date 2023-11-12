#pragma once

#include "Component/Mesh.hpp"
#include "OpenGL/GLState.hpp"
#include "Utility/Utility.hpp"

#include <array>
#include <vector>
#include <numbers>
#include <utility>
#include <type_traits>

namespace Utility
{
	template <typename VertexType = Data::Vertex, OpenGL::PrimitiveMode primitive_mode = OpenGL::PrimitiveMode::Triangles>
	requires Data::is_valid_mesh_vert<VertexType>
	class MeshBuilder
	{
		std::vector<VertexType> data;
		glm::vec4 current_colour;

	public:
		MeshBuilder() noexcept
			: data{}
			, current_colour{glm::vec4{1.f}}
		{}

		template <typename Vertex>
		void add_vertex(Vertex&& v)
		{
			static_assert(primitive_mode == OpenGL::PrimitiveMode::Points, "add_vertex requires MeshBuilder PrimitiveMode to be Points.");
			static_assert(std::is_same_v<std::decay_t<Vertex>, VertexType>, "Vertex type must match the MeshBuilder VertexType.");
			static_assert(!Data::has_normal_member<VertexType>, "add_vertex doesnt support normal data. Remove normal from Vertex.");
			static_assert(!Data::has_UV_member<VertexType>, "add_vertex doesnt support UV data. Remove UV from VertexType.");

			if constexpr (Data::has_colour_member<VertexType>)
				v.colour = current_colour;

			data.emplace_back(std::forward<Vertex>(v));
		}
		template <typename Vertex, typename Vertex2>
		void add_line(Vertex&& v1, Vertex2&& v2)
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
				add_line(std::forward<VertexType>(v1_t), std::forward<VertexType>(v2_t));
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
		// Add a triangle to the mesh.
		// If VertexType has one, calculates the normal from the positions. If the normal is pre-computed use the other overload.
		template <typename Vertex, typename Vertex2, typename Vertex3>
		void add_triangle(Vertex&& v1, Vertex2&& v2, Vertex3&& v3)
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
				add_triangle(std::forward<VertexType>(v1_t), std::forward<VertexType>(v2_t), std::forward<VertexType>(v3_t));
			}
			else if constexpr (std::is_same_v<std::decay_t<Vertex>, VertexType> && std::is_same_v<std::decay_t<Vertex2>, VertexType> && std::is_same_v<std::decay_t<Vertex3>, VertexType>)
			{
				if constexpr (Data::has_normal_member<VertexType>)
				{
					const auto edge1       = v2.position - v1.position;
					const auto edge2       = v3.position - v1.position;
					const auto calc_normal = glm::cross(edge1, edge2);
					add_triangle(std::forward<Vertex>(v1), std::forward<Vertex2>(v2), std::forward<Vertex3>(v3), calc_normal);
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
		// Add a triangle to the mesh.
		// Uses the provided normal. If the vertices dont have a normal, use the other overload.
		template <typename Vertex, typename Vertex2, typename Vertex3>
		void add_triangle(Vertex&& v1, Vertex2&& v2, Vertex3&& v3, const glm::vec3& normal)
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




		void add_circle(const glm::vec3& center, float radius, size_t segments, const glm::vec3& normal = {0.f, 1.f, 0.f})
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto points_and_UVs = get_circle_points(center, radius, segments, normal);

				for (auto i = 0; i < segments; ++i)
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
						add_triangle(v1, v2, v3, normal);
					else
						add_triangle(v1, v2, v3);
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_circle for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		void add_quad(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right)
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
						add_triangle(top_left_v, bottom_left_v, bottom_right_v, normal);
						add_triangle(top_left_v, bottom_right_v, top_right_v, normal);
					}
					else
					{
						add_triangle(top_left_v, bottom_left_v, bottom_right_v);
						add_triangle(top_left_v, bottom_right_v, top_right_v);
					}
				}
				else if constexpr (primitive_mode == OpenGL::PrimitiveMode::Lines)
				{
					add_line(top_left_v, bottom_left_v);
					add_line(bottom_left_v, bottom_right_v);
					add_line(bottom_right_v, top_right_v);
					add_line(top_right_v, top_left_v);
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_quad for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		void add_cone(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments)
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto top_to_base    = glm::normalize(base - top);
				const auto points_and_UVs = get_circle_points(base, radius, segments, top_to_base);

				for (auto i = 0; i < segments; ++i)
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

					add_triangle(v1, v2, v3);
				}

				add_circle(base, radius, segments, top_to_base);
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_cone for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		void add_cylinder(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments)
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

				for (auto i = 0; i < segments; ++i)
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
						add_triangle(base_vertex_1, top_vertex_1, base_vertex_2);
						add_triangle(base_vertex_2, top_vertex_1, top_vertex_2);
					}
					{// Add the triangles for the circles at the base and top of the cylinder.
					 // Though identical to MeshBuilder::add_circle, we dont want to call get_circle_points more than once so we reuse the base_circle_points.
					 	if constexpr (Data::has_normal_member<VertexType>)
						{
							add_triangle(base_vertex_2, base_vertex_center, base_vertex_1, top_to_base_dir);
							add_triangle(top_vertex_1, top_vertex_center, top_vertex_2, base_to_top_dir);
						}
						else
						{
							add_triangle(base_vertex_2, base_vertex_center, base_vertex_1);
							add_triangle(top_vertex_1, top_vertex_center, top_vertex_2);
						}
					}
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_cylinder for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
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

				for (auto i = 0; i < subdivisions; i++)
				{
					std::vector<glm::vec3> new_points;
					new_points.reserve(points.size() * 4);

					// Going through each triangle, subdivide it into 4 triangles.
					for (auto i = 0; i < points.size(); i += 3)
					{
						const auto a = (points[i]     + points[i + 1]) / 2.f;
						const auto b = (points[i + 1] + points[i + 2]) / 2.f;
						const auto c = (points[i + 2] + points[i])     / 2.f;

						// T1
						new_points.emplace_back(points[i]);
						new_points.emplace_back(a);
						new_points.emplace_back(c);
						// T2
						new_points.emplace_back(points[i + 1]);
						new_points.emplace_back(b);
						new_points.emplace_back(a);
						// T3
						new_points.emplace_back(points[i + 2]);
						new_points.emplace_back(c);
						new_points.emplace_back(b);
						// T4
						new_points.emplace_back(a);
						new_points.emplace_back(b);
						new_points.emplace_back(c);
					}
					points = std::move(new_points);
				}

				for (auto i = 0; i < points.size(); i += 3)
				{
					points[i]     = ((points[i]     / glm::length(points[i]    )) * radius) + center;
					points[i + 1] = ((points[i + 1] / glm::length(points[i + 1])) * radius) + center;
					points[i + 2] = ((points[i + 2] / glm::length(points[i + 2])) * radius) + center;

					add_triangle(points[i], points[i + 1], points[i + 2]);
				}
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_icosphere for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		void add_cuboid(const glm::vec3& center, const glm::vec3& size, const glm::quat& rotation = glm::identity<glm::quat>())
		{
			if constexpr (primitive_mode == OpenGL::PrimitiveMode::Triangles || primitive_mode == OpenGL::PrimitiveMode::Lines)
			{
				const auto half_size = size / 2.f;

				// Bottom face
				const auto p1 = center + glm::vec3(-half_size.x, -half_size.y, -half_size.z);
				const auto p2 = center + glm::vec3(-half_size.x, -half_size.y,  half_size.z);
				const auto p3 = center + glm::vec3( half_size.x, -half_size.y,  half_size.z);
				const auto p4 = center + glm::vec3( half_size.x, -half_size.y, -half_size.z);
				// Top face
				const auto p5 = center + glm::vec3(-half_size.x,  half_size.y, -half_size.z);
				const auto p6 = center + glm::vec3(-half_size.x,  half_size.y,  half_size.z);
				const auto p7 = center + glm::vec3( half_size.x,  half_size.y,  half_size.z);
				const auto p8 = center + glm::vec3( half_size.x,  half_size.y, -half_size.z);

				add_quad(p7, p8, p3, p4); // Left
				add_quad(p5, p6, p1, p2); // Right
				add_quad(p2, p3, p1, p4); // Bottom
				add_quad(p5, p8, p6, p7); // Top
				add_quad(p6, p7, p2, p3); // Front
				add_quad(p8, p5, p4, p1); // Back
			}
			else
				[]<bool flag=false>(){ static_assert(flag, "Not implemented add_cuboid for this primitive_mode."); }(); // #CPP23 P2593R0 swap for static_assert(false)
		}
		void reserve(size_t size)
		{
			data.reserve(size);
		}
		void clear()
		{
			data.clear();
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
		[[NODISCARD]] Data::Mesh get_mesh()
		{
			return Data::Mesh{data, primitive_mode};
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
			const auto rotation = Utility::getRotation(glm::vec3{0.f, 1.f, 0.f}, normal); // Get the rotation quaternion to rotate the circle to the correct orientation.
			const float angle_step = 2.0f * std::numbers::pi_v<float> / segments;

			for (auto i = 0; i < segments; ++i)
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
			for (auto i = 0; i < indices.size(); i++)
				points_flat[i] = points[indices[i]];

			return points_flat;
		}
	};
}