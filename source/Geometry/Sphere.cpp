#include "Sphere.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"

#include "glm/mat4x4.hpp"
#include "imgui.h"

void Geometry::Sphere::transform(const glm::mat4& p_transform, const glm::vec3& p_scale)
{
	m_center = glm::vec3(p_transform * glm::vec4(m_center, 1.f));

	// The scaling in x, y and z axis must be equal because the sphere is a perfect sphere.
	ASSERT_THROW(Utility::equal_floats(p_scale.x, p_scale.y) && Utility::equal_floats(p_scale.x, p_scale.z), "Sphere scaling must be uniform - Sphere has to remain a perfect sphere.");
	m_radius *= p_scale.x;
}

void Geometry::Sphere::draw_UI() const
{
	ImGui::SeparatorText("Sphere");
	ImGui::Text("Center", m_center);
	ImGui::Text("Radius", m_radius);
}