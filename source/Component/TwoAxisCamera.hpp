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
		float m_pan_sensitivity;

		float m_pitch;            // Pitch angle of view in radians.
		float m_yaw;              // Yaw angle of view in radians.
		float m_orbit_radius;     // Controls the radial distance between the camera and the point it is orbiting.
		glm::vec3 m_orbit_center; // The world space position serving as the orbit center or target point for the camera.

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
		// Process mouse p_offset to apply a change to the orbit radius of the camera.
		//@param p_offset XY offset to apply in raw input data.
		void pan(const glm::vec2& p_offset);
		// Process mouse scrollwheel p_offset to apply a change to the orbit radius of the camera.
		void zoom(float p_offset);

		void draw_UI();
	};
}