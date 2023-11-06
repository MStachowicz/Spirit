#pragma once

#include "Logger.hpp"

#include "OpenGL/Types.hpp"
#include "OpenGL/GLState.hpp"

#include "Component/Mesh.hpp"

#include "Utility/Utility.hpp"

#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <vector>
#include <numbers>
#include <utility>

namespace Utility
{
	class MeshBuilder
	{
	public:
		MeshBuilder(OpenGL::PrimitiveMode mode) noexcept
			: data{}
			, current_colour{glm::vec3{1.f}}
			, primitive_mode{mode}
		{}

		void add_vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2 uv = {0,0})
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Points)
				data.emplace_back(Data::Vertex{position, normal, uv, current_colour});
			else
				ASSERT(false, "add_vertex for this primitive mode is not supported.");
		}
		void add_triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec2 uv1 = {0, 0}, const glm::vec2 uv2 = {0, 0}, const glm::vec2 uv3 = {0, 0}, const glm::vec3& normal = {0.f, 0.f, 0.f})
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				if (normal == glm::vec3{0.f})
				{
					const auto v1          = p2 - p1;
					const auto v2          = p3 - p1;
					const auto calc_normal = glm::cross(v1, v2);

					data.emplace_back(Data::Vertex{p1, calc_normal, uv1, current_colour});
					data.emplace_back(Data::Vertex{p2, calc_normal, uv2, current_colour});
					data.emplace_back(Data::Vertex{p3, calc_normal, uv3, current_colour});
				}
				else
				{
					data.emplace_back(Data::Vertex{p1, normal, uv1, current_colour});
					data.emplace_back(Data::Vertex{p2, normal, uv2, current_colour});
					data.emplace_back(Data::Vertex{p3, normal, uv3, current_colour});
				}
			}
			else
				ASSERT(false, "add_triangle for this primitive mode is not supported.");

		}
		void add_circle(const glm::vec3& center, float radius, size_t segments, const glm::vec3& normal = {0.f, 1.f, 0.f})
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto points_and_UVs = get_circle_points(center, radius, segments, normal);

				for (auto i = 0; i < segments; ++i)
					add_triangle(
						points_and_UVs[(i + 1) % segments].first,
						center,
						points_and_UVs[i].first,
						glm::vec2(0.5f) - (points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f)),
						glm::vec2{0.5f, 0.5f},
						glm::vec2(0.5f) - (points_and_UVs[i].second * glm::vec2(0.5f, -0.5f)),
						normal);
			}
			else
				ASSERT(false, "add_circle for this primitive mode is not supported.");
		}
		void add_quad(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right)
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{// Reverse winding order to ensure the normal is facing the correct way.
				add_triangle(top_left, bottom_left, bottom_right,     glm::vec2(0.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f , 0.f));
				add_triangle(top_left, bottom_right, top_right,       glm::vec2(0.f, 1.f), glm::vec2(1.f, 0.f), glm::vec2(1.f , 1.f));
			}
			else
				ASSERT(false, "add_quad for this primitive mode is not supported.");
		}
		void add_cone(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments)
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto top_to_base    = glm::normalize(base - top);
				const auto points_and_UVs = get_circle_points(base, radius, segments, top_to_base);

				for (auto i = 0; i < segments; ++i)
					add_triangle(
						points_and_UVs[i].first,
						top,
						points_and_UVs[(i + 1) % segments].first,
						glm::vec2(0.5f) - (points_and_UVs[i].second * glm::vec2(0.5f, -0.5f))),
						glm::vec2{0.5f, 0.5f},
						glm::vec2(0.5f) - (points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f));

				add_circle(base, radius, segments, top_to_base);
			}
			else
				ASSERT(false, "add_cone for this primitive mode is not supported.");
		}
		void add_cylinder(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments)
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const auto base_to_top                = top - base;
				const auto base_to_top_dir            = glm::normalize(base_to_top);
				const auto top_to_base_dir            = glm::normalize(base - top);
				const auto base_circle_points_and_UVs = get_circle_points(base, radius, segments, top_to_base_dir);

				for (auto i = 0; i < segments; ++i)
				{
					{// Add the triangles for the side of the cylinder. Go in triangle pairs forming quads along the side.
						add_triangle(
							base_circle_points_and_UVs[i].first,
							base_circle_points_and_UVs[i].first + base_to_top,
							base_circle_points_and_UVs[(i + 1) % segments].first,
							glm::vec2(0.5f) - (base_circle_points_and_UVs[i].second * glm::vec2(0.5f, -0.5f))),
							glm::vec2{0.5f, 0.5f},
							glm::vec2(0.5f) - (base_circle_points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f));
						add_triangle(
							base_circle_points_and_UVs[(i + 1) % segments].first,
							base_circle_points_and_UVs[i].first + base_to_top,
							base_circle_points_and_UVs[(i + 1) % segments].first + base_to_top,
							glm::vec2(0.5f) - (base_circle_points_and_UVs[i].second * glm::vec2(0.5f, -0.5f))),
							glm::vec2{0.5f, 0.5f},
							glm::vec2(0.5f) - (base_circle_points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f));
					}
					{// Add the triangles for the circles at the base and top of the cylinder.
						// Though identical to MeshBuilder::add_circle, we dont want to call get_circle_points more than once so we reuse the base_circle_points.
						add_triangle(// Base triangle
							base_circle_points_and_UVs[(i + 1) % segments].first,
							base,
							base_circle_points_and_UVs[i].first,
							glm::vec2(0.5f) - (base_circle_points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f)),
							glm::vec2{0.5f, 0.5f},
							glm::vec2(0.5f) - (base_circle_points_and_UVs[i].second * glm::vec2(0.5f, -0.5f)),
							top_to_base_dir);
						add_triangle( // Top triangle. Flip order of vertices to reverse winding order.
							base_circle_points_and_UVs[i].first + base_to_top,
							base + base_to_top,
							base_circle_points_and_UVs[(i + 1) % segments].first + base_to_top,
							glm::vec2(0.5f) - (base_circle_points_and_UVs[(i + 1) % segments].second * glm::vec2(0.5f, -0.5f)),
							glm::vec2{0.5f, 0.5f},
							glm::vec2(0.5f) - (base_circle_points_and_UVs[i].second * glm::vec2(0.5f, -0.5f)),
							-top_to_base_dir);
					}
				}
			}
			else
				ASSERT(false, "add_cylinder for this primitive mode is not supported.");
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
		void resereve(size_t size)
		{
			data.reserve(size);
		}
		void set_colour(const glm::vec3& colour)
		{
			current_colour = colour;
		}
		[[NODISCARD]] Data::NewMesh get_mesh()
		{
			return Data::NewMesh{data, primitive_mode};
		}

	private:
		std::vector<Data::Vertex> data;
		glm::vec3 current_colour;
		const OpenGL::PrimitiveMode primitive_mode;

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
	};
}