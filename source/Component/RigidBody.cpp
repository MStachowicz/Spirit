#include "RigidBody.hpp"

#include "imgui.h"

namespace Component
{
    void RigidBody::DrawImGui()
    {
        if(ImGui::TreeNode("Rigid body"))
        {
            ImGui::SliderFloat3("Force              (N)", &mForce.x, -10.f, 10.f);
            ImGui::SliderFloat3("Momentum     (kgms^-1)", &mMomentum.x, -10.f, 10.f);
            ImGui::SliderFloat3("Acceleration   (ms^-2)", &mAcceleration.x, -10.f, 10.f);
            ImGui::SliderFloat3("Velocity       (ms^-1)", &mVelocity.x, -10.f, 10.f);
            ImGui::SliderFloat("Mass              (kg)", &mMass, -100, 100);
            ImGui::Checkbox("Apply Gravity", &mApplyGravity);
            ImGui::TreePop();
        }
    }
}