#include "Lights.hpp"
#include "Component/Transform.hpp"
#include "Geometry/AABB.hpp"
#include "Utility/Serialise.hpp"

#include "glm/trigonometric.hpp"
#include "imgui.h"

namespace Component
{
	DirectionalLight::DirectionalLight() noexcept
		: m_direction{glm::normalize(glm::vec3(-1.f, -1.f, 0.f))}
		, m_colour{glm::vec3(1.f, 1.f, 1.f)}
		, m_ambient_intensity{0.05f}
		, m_diffuse_intensity{0.15f}
		, m_specular_intensity{0.5f}
		, m_shadow_near_plane{0.001f}
		, m_shadow_far_plane{15.f}
		, m_ortho_size{10.f}
	{}
	DirectionalLight::DirectionalLight(const glm::vec3& p_direction, float p_ambient_intensity, float m_diffuse_intensity) noexcept
		: m_direction{p_direction}
		, m_colour{glm::vec3(1.f, 1.f, 1.f)}
		, m_ambient_intensity{p_ambient_intensity}
		, m_diffuse_intensity{m_diffuse_intensity}
		, m_specular_intensity{0.5f}
		, m_shadow_near_plane{0.001f}
		, m_shadow_far_plane{15.f}
		, m_ortho_size{10.f}
	{}

	void DirectionalLight::draw_UI()
	{
		if(ImGui::TreeNode("Directional light"))
		{
			if (ImGui::SliderFloat3("Direction", &m_direction.x, -1.f, 1.f) && glm::dot(m_direction, m_direction) > 0.f)
				m_direction = glm::normalize(m_direction);

			ImGui::ColorEdit3("Colour", &m_colour.x);
			ImGui::SliderFloat("Ambient intensity", &m_ambient_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Diffuse intensity", &m_diffuse_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Specular intensity", &m_specular_intensity, 0.f, 1.f);

			{
				ImGui::SeparatorText("Shadow");
				ImGui::Slider("Ortho size", m_ortho_size, 1.f, 50.f);
				ImGui::Slider("PCF bias", PCF_bias, -1.f, 1.f);
				ImGui::Slider("Near plane", m_shadow_near_plane, 0.1f, 10.f);
				ImGui::Slider("Far plane", m_shadow_far_plane, 10.1f, 150.f);
				ImGui::TreePop();
			}
		}
	}

	void DirectionalLight::serialise(Utility::BinaryWriter& p_out, const DirectionalLight& p_light)
	{
		p_out.write(p_light.m_direction);
		p_out.write(p_light.m_colour);
		p_out.write(p_light.m_ambient_intensity);
		p_out.write(p_light.m_diffuse_intensity);
		p_out.write(p_light.m_specular_intensity);
		p_out.write(p_light.m_shadow_near_plane);
		p_out.write(p_light.m_shadow_far_plane);
		p_out.write(p_light.m_ortho_size);
	}

	DirectionalLight DirectionalLight::deserialise(Utility::BinaryReader& p_in)
	{
		DirectionalLight light;
		p_in.read(light.m_direction);
		p_in.read(light.m_colour);
		p_in.read(light.m_ambient_intensity);
		p_in.read(light.m_diffuse_intensity);
		p_in.read(light.m_specular_intensity);
		p_in.read(light.m_shadow_near_plane);
		p_in.read(light.m_shadow_far_plane);
		p_in.read(light.m_ortho_size);
		return light;
	}
	static_assert(Utility::Is_Serializable_v<DirectionalLight>, "DirectionalLight is not serializable, check that the required functions are implemented.");

	glm::mat4 DirectionalLight::get_view_proj(const Geometry::AABB& p_scene_AABB)
	{
		// DirectionalLight has no position, instead consider at the extents of the scene in the opposite direction its casting.
		glm::vec3 size             = p_scene_AABB.get_size();
		glm::vec3 center           = p_scene_AABB.get_center();
		glm::vec3 start_position   = center - m_direction * (size / 2.f);
		glm::mat4 view             = glm::lookAt(start_position, center, glm::vec3(0.0f, 0.0f, 1.0f)); // Up
		glm::mat4 projection       = glm::ortho(-m_ortho_size, m_ortho_size, -m_ortho_size, m_ortho_size, m_shadow_near_plane, m_shadow_far_plane);
		return projection * view;
	}

	PointLight::PointLight() noexcept
		: m_position{glm::vec3(0.f, 0.f, 0.f)}
		, m_colour{glm::vec3(1.f, 1.f, 1.f)}
		, m_ambient_intensity{0.05f}
		, m_diffuse_intensity{0.8f}
		, m_specular_intensity{1.0f}
		, m_constant{1.f}
		, m_linear{0.09f}
		, m_quadratic{0.032f}
	{}
	PointLight::PointLight(const glm::vec3& p_position) noexcept
		: m_position{p_position}
		, m_colour{glm::vec3(1.f, 1.f, 1.f)}
		, m_ambient_intensity{0.05f}
		, m_diffuse_intensity{0.8f}
		, m_specular_intensity{1.0f}
		, m_constant{1.f}
		, m_linear{0.09f}
		, m_quadratic{0.032f}
	{}

	void PointLight::draw_UI()
	{
		if(ImGui::TreeNode("Point light"))
		{
			ImGui::SliderFloat3("Position", &m_position.x, -10.f, 10.f);
			ImGui::ColorEdit3("Colour", &m_colour.x);
			ImGui::SliderFloat("Ambient intensity", &m_ambient_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Diffuse intensity", &m_diffuse_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Specular intensity", &m_specular_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Constant", &m_constant, 0.f, 1.f);
			ImGui::SliderFloat("Linear", &m_linear, 0.f, 1.f);
			ImGui::SliderFloat("Quadratic", &m_quadratic, 0.f, 1.f);
			ImGui::TreePop();
		}
	}

	void PointLight::serialise(Utility::BinaryWriter& p_out, const PointLight& p_light)
	{
		p_out.write(p_light.m_position);
		p_out.write(p_light.m_colour);
		p_out.write(p_light.m_ambient_intensity);
		p_out.write(p_light.m_diffuse_intensity);
		p_out.write(p_light.m_specular_intensity);
		p_out.write(p_light.m_constant);
		p_out.write(p_light.m_linear);
		p_out.write(p_light.m_quadratic);
	}
	PointLight PointLight::deserialise(Utility::BinaryReader& p_in)
	{
		PointLight light;
		p_in.read(light.m_position);
		p_in.read(light.m_colour);
		p_in.read(light.m_ambient_intensity);
		p_in.read(light.m_diffuse_intensity);
		p_in.read(light.m_specular_intensity);
		p_in.read(light.m_constant);
		p_in.read(light.m_linear);
		p_in.read(light.m_quadratic);
		return light;
	}
	static_assert(Utility::Is_Serializable_v<PointLight>, "PointLight is not serializable, check that the required functions are implemented.");

	SpotLight::SpotLight() noexcept
		: m_position{glm::vec3(0.f, 0.f, 0.f)}
		, m_direction{glm::vec3(0.f, 0.f, -1.f)}
		, m_colour{glm::vec3(1.f, 1.f, 1.f)}
		, m_ambient_intensity{0.0f}
		, m_diffuse_intensity{1.0f}
		, m_specular_intensity{1.0f}
		, m_constant{1.f}
		, m_linear{0.09f}
		, m_quadratic{0.032f}
		, m_cutoff{glm::cos(glm::radians(12.5f))}
		, m_outer_cutoff{glm::cos(glm::radians(15.0f))}
	{}
	void SpotLight::draw_UI()
	{
		if(ImGui::TreeNode("SpotLight"))
		{
			ImGui::SliderFloat3("Position", &m_position.x, -1.f, 1.f);
			if (ImGui::SliderFloat3("Direction", &m_direction.x, -1.f, 1.f) && glm::dot(m_direction, m_direction) > 0.f)
				m_direction = glm::normalize(m_direction);
			ImGui::ColorEdit3("Colour", &m_colour.x);
			ImGui::SliderFloat("Ambient intensity", &m_ambient_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Diffuse intensity", &m_diffuse_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Specular intensity", &m_specular_intensity, 0.f, 1.f);
			ImGui::SliderFloat("Constant", &m_constant, 0.f, 1.f);
			ImGui::SliderFloat("Linear", &m_linear, 0.f, 1.f);
			ImGui::SliderFloat("Quadratic", &m_quadratic, 0.f, 1.f);
			ImGui::SliderFloat("Cutoff", &m_cutoff, 0.f, 1.f);
			ImGui::SliderFloat("Outer cutoff", &m_outer_cutoff, 0.f, 1.f);
			ImGui::TreePop();
		}
	}
	void SpotLight::serialise(Utility::BinaryWriter& p_out, const SpotLight& p_light)
	{
		p_out.write(p_light.m_position);
		p_out.write(p_light.m_direction);
		p_out.write(p_light.m_colour);
		p_out.write(p_light.m_ambient_intensity);
		p_out.write(p_light.m_diffuse_intensity);
		p_out.write(p_light.m_specular_intensity);
		p_out.write(p_light.m_constant);
		p_out.write(p_light.m_linear);
		p_out.write(p_light.m_quadratic);
		p_out.write(p_light.m_cutoff);
		p_out.write(p_light.m_outer_cutoff);
	}
	SpotLight SpotLight::deserialise(Utility::BinaryReader& p_in)
	{
		SpotLight light;
		p_in.read(light.m_position);
		p_in.read(light.m_direction);
		p_in.read(light.m_colour);
		p_in.read(light.m_ambient_intensity);
		p_in.read(light.m_diffuse_intensity);
		p_in.read(light.m_specular_intensity);
		p_in.read(light.m_constant);
		p_in.read(light.m_linear);
		p_in.read(light.m_quadratic);
		p_in.read(light.m_cutoff);
		p_in.read(light.m_outer_cutoff);
		return light;
	}
	static_assert(Utility::Is_Serializable_v<SpotLight>, "SpotLight is not serializable, check that the required functions are implemented.");
} // namespace Component