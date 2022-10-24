#include "Collider.hpp"

#include "imgui.h"

namespace Data
{
    void Collider::DrawImGui()
    {
        if(ImGui::TreeNode("Collider"))
        {
            ImGui::InputDouble("Top",    &mBoundingBox.mTop);
            ImGui::InputDouble("Bottom", &mBoundingBox.mBottom);
            ImGui::InputDouble("Left",   &mBoundingBox.mLeft);
            ImGui::InputDouble("Right",  &mBoundingBox.mRight);
            ImGui::TreePop();
        }
    }
}