#include "SpotLight.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

namespace Component
{
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