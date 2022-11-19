#include "Transform.hpp"

#include "imgui.h"

namespace Component
{
  void Transform::DrawImGui()
  {
    if(ImGui::TreeNode("Transform"))
    {
      ImGui::SliderFloat3("Position", &mPosition.x, -50.f, 50.f);
      ImGui::SliderFloat3("Rotation", &mRotation.x, -360.f, 360.f);
      ImGui::SliderFloat3("Scale", &mScale.x, 0.1f, 10.f);
      ImGui::TreePop();
    }
  }
}