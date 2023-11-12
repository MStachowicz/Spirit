#include "Lights.hpp"
#include "Component/Transform.hpp"
#include "Geometry/AABB.hpp"

#include "glm/trigonometric.hpp"
#include "imgui.h"

namespace Component
{
	DirectionalLight::DirectionalLight() noexcept
		: mDirection{glm::vec3(0.f, -1.f, 0.f)}
		, mColour{glm::vec3(1.f, 1.f, 1.f)}
		, mAmbientIntensity{0.05f}
		, mDiffuseIntensity{0.15f}
		, mSpecularIntensity{0.5f}
	{}
	DirectionalLight::DirectionalLight(const glm::vec3& p_direction, float p_ambient_intensity, float m_diffuse_intensity) noexcept
		: mDirection{p_direction}
		, mColour{glm::vec3(1.f, 1.f, 1.f)}
		, mAmbientIntensity{p_ambient_intensity}
		, mDiffuseIntensity{m_diffuse_intensity}
		, mSpecularIntensity{0.5f}
		, m_ortho_size{10.f}
		, m_shadow_near_plane{0.001f}
		, m_shadow_far_plane{15.f}
	{}

	void DirectionalLight::draw_UI()
	{
		if(ImGui::TreeNode("Directional light"))
		{
			if (ImGui::SliderFloat3("Direction", &mDirection.x, -1.f, 1.f))
				glm::normalize(mDirection);

			ImGui::ColorEdit3("Colour", &mColour.x);
			ImGui::SliderFloat("Ambient intensity", &mAmbientIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Diffuse intensity", &mDiffuseIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Specular intensity", &mSpecularIntensity, 0.f, 1.f);

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

	glm::mat4 DirectionalLight::get_view_proj(const Geometry::AABB& p_scene_AABB)
	{
		// DirectionalLight has no position, instead consider at the extents of the scene in the opposite direction its casting.
		glm::vec3 size             = p_scene_AABB.get_size();
		glm::vec3 center           = p_scene_AABB.get_center();
		glm::vec3 start_position   = center - mDirection * (size / 2.f);
		glm::vec3 end_position     = center + mDirection * (size / 2.f);
		glm::mat4 view             = glm::lookAt(start_position, center, glm::vec3(0.0f, 0.0f, 1.0f)); // Up
		glm::mat4 projection       = glm::ortho(-m_ortho_size, m_ortho_size, -m_ortho_size, m_ortho_size, m_shadow_near_plane, m_shadow_far_plane);
		return projection * view;
	}

	PointLight::PointLight() noexcept
		: mPosition{glm::vec3(0.f, 0.f, 0.f)}
		, mColour{glm::vec3(1.f, 1.f, 1.f)}
		, mAmbientIntensity{0.05f}
		, mDiffuseIntensity{0.8f}
		, mSpecularIntensity{1.0f}
		, mConstant{1.f}
		, mLinear{0.09f}
		, mQuadratic{0.032f}
	{}
	PointLight::PointLight(const glm::vec3& p_position) noexcept
		: mPosition{p_position}
		, mColour{glm::vec3(1.f, 1.f, 1.f)}
		, mAmbientIntensity{0.05f}
		, mDiffuseIntensity{0.8f}
		, mSpecularIntensity{1.0f}
		, mConstant{1.f}
		, mLinear{0.09f}
		, mQuadratic{0.032f}
	{}

	void PointLight::draw_UI()
	{
		if(ImGui::TreeNode("Point light"))
		{
			ImGui::SliderFloat3("Position", &mPosition.x, -10.f, 10.f);
			ImGui::ColorEdit3("Colour", &mColour.x);
			ImGui::SliderFloat("Ambient intensity", &mAmbientIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Diffuse intensity", &mDiffuseIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Specular intensity", &mSpecularIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Constant", &mConstant, 0.f, 1.f);
			ImGui::SliderFloat("Linear", &mLinear, 0.f, 1.f);
			ImGui::SliderFloat("Quadratic", &mQuadratic, 0.f, 1.f);
			ImGui::TreePop();
		}
	}

	SpotLight::SpotLight() noexcept
		: mPosition{glm::vec3(0.f, 0.f, 0.f)}
		, mDirection{glm::vec3(0.f, 0.f, -1.f)}
		, mColour{glm::vec3(1.f, 1.f, 1.f)}
		, mAmbientIntensity{0.0f}
		, mDiffuseIntensity{1.0f}
		, mSpecularIntensity{1.0f}
		, mConstant{1.f}
		, mLinear{0.09f}
		, mQuadratic{0.032f}
		, mCutOff{glm::cos(glm::radians(12.5f))}
		, mOuterCutOff{glm::cos(glm::radians(15.0f))}
	{}
	void SpotLight::draw_UI()
	{
		if(ImGui::TreeNode("SpotLight"))
		{
			ImGui::SliderFloat3("Position", &mPosition.x, -1.f, 1.f);
			if (ImGui::SliderFloat3("Direction", &mDirection.x, -1.f, 1.f))
				glm::normalize(mDirection);
			ImGui::ColorEdit3("Colour", &mColour.x);
			ImGui::SliderFloat("Ambient intensity", &mAmbientIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Diffuse intensity", &mDiffuseIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Specular intensity", &mSpecularIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Constant", &mConstant, 0.f, 1.f);
			ImGui::SliderFloat("Linear", &mLinear, 0.f, 1.f);
			ImGui::SliderFloat("Quadratic", &mQuadratic, 0.f, 1.f);
			ImGui::SliderFloat("Cutoff", &mCutOff, 0.f, 1.f);
			ImGui::SliderFloat("Outer cutoff", &mOuterCutOff, 0.f, 1.f);
			ImGui::TreePop();
		}
	}
}