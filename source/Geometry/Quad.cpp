#include "Quad.hpp"
#include "Plane.hpp"
#include "Triangle.hpp"

#include "glm/glm.hpp"
#include "imgui.h"

namespace Geometry
{
	Quad::Quad(const glm::vec3& p_point, const glm::vec3& p_normal) noexcept
	{
		glm::vec3 right;
		if (p_normal.x != 0 || p_normal.z != 0)
			right = glm::normalize(glm::vec3(p_normal.z, 0.f, -p_normal.x));
		else
			right = glm::normalize(glm::vec3(p_normal.y, -p_normal.x, 0.f));

		glm::vec3 up = -glm::cross(right, p_normal);

		m_top_left     = glm::vec3(p_point - right + up);
		m_top_right    = glm::vec3(p_point + right + up);
		m_bottom_left  = glm::vec3(p_point - right - up);
		m_bottom_right = glm::vec3(p_point + right - up);
	}
	Quad::Quad(const Plane& p_plane) noexcept
	{
		// Since Plane has no real position, we construct the quad at the arbitrary center-point represented by the closest point
		// on the plane to the origin.
		const glm::vec3 p = p_plane.m_normal * p_plane.m_distance;
		auto quad         = Quad(p, p_plane.m_normal);
		m_top_left        = quad.m_top_left;
		m_top_right       = quad.m_top_right;
		m_bottom_left     = quad.m_bottom_left;
		m_bottom_right    = quad.m_bottom_right;
	}
	void Quad::transform(const glm::mat4& p_transform)
	{
		m_top_left     = glm::vec3(p_transform * glm::vec4(m_top_left, 1.f));
		m_top_right    = glm::vec3(p_transform * glm::vec4(m_top_right, 1.f));
		m_bottom_left  = glm::vec3(p_transform * glm::vec4(m_bottom_left, 1.f));
		m_bottom_right = glm::vec3(p_transform * glm::vec4(m_bottom_right, 1.f));
	}
	void Quad::draw_UI() const
	{
		ImGui::SeparatorText("Quad");
		ImGui::Text("Point 1", m_top_left);
		ImGui::Text("Point 2", m_top_right);
		ImGui::Text("Point 3", m_bottom_left);
		ImGui::Text("Point 4", m_bottom_right);
		ImGui::Text("Center", center());
	}
	Quad::Quad(const Triangle& p_triangle) noexcept
	{
		// First find the up and right directions local to p_triangle.
		// Next using the up, down, left and right directions find the how much to scale the unit quad to encompass the triangle,
		// by finding the largest magnitude dot products of points from the centroid of p_triangle to the edges.
		glm::vec3 right;
		if (p_triangle.normal().x != 0 || p_triangle.normal().z != 0)
			right = glm::normalize(glm::vec3(p_triangle.normal().z, 0.f, -p_triangle.normal().x));
		else
			right = glm::normalize(glm::vec3(p_triangle.normal().y, -p_triangle.normal().x, 0.f));

		const glm::vec3 up = -glm::normalize(glm::cross(right, p_triangle.normal()));
		float most_up_value      = 0;
		float most_down_value    = 0;
		float most_left_value    = 0;
		float most_right_value   = 0;
		auto triangle_center = p_triangle.centroid();
		std::array<glm::vec3, 3> triangle_center_to_edges = {p_triangle.m_point_1 - triangle_center, p_triangle.m_point_2 - triangle_center, p_triangle.m_point_3 - triangle_center};

		for (uint8_t i = 0; i < triangle_center_to_edges.size(); i++)
		{
			auto dot_up    = glm::dot(triangle_center_to_edges[i], up);
			auto dot_right = glm::dot(triangle_center_to_edges[i], right);
			auto dot_down  = glm::dot(triangle_center_to_edges[i], -up);
			auto dot_left  = glm::dot(triangle_center_to_edges[i], -right);

			if (dot_up > most_up_value)
				most_up_value = dot_up;
			if (dot_down > most_down_value)
				most_down_value = dot_down;
			if (dot_right > most_right_value)
				most_right_value = dot_right;
			if (dot_left > most_left_value)
				most_left_value = dot_left;
		}

		m_top_left     = glm::vec3(triangle_center + (right  * most_right_value) + (up  * most_up_value));
		m_top_right    = glm::vec3(triangle_center + (-right * most_left_value)  + (up  * most_up_value));
		m_bottom_left  = glm::vec3(triangle_center + (-right * most_left_value)  + (-up * most_down_value));
		m_bottom_right = glm::vec3(triangle_center + (right  * most_right_value) + (-up * most_down_value));
	}

	glm::vec3 Quad::center() const
	{
		return (m_top_left + m_top_right + m_bottom_left + m_bottom_right) / 4.0f;
	}
	void Quad::scale(const float p_scale)
	{
		const auto c = center();
		m_top_left = m_top_left + (glm::normalize(m_top_left - c) * p_scale);
		m_top_right = m_top_right + (glm::normalize(m_top_right - c) * p_scale);
		m_bottom_left = m_bottom_left + (glm::normalize(m_bottom_left - c) * p_scale);
		m_bottom_right = m_bottom_right + (glm::normalize(m_bottom_right - c) * p_scale);
	}

	std::array<Triangle, 2> Quad::get_triangles() const
	{
		return std::array<Triangle, 2>({Triangle{m_top_left, m_top_right, m_bottom_left}, Triangle{m_bottom_left, m_bottom_right, m_top_left}});
	}
}