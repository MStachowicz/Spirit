#include "Collider.hpp"

#include "imgui.h"

namespace Component
{
    void Collider::DrawImGui()
    {
        if(ImGui::TreeNode("Collider"))
        {
           ImGui::InputDouble("Low X",  &mBoundingBox.mLowX);
           ImGui::InputDouble("High X", &mBoundingBox.mHighX);
           ImGui::InputDouble("Low Y",  &mBoundingBox.mLowY);
           ImGui::InputDouble("High Y", &mBoundingBox.mHighY);
           ImGui::InputDouble("Low Z",  &mBoundingBox.mLowZ);
           ImGui::InputDouble("High Z", &mBoundingBox.mHighZ);
           ImGui::TreePop();
        }
    }
}