#pragma once

#include "Logger.hpp"

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include "OpenGL/Types.hpp"
#include "OpenGL/GLState.hpp"

#include "glm/glm.hpp"

#include <vector>
#include <numbers>

namespace Utility
{
	class Vertex
	{
	public:
		glm::vec3 position = glm::vec3{0.f};
		glm::vec3 normal   = glm::vec3{0.f};
		glm::vec2 uv       = glm::vec2{0.f};
		glm::vec3 colour   = glm::vec3{1.f};
	};

	class Mesh
	{
		OpenGL::VAO VAO   = {};
		OpenGL::VBO VBO   = {};
		GLsizei draw_size = 0;
		OpenGL::PrimitiveMode primitive_mode;

	public:
		void draw()
		{
			VAO.bind();
			OpenGL::draw_arrays(primitive_mode, 0, draw_size);
		}

		Mesh(const std::vector<Vertex>& vertex_data, OpenGL::PrimitiveMode primitive_mode)
			: VAO{}
			, VBO{}
			, draw_size{(GLsizei)vertex_data.size()}
			, primitive_mode{primitive_mode}
		{
			VAO.bind();
			VBO.bind();

			OpenGL::buffer_data(OpenGL::BufferType::ArrayBuffer, vertex_data.size() * sizeof(Vertex), vertex_data.data(), OpenGL::BufferUsage::StaticDraw);

			OpenGL::vertex_attrib_pointer(0, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
			OpenGL::vertex_attrib_pointer(1, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			OpenGL::vertex_attrib_pointer(3, 2, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, uv));
			OpenGL::vertex_attrib_pointer(2, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, colour));

			OpenGL::enable_vertex_attrib_array(0);
			OpenGL::enable_vertex_attrib_array(1);
			OpenGL::enable_vertex_attrib_array(2);
			OpenGL::enable_vertex_attrib_array(3);
		}
	};

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
				data.emplace_back(Vertex{position, normal, uv, current_colour});
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

					data.emplace_back(Vertex{p1, calc_normal, uv1, current_colour});
					data.emplace_back(Vertex{p2, calc_normal, uv2, current_colour});
					data.emplace_back(Vertex{p3, calc_normal, uv3, current_colour});
				}
				else
				{
				data.emplace_back(Vertex{p1, normal, uv1, current_colour});
				data.emplace_back(Vertex{p2, normal, uv2, current_colour});
				data.emplace_back(Vertex{p3, normal, uv3, current_colour});
				}
			}
			else
				ASSERT(false, "add_triangle for this primitive mode is not supported.");

		}
		void add_circle(const glm::vec3& center, float radius, size_t segments)
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const float angle_step = 2.0f * std::numbers::pi_v<float> / segments;

				for (auto i = 0; i < segments; ++i)
				{
					float angle     = i * angle_step;
					float nextAngle = (i + 1) * angle_step;

					glm::vec3 p1 = center + radius * glm::vec3(glm::cos(angle), 0, glm::sin(angle));
					glm::vec3 p2 = center + radius * glm::vec3(glm::cos(nextAngle), 0, glm::sin(nextAngle));

					add_triangle(p1, p2, center, glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(0.5f, 1.f));
				}
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
				const float angle_step = 2.0f * std::numbers::pi_v<float> / segments;

				for (auto i = 0; i < segments; ++i)
				{
					float angle     = i * angle_step;
					float nextAngle = (i + 1) * angle_step;

					glm::vec3 p1 = base + radius * glm::vec3(glm::cos(nextAngle), 0, glm::sin(nextAngle));
					glm::vec3 p2 = base + radius * glm::vec3(glm::cos(angle),     0, glm::sin(angle));

					add_triangle(p1, p2, top, glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(0.5f, 1.f));
				}
				add_circle(base, radius, segments);
			}
			else
				ASSERT(false, "add_cone for this primitive mode is not supported.");
		}
		void add_cylinder(const glm::vec3& base, const glm::vec3& top, float radius, size_t segments)
		{
			if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
			{
				const float angle_step = 2.0f * std::numbers::pi_v<float> / segments;

				for (auto i = 0; i < segments; ++i)
				{
					float angle     = i * angle_step;
					float nextAngle = (i + 1) * angle_step;

					glm::vec3 p1 = base + radius * glm::vec3(glm::cos(angle), 0, glm::sin(angle));
					glm::vec3 p2 = base + radius * glm::vec3(glm::cos(nextAngle), 0, glm::sin(nextAngle));
					glm::vec3 p3 = top + radius * glm::vec3(glm::cos(angle), 0, glm::sin(angle));
					glm::vec3 p4 = top + radius * glm::vec3(glm::cos(nextAngle), 0, glm::sin(nextAngle));

					add_triangle(p1, p2, p3, glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(0.5f, 1.f));
					add_triangle(p2, p4, p3, glm::vec2(1.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(0.5f, 1.f));
				}

				add_circle(base, radius, segments);
				add_circle(top, radius, segments);
			}
			else
				ASSERT(false, "add_cylinder for this primitive mode is not supported.");
		}
		void resereve(size_t size)
		{
			data.reserve(size);
		}
		void set_colour(const glm::vec3& colour)
		{
			current_colour = colour;
		}
		[[NODISCARD]] Mesh get_mesh()
		{
			return Mesh{data, primitive_mode};
		}

	private:
		std::vector<Vertex> data;
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