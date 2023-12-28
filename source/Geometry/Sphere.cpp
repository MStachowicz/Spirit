#include "Sphere.hpp"

#include "glm/mat4x4.hpp"
#include "imgui.h"

void Geometry::Sphere::transform(const glm::mat4& p_transform, const glm::vec3& p_scale)
{
	m_center = glm::vec3(p_transform * glm::vec4(m_center, 1.f));
	m_radius *= p_scale.x;
}

void Geometry::Sphere::draw_UI() const
{
	ImGui::SeparatorText("Sphere");
	ImGui::Text("Center", m_center);
	ImGui::Text("Radius", m_radius);
}