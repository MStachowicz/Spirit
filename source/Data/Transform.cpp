#include "Transform.hpp"

#include "imgui.h"

namespace Data
{
    void Transform::DrawImGui()
    {
		ImGui::SliderFloat3("Position", &mPosition.x, -50.f, 50.f);
		ImGui::SliderFloat3("Rotation", &mRotation.x, -360.f, 360.f);
		ImGui::SliderFloat3("Scale", &mScale.x, 0.1f, 10.f);
    }
}