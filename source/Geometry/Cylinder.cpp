#include "Cylinder.hpp"

#include "imgui.h"

void Geometry::Cylinder::draw_UI() const
{
	ImGui::SeparatorText("Cylinder");
	ImGui::Text("Half height", m_half_height);
	ImGui::Text("Radius", m_radius);
}
