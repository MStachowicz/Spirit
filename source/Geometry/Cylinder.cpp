#include "Cylinder.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"

#include "glm/mat4x4.hpp"

#include "imgui.h"

void Geometry::Cylinder::transform(const glm::mat4& p_model, const glm::vec3& p_scale)
{
	m_base = glm::vec3(p_model * glm::vec4(m_base, 1.f));
	m_top  = glm::vec3(p_model * glm::vec4(m_top, 1.f));

	// The scaling in x and z axis must be equal because the cone is a circular cone.
	ASSERT_THROW(Utility::equal_floats(p_scale.x, p_scale.z), "[CYLINDER] Scaling in x and z axis must be equal.");
	m_radius *= p_scale.x;
}

void Geometry::Cylinder::draw_UI() const
{
	ImGui::SeparatorText("Cylinder");
	ImGui::Text("Base", m_base);
	ImGui::Text("Top", m_top);
	ImGui::Text("Radius", m_radius);
}
