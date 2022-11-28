#include "Collider.hpp"

#include "imgui.h"

namespace Component
{
    void Collider::DrawImGui()
    {
        if(ImGui::TreeNode("Collider"))
        {
            ImGui::Checkbox("Colliding", &mCollided);
            ImGui::TreePop();
        }
    }
}