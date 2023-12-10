#include "Triangle.hpp"

#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"

namespace Geometry
{
	glm::vec3 Triangle::centroid() const
	{
		// calculates the arithmetic mean of the three points of the triangle, which gives the center of the triangle.
		return (m_point_1 + m_point_2 + m_point_3) / 3.0f;
	}
	glm::vec3 Triangle::normal() const
	{
		return glm::normalize(glm::cross(m_point_2 - m_point_1, m_point_3 - m_point_1));
	}

	void Triangle::transform(const glm::mat4& p_transform)
	{
		m_point_1 = glm::vec3(p_transform * glm::vec4(m_point_1, 1.f));
		m_point_2 = glm::vec3(p_transform * glm::vec4(m_point_2, 1.f));
		m_point_3 = glm::vec3(p_transform * glm::vec4(m_point_3, 1.f));
	}

	void Triangle::translate(const glm::vec3& p_translate)
	{
		m_point_1 += p_translate;
		m_point_2 += p_translate;
		m_point_3 += p_translate;
	}

	void Triangle::draw_UI() const
	{
		ImGui::SeparatorText("Triangle");
		ImGui::Text("Point 1", m_point_1);
		ImGui::Text("Point 2", m_point_2);
		ImGui::Text("Point 3", m_point_3);
		ImGui::Text("Centroid", centroid());
		ImGui::Text("Normal", normal());
	}

	std::array<Triangle, 4> Triangle::subdivide() const
	{
		// Find the midpoint of all 3 edges and construct the 4 Triangles.
		const auto a = (m_point_1 + m_point_2) / 2.f;
		const auto b = (m_point_2 + m_point_3) / 2.f;
		const auto c = (m_point_3 + m_point_1) / 2.f;
		return {Triangle{m_point_1, a, c}, Triangle{m_point_2, b, a}, Triangle{m_point_3, c, b}, Triangle{a, b, c}};
	}
}