#include "Cuboid.hpp"

#include "Utility/Utility.hpp"

#include "glm/vec4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "imgui.h"

namespace Geometry
{
	void Cuboid::draw_UI() const
	{
		ImGui::SeparatorText("Cuboid");
		ImGui::Text("Half extents", m_half_extents);
	}
	std::array<glm::vec3, 8> Cuboid::get_vertices(const glm::vec3& p_position, const glm::quat& p_orientation) const noexcept
	{
		// The function calculates the vertices of the cuboid based on its position, orientation, and half extents.
		// First, it calculates the rotated x, y, and z axes by applying the orientation to the unit vectors along these axes.
		// Then, it scales these axes by the half extents to get the vectors from the center of the cuboid to its faces.
		// Finally, it adds and subtracts these vectors to/from the position of the cuboid to get the vertices.
		// Each vertex is the sum of the position and three vectors, each of which is either x, y, z or their negations.
		// The eight combinations of plus and minus give the eight vertices.

		const glm::vec3 x_axis = p_orientation * glm::vec3(1.f, 0.f, 0.f);
		const glm::vec3 y_axis = p_orientation * glm::vec3(0.f, 1.f, 0.f);
		const glm::vec3 z_axis = p_orientation * glm::vec3(0.f, 0.f, 1.f);

		const glm::vec3 x = x_axis * m_half_extents.x;
		const glm::vec3 y = y_axis * m_half_extents.y;
		const glm::vec3 z = z_axis * m_half_extents.z;

		return std::array<glm::vec3, 8>{
			p_position + x + y + z, // 0 Top     right  front
			p_position + x + y - z, // 1 Top     right  back
			p_position - x + y + z, // 2 Top     left   front
			p_position - x + y - z, // 3 Top     left   back
			p_position + x - y + z, // 4 Bottom  right  front
			p_position + x - y - z, // 5 Bottom  right  back
			p_position - x - y + z, // 6 Bottom  left   front
			p_position - x - y - z  // 7 Bottom  left   back
		};
	}
} // namespace Geometry