#include "TwoAxisCamera.hpp"
#include "OpenGL/DebugRenderer.hpp"
#include "Geometry/Sphere.hpp"
#include "Platform/Input.hpp"
#include "Utility/Logger.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"

#include <cmath>

namespace Component
{
	TwoAxisCamera::TwoAxisCamera()
		: m_FOV{90.f}
		, m_near{0.04f}
		, m_far{10000000.f} // Far enough to cover the entire extents of the GridRenderer grid diagonally at all angles. (1000 half size units).
		, m_look_sensitivity{0.5f}
		, m_zoom_sensitivity{0.5f}
		, m_pan_sensitivity{0.005f}
		, m_pitch{0.f}
		, m_yaw{0.f}
		, m_orbit_radius{10.f}
		, m_orbit_center{0.f, 0.f, 0.f}
		, m_dolly_threshold{0.5f}
		, m_is_orthographic{false}
		, m_ortho_size{10.f}
		, m_ortho_distance_multipler{90.f} // Could use scene bounds size to determine this value.
		, m_refit_AABB{}
		, m_refit_sphere_radius{0.f}
		, m_show_refit_AABB{false}
		, m_show_refit_sphere{false}
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
		// In orthographic, the camera has no concept of position. We scale the distance by the ortho distance
		// multiplier to prevent the camera from getting too close to the orbit center keeping the scene in view.
		if (m_is_orthographic)
			return m_orbit_center - m_ortho_size * forward() * m_ortho_distance_multipler;
		else
			return m_orbit_center - m_orbit_radius * forward();
	}
	void TwoAxisCamera::set_view_direction(const glm::vec3& p_view_direction)
	{
		m_pitch = glm::asin(p_view_direction.y);
		m_yaw   = glm::atan(p_view_direction.z, p_view_direction.x);
	}
	void TwoAxisCamera::refit(const Geometry::AABB& p_AABB, float p_aspect_ratio, float p_padding)
	{
		Geometry::AABB bounds = p_AABB;

		m_orbit_center = bounds.get_center();

		// Compute the radius of the bounding sphere circumscribing the AABB.
		// This guarantees the AABB is visible from any view angle.
		float radius = glm::length(bounds.get_size()) * 0.5f;

		// Cache for debug visualisation.
		m_refit_AABB          = bounds;
		m_refit_sphere_radius = radius;

		if (m_is_orthographic)
		{
			// In orthographic mode we need to set the ortho size so the AABB fits in the viewport.
			// Account for the aspect ratio so the tighter axis determines the size.
			float half_height          = radius;
			float half_width_as_height = radius / p_aspect_ratio;
			m_ortho_size = glm::max(half_height, half_width_as_height) * p_padding;
		}
		else
		{
			// In perspective mode compute the orbit distance so the bounding sphere fits inside the frustum.
			float half_fov_v = glm::radians(m_FOV) * 0.5f;
			float half_fov_h = glm::atan(glm::tan(half_fov_v) * p_aspect_ratio);
			// Use the tighter FOV to ensure the sphere fits both horizontally and vertically.
			float half_fov   = glm::min(half_fov_v, half_fov_h);
			m_orbit_radius   = (radius / glm::sin(half_fov)) * p_padding;
		}
	}
	glm::mat4 TwoAxisCamera::view() const
	{
		return glm::lookAt(position(), m_orbit_center, up());
	}
	ViewInformation TwoAxisCamera::view_information(const float& p_aspect_ratio) const
	{
		ViewInformation view_info;
		view_info.m_view_position = {position(), 1.f};
		view_info.m_view          = view();

		if (m_is_orthographic)
			view_info.m_projection = glm::ortho(-m_ortho_size * p_aspect_ratio, m_ortho_size * p_aspect_ratio, -m_ortho_size, m_ortho_size, m_near, m_far);
		else
			view_info.m_projection = glm::perspective(glm::radians(m_FOV), p_aspect_ratio, m_near, m_far);

		return view_info;
	}

	void TwoAxisCamera::set_orthographic(bool p_orthographic)
	{
		// If the camera is already in the desired mode, return early. Otherwise, adjust the perspective
		// parameters to match the apparent distance/size of the scene.
		if (m_is_orthographic != p_orthographic)
		{
			m_is_orthographic = p_orthographic;

			if (m_is_orthographic)
				m_ortho_size = m_orbit_radius * glm::tan(glm::radians(m_FOV) * 0.5f);
			else
				m_orbit_radius = m_ortho_size / glm::tan(glm::radians(m_FOV) * 0.5f);
		}
	}

	// Don't let the camera flip upside down, cap the pitch angle at straight above and below.
	constexpr float pitch_constaint_deg  = 90.f;
	constexpr float pitch_constraint_rad = glm::radians(pitch_constaint_deg);

	void TwoAxisCamera::mouse_look(const glm::vec2& p_offset)
	{
		m_yaw   -= glm::radians(p_offset.x * m_look_sensitivity);
		m_pitch += glm::radians(p_offset.y * m_look_sensitivity);

		// The constraint is applied to the pitch angle to prevent the camera from flipping upside down
		if      (m_pitch >  pitch_constraint_rad)  m_pitch =  pitch_constraint_rad;
		else if (m_pitch < -pitch_constraint_rad)  m_pitch = -pitch_constraint_rad;
	}
	void TwoAxisCamera::pan(const glm::vec2& p_offset)
	{
		float distance = glm::distance(m_orbit_center, position());
		float pan_factor = distance * m_pan_sensitivity;

		glm::vec3 pan_offset = -p_offset.x * right() - p_offset.y * up();
		m_orbit_center += pan_offset * pan_factor;
	}
	void TwoAxisCamera::zoom(float p_offset)
	{
		// Exponential zoom: each scroll tick scales the distance by a constant ratio,
		// giving a consistent feel at any distance (near or far).
		float zoom_base = 1.f + m_zoom_sensitivity * 0.15f;
		float scale     = std::pow(zoom_base, -p_offset);

		if (m_is_orthographic)
		{
			m_ortho_size *= scale;
			m_ortho_size = glm::max(m_ortho_size, 0.001f);
		}
		else
		{
			float new_radius = m_orbit_radius * scale;

			// Dolly-through: when the radius drops below the threshold, push the orbit
			// center forward instead of letting the camera stall at the focus point.
			if (new_radius < m_dolly_threshold)
			{
				float excess = m_dolly_threshold - new_radius;
				m_orbit_center += forward() * excess;
				new_radius = m_dolly_threshold;
			}

			m_orbit_radius = new_radius;
		}
	}

	void TwoAxisCamera::draw_UI()
	{
		bool ortho = m_is_orthographic;
		if (ImGui::Checkbox("Orthographic", &ortho))
			set_orthographic(ortho);

		if (m_is_orthographic)
		{
			ImGui::Slider("Ortho size", m_ortho_size, 0.1f, 100.f);
			ImGui::Slider("Ortho distance multiplier", m_ortho_distance_multipler, 1.f, 100.f);
		}
		else
		{
			ImGui::Slider("FOV", m_FOV, 1.f, 90.f, "%.3f°");
			ImGui::Slider("Orbit radius", m_orbit_radius, 0.1f, 100.f);
			ImGui::Slider("Dolly threshold", m_dolly_threshold, 0.01f, 5.f);
		}

		ImGui::Slider("Near", m_near, 0.001f, 10.f);
		ImGui::Slider("Far",  m_far,  1.f,    10000000.f);
		ImGui::Slider("Orbit center", m_orbit_center, -100.f, 100.f);

		// For displaying in UI we convert the angles to degrees and back again after.
		auto yaw_deg   = glm::degrees(m_yaw);
		auto pitch_deg = glm::degrees(m_pitch);
		ImGui::Slider("Yaw",   yaw_deg,   -360.f, 360.f, "%.3f°");
		ImGui::Slider("Pitch", pitch_deg, -pitch_constaint_deg, pitch_constaint_deg, "%.3f°");
		m_yaw   = glm::radians(yaw_deg);
		m_pitch = glm::radians(pitch_deg);

		ImGui::SeparatorText("Controls");
		ImGui::Slider("Look sensitivity", m_look_sensitivity, 0.01f, 10.f);
		ImGui::Slider("Pan sensitivity",  m_pan_sensitivity,  0.01f, 10.f);
		ImGui::Slider("Zoom sensitivity", m_zoom_sensitivity, 0.01f, 10.f);

		ImGui::SeparatorText("Refit debug");
		ImGui::Checkbox("Show refit AABB",   &m_show_refit_AABB);
		ImGui::Checkbox("Show refit sphere", &m_show_refit_sphere);

		if (m_show_refit_AABB)
			OpenGL::DebugRenderer::add(m_refit_AABB, glm::vec4(1.f, 0.5f, 0.f, 1.f));
		if (m_show_refit_sphere)
			OpenGL::DebugRenderer::add(Geometry::Sphere{m_refit_AABB.get_center(), m_refit_sphere_radius}, glm::vec4(0.f, 0.5f, 1.f, 0.25f));

		ImGui::SeparatorText("Info");
		ImGui::Text("Position", position());
		ImGui::Text("Right",    right());
		ImGui::Text("Up",       up());
		ImGui::Text("Forward",  forward());

		ImGui::Separator();
		ImGui::Text("View", view());
	}
}