#pragma once

#include "ViewInformation.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Component
{
	class TwoAxisCamera
	{
	public:
		float m_FOV; // Field of view in degrees.
		float m_near;
		float m_far;

		float m_look_sensitivity;
		float m_zoom_sensitivity;

		float m_distance; // Distance from the target.
		float m_pitch;    // Pitch angle of view in radians.
		float m_yaw;      // Yaw angle of view in radians.

	public:
		TwoAxisCamera();

		glm::vec3 right() const;
		glm::vec3 up() const;
		glm::vec3 forward() const;
		glm::vec3 position() const;

		glm::mat4 view() const;
		ViewInformation view_information(const float& p_aspect_ratio) const;

		// Process mouse p_offset to apply a change to the view direction of the camera.
		//@param p_offset XY offset to apply in raw input data.
		void mouse_look(const glm::vec2& p_offset);
		void mouse_scroll(float p_offset);

		void draw_UI();
	};
}