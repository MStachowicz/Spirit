#include "RigidBody.hpp"

#include "imgui.h"

namespace Component
{
    void RigidBody::DrawImGui()
    {
        if(ImGui::TreeNode("Rigid body"))
        {
            ImGui::SliderFloat3("Force                  (N)", &mForce.x, -10.f, 10.f);
            ImGui::SliderFloat3("Momentum          (kg m/s)", &mMomentum.x, -10.f, 10.f);
            ImGui::SliderFloat3("Acceleration        (m/s²)", &mAcceleration.x, -10.f, 10.f);
            ImGui::SliderFloat3("Velocity             (m/s)", &mVelocity.x, -10.f, 10.f);
            ImGui::SliderFloat( "Mass                  (kg)", &mMass, -100, 100);

            ImGui::Separator();
            ImGui::SliderFloat3("Torque                (N m)", &mTorque.x, -10.f, 10.f);
            ImGui::SliderFloat3("Angular Momentum  (kg m²/s)", &mAngularMomentum.x, -10.f, 10.f);
            ImGui::SliderFloat3("Angular Velocity    (rad/s)", &mAngularVelocity.x, -10.f, 10.f);
            ImGui::SliderFloat( "Inertia             (kg m²)", &mInertia, 0.f, 100.f);

            ImGui::Separator();
            ImGui::Checkbox("Apply Gravity", &mApplyGravity);
            ImGui::TreePop();
        }
    }
}