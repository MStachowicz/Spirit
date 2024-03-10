#include "TwoAxisCamera.hpp"
#include "Platform/Input.hpp"
#include "Utility/Logger.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"

namespace Component
{
	TwoAxisCamera::TwoAxisCamera()
		: m_FOV{90.f}
		, m_near{0.001f}
		, m_far{100.f}
		, m_look_sensitivity{0.5f}
		, m_zoom_sensitivity{1.f}
		, m_pan_sensitivity{0.05f}
		, m_pitch{0.f}
		, m_yaw{0.f}
		, m_orbit_center{0.f, 0.f, 0.f}
		, m_orbit_radius{10.f}
	{}

	glm::vec3 TwoAxisCamera::up() const
	{
		auto up = glm::vec3(0.f, 1.f, 0.f);
		up = glm::rotate(up, m_pitch, glm::vec3(1.f, 0.f, 0.f));
		up = glm::rotate(up, m_yaw,   glm::vec3(0.f, 1.f, 0.f));
		return up;
	}
	glm::vec3 TwoAxisCamera::right() const
	{
		auto right = glm::vec3(1.f, 0.f, 0.f);
		right = glm::rotate(right, m_pitch, glm::vec3(1.f, 0.f, 0.f));
		right = glm::rotate(right, m_yaw,   glm::vec3(0.f, 1.f, 0.f));
		return right;
	}
	glm::vec3 TwoAxisCamera::forward() const
	{
		auto forward = glm::vec3(0.0f, 0.0f, -1.0f);
		forward = glm::rotate(forward, m_pitch, glm::vec3(1.f, 0.f, 0.f));
		forward = glm::rotate(forward, m_yaw,   glm::vec3(0.f, 1.f, 0.f));
		return forward;
	}
	glm::vec3 TwoAxisCamera::position() const
	{
		return m_orbit_center - m_orbit_radius * forward();
	}
	glm::mat4 TwoAxisCamera::view() const
	{
		return glm::lookAt(position(), m_orbit_center, up());
	}
	ViewInformation TwoAxisCamera::view_information(const float& p_aspect_ratio) const
	{
		ViewInformation view_info;
		view_info.m_view_position = position();
		view_info.m_view          = view();
		view_info.m_projection    = glm::perspective(glm::radians(m_FOV), p_aspect_ratio, m_near, m_far);
		return view_info;
	}

	void TwoAxisCamera::mouse_look(const glm::vec2& p_offset)
	{
		m_yaw   -= glm::radians(p_offset.x * m_look_sensitivity);
		m_pitch += glm::radians(p_offset.y * m_look_sensitivity);

		// The constraint is applied to the pitch angle to prevent the camera from flipping upside down
		constexpr float pitch_constraint = glm::radians(90.f) - 0.01f;
		if      (m_pitch >  pitch_constraint)  m_pitch =  pitch_constraint;
		else if (m_pitch < -pitch_constraint)  m_pitch = -pitch_constraint;
	}
	void TwoAxisCamera::pan(const glm::vec2& p_offset)
	{
		glm::vec3 pan_offset = -p_offset.x * right() - p_offset.y * up();
		m_orbit_center += pan_offset * m_pan_sensitivity;
	}
	void TwoAxisCamera::zoom(float p_offset)
	{
		constexpr float zoom_near_constraint = 0.1f;

		// Adjust the distance from the orbit center based on the scroll offset
		m_orbit_radius -= p_offset * m_zoom_sensitivity;

		// Apply a lower constraint to prevent the camera from getting too close to the orbit center
		if (m_orbit_radius < zoom_near_constraint)
			m_orbit_radius = zoom_near_constraint;
	}

	void TwoAxisCamera::draw_UI()
	{
		ImGui::Begin("Camera options");
		ImGui::Slider("FOV",  m_FOV,  1.f,    90.f, "%.3f°");
		ImGui::Slider("Near", m_near, 0.001f, 10.f);
		ImGui::Slider("Far",  m_far,  1.f,    1000.f);
		ImGui::Slider("Look sensitivity", m_look_sensitivity, 0.01f, 10.f);
		ImGui::Slider("Pan sensitivity",  m_pan_sensitivity,  0.01f, 10.f);
		ImGui::Slider("Zoom sensitivity", m_zoom_sensitivity, 0.01f, 10.f);
		ImGui::Slider("Orbit radius",     m_orbit_radius,     0.1f,  100.f);

		// For displaying in UI we convert the angles to degrees and back again after.
		auto yaw_deg   = glm::degrees(m_yaw);
		auto pitch_deg = glm::degrees(m_pitch);
		ImGui::Slider("Yaw",   yaw_deg,   -360.f, 360.f, "%.3f°");
		ImGui::Slider("Pitch", pitch_deg, -360.f, 360.f, "%.3f°");
		m_yaw   = glm::radians(yaw_deg);
		m_pitch = glm::radians(pitch_deg);

		ImGui::SeparatorText("Info");
		ImGui::Text("Position", position());
		ImGui::Text("Right",    right());
		ImGui::Text("Up",       up());
		ImGui::Text("Forward",  forward());

		ImGui::Separator();
		ImGui::Text("View", view());

		ImGui::End();
	}
}