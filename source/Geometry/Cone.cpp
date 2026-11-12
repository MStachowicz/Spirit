#include "Cone.hpp"

#include "imgui.h"

namespace Geometry
{
	void Cone::draw_UI() const
	{
		ImGui::SeparatorText("Cone");
		ImGui::Text("Half height", m_half_height);
		ImGui::Text("Base radius", m_base_radius);
	}
} // namespace Geometry