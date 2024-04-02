#pragma once

#include "glm/gtx/quaternion.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include <array>
#include <fstream>

namespace Component
{
	// Represents the position, orientation and scale of an object in the world.
	struct Transform
	{
		constexpr static size_t Persistent_ID = 0;
		constexpr static glm::vec3 Starting_Forward_Direction = glm::vec3(0.f, 0.f, -1.f);

		enum class MoveDirection : uint8_t { Forward, Backward, Left, Right, Up, Down };

		constexpr Transform(const glm::vec3& p_position = glm::vec3(0.f)) noexcept
			: m_position{p_position}
			, m_scale{glm::vec3(1.0f)}
			, m_orientation{glm::identity<glm::quat>()}
		{}

		glm::vec3 m_position;       // World-space position.
		glm::vec3 m_scale;          // Scale in each axis.
		glm::quat m_orientation;    // Unit quaternion taking the Starting_Forward_Direction to the current forward direction.

		// Rotate the object to roll pitch and yaw euler angles in the order XYZ. Angles suppled are in degrees.
		void rotate_euler_degrees(const glm::vec3& p_roll_pitch_yaw_degrees);
		// Focus the orientation/forward to p_point.
		void look_at(const glm::vec3& p_point);
		// Set the model matrix directly.
		void set_model(const glm::mat4& p_model);

		glm::mat4 get_model() const { return glm::translate(glm::identity<glm::mat4>(), m_position) * glm::mat4_cast(m_orientation) * glm::scale(glm::identity<glm::mat4>(), m_scale); }
		glm::vec3 forward()   const { return glm::normalize(m_orientation * Starting_Forward_Direction); };
		glm::vec3 right()     const { return glm::normalize(m_orientation * glm::vec3(1.f, 0.f, 0.f));   };
		glm::vec3 up()        const { return glm::normalize(m_orientation * glm::vec3(0.f, 1.f, 0.f));   };
		// Get the local space XYZ vectors (XYZ = right, up, forward).
		std::array<glm::vec3, 3> get_local_axes() const { return {right(), up(), forward()}; };

		void draw_UI();
		static void Serialise(const Transform& p_transform, std::ofstream& p_out, uint16_t p_version);
		static Transform Deserialise(std::ifstream& p_in, uint16_t p_version);
	};
}