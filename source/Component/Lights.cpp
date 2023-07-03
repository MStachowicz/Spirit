#include "Lights.hpp"

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
    DirectionalLight::DirectionalLight(float p_ambient_intensity, float m_diffuse_intensity) noexcept
        : mDirection{glm::vec3(0.f, -1.f, 0.f)}
        , mColour{glm::vec3(1.f, 1.f, 1.f)}
        , mAmbientIntensity{p_ambient_intensity}
        , mDiffuseIntensity{m_diffuse_intensity}
        , mSpecularIntensity{0.5f}
    {}
    void DirectionalLight::DrawImGui()
    {
        if(ImGui::TreeNode("Directional light"))
        {
            if (ImGui::SliderFloat3("Direction", &mDirection.x, -1.f, 1.f))
                glm::normalize(mDirection);

            ImGui::ColorEdit3("Colour", &mColour.x);
            ImGui::SliderFloat("Ambient intensity", &mAmbientIntensity, 0.f, 1.f);
            ImGui::SliderFloat("Diffuse intensity", &mDiffuseIntensity, 0.f, 1.f);
            ImGui::SliderFloat("Specular intensity", &mSpecularIntensity, 0.f, 1.f);
            ImGui::TreePop();
        }
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

    void PointLight::DrawImGui()
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
    void SpotLight::DrawImGui()
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