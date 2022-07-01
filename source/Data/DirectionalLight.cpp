#include "DirectionalLight.hpp"
#include "imgui.h"

namespace Data
{
    void DirectionalLight::DrawImGui()
    {
        if (ImGui::SliderFloat3("Direction", &mDirection.x, -1.f, 1.f))
            glm::normalize(mDirection);

        ImGui::ColorEdit3("Colour", &mColour.x);
        ImGui::SliderFloat("Ambient intensity", &mAmbientIntensity, 0.f, 1.f);
        ImGui::SliderFloat("Diffuse intensity", &mDiffuseIntensity, 0.f, 1.f);
        ImGui::SliderFloat("Specular intensity", &mSpecularIntensity, 0.f, 1.f);
    }
}