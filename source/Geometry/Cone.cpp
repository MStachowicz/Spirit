#include "Cone.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"

#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "imgui.h"

namespace Geometry
{
	void Cone::transform(const glm::mat4& p_model, const glm::vec3& p_scale)
	{
		m_base = glm::vec3(p_model * glm::vec4(m_base, 1.f));
		m_top  = glm::vec3(p_model * glm::vec4(m_top, 1.f));

		// The scaling in x and z axis must be equal because the cone is a circular cone.
		ASSERT_THROW(Utility::equal_floats(p_scale.x, p_scale.z), "[CONE] Scaling in x and z axis must be equal - cone must stay right-circular.");
		m_base_radius *= p_scale.x;
	}
	void Cone::draw_UI() const
	{
		ImGui::SeparatorText("Cone");
		ImGui::Text("Base", m_base);
		ImGui::Text("Top", m_top);
		ImGui::Text("Base radius", m_base_radius);
	}
} // namespace Geometry