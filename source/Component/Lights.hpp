#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace Geometry
{
	class AABB;
}
namespace Component
{
	class DirectionalLight
	{
	public:
		static inline float PCF_bias = -0.001f;

		DirectionalLight() noexcept;
		DirectionalLight(const glm::vec3& p_direction, float p_ambient_intensity, float m_diffuse_intensity) noexcept;

		glm::vec3 mDirection;
		glm::vec3 mColour;
		float mAmbientIntensity;
		float mDiffuseIntensity;
		float mSpecularIntensity;

		float m_shadow_near_plane;
		float m_shadow_far_plane;
		float m_ortho_size;

		glm::mat4 get_view_proj(const Geometry::AABB& scene_AABB);

		void draw_UI();
	};

	class PointLight
	{
	public:
		PointLight() noexcept;
		PointLight(const glm::vec3& p_position) noexcept;

		glm::vec3 mPosition;
		glm::vec3 mColour;

		float mAmbientIntensity;
		float mDiffuseIntensity;
		float mSpecularIntensity;

		float mConstant;
		float mLinear;
		float mQuadratic;

		void draw_UI();
	};

	class SpotLight
	{
	public:
		SpotLight() noexcept;

		glm::vec3 mPosition;
		glm::vec3 mDirection;
		glm::vec3 mColour;
		float mAmbientIntensity;
		float mDiffuseIntensity;
		float mSpecularIntensity;

		float mConstant;
		float mLinear;
		float mQuadratic;

		float mCutOff;
		float mOuterCutOff;

		void draw_UI();
	};
}