#include "Sphere.hpp"

#include "imgui.h"

void Geometry::Sphere::draw_UI() const
{
	ImGui::SeparatorText("Sphere");
	ImGui::Text("Radius", m_radius);
}