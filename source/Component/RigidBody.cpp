#include "RigidBody.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"

namespace Component
{
    RigidBody::RigidBody()
        : mMass{1}
        , mApplyGravity{false}
        , mForce{0.f, 0.f, 0.f}
        , mMomentum{0.f, 0.f, 0.f}
        , mAcceleration{0.f, 0.f, 0.f}
        , mVelocity{0.f, 0.f, 0.f}
        , mTorque{0.f, 0.f, 0.f}
        , mAngularMomentum{0.f, 0.f, 0.f}
        , mAngularVelocity{0.f, 0.f, 0.f}
        , mInertiaTensor{glm::identity<glm::mat3>()}
    {}

    void RigidBody::DrawImGui()
    {
        if(ImGui::TreeNode("Rigid body"))
        {
            ImGui::SliderFloat3("Force                  (N)", &mForce.x, -10.f, 10.f);
            ImGui::SliderFloat3("Momentum          (kg m/s)", &mMomentum.x, -10.f, 10.f);
            ImGui::SliderFloat3("Acceleration        (m/s²)", &mAcceleration.x, -10.f, 10.f);
            ImGui::SliderFloat3("Velocity             (m/s)", &mVelocity.x, -10.f, 10.f);
            ImGui::SliderFloat( "Mass                  (kg)", &mMass, 0.001f, 100.f);

            ImGui::Separator();
            ImGui::SliderFloat3("Torque               (N m)", &mTorque.x, -10.f, 10.f);
            ImGui::SliderFloat3("Angular Momentum (kg m²/s)", &mAngularMomentum.x, -10.f, 10.f);
            ImGui::SliderFloat3("Angular Velocity   (rad/s)", &mAngularVelocity.x, -10.f, 10.f);

            const float inertiaLimit = mMass * 100.f;
            ImGui::SliderFloat3("Angular Tensor 1   (kg m²)", &mInertiaTensor[0][0], -inertiaLimit, inertiaLimit);
            ImGui::SliderFloat3("Angular Tensor 2   (kg m²)", &mInertiaTensor[1][0], -inertiaLimit, inertiaLimit);
            ImGui::SliderFloat3("Angular Tensor 3   (kg m²)", &mInertiaTensor[2][0], -inertiaLimit, inertiaLimit);

            ImGui::Separator();
            ImGui::Checkbox("Apply Gravity", &mApplyGravity);
            ImGui::TreePop();

            if (ImGui::Button("Reset"))
            {
                //mMass            = {1};
                mApplyGravity    = {false};
                mForce           = {0.f, 0.f, 0.f};
                mMomentum        = {0.f, 0.f, 0.f};
                mAcceleration    = {0.f, 0.f, 0.f};
                mVelocity        = {0.f, 0.f, 0.f};
                mTorque          = {0.f, 0.f, 0.f};
                mAngularMomentum = {0.f, 0.f, 0.f};
                mAngularVelocity = {0.f, 0.f, 0.f};
                //mInertiaTensor   = {glm::identity<glm::mat3>()};
            }
        }
    }
}