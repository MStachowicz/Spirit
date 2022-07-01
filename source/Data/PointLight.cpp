#include "PointLight.hpp"

#include "imgui.h"

namespace Data
{
    void SpotLight::DrawImGui()
    {
        ImGui::SliderFloat3("Position", &mPosition.x, -10.f, 10.f);
        ImGui::ColorEdit3("Colour", &mColour.x);
        ImGui::SliderFloat("Ambient intensity", &mAmbientIntensity, 0.f, 1.f);
        ImGui::SliderFloat("Diffuse intensity", &mDiffuseIntensity, 0.f, 1.f);
        ImGui::SliderFloat("Specular intensity", &mSpecularIntensity, 0.f, 1.f);
        ImGui::SliderFloat("Constant", &mConstant, 0.f, 1.f);
        ImGui::SliderFloat("Linear", &mLinear, 0.f, 1.f);
        ImGui::SliderFloat("Quadratic", &mQuadratic, 0.f, 1.f);
    }
}