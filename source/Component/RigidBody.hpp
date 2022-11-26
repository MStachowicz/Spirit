#pragma once

#include "glm/vec3.hpp"

namespace Component
{
    struct RigidBody
    {
        // Linear motion
        // -----------------------------------------------------------------------------
        glm::vec3 mForce        = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mMomentum     = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mAcceleration = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mVelocity     = glm::vec3(0.f, 0.f, 0.f);
        // mPoisition is contained by Component::Transform.
        float mMass = 1;
        bool mApplyGravity = false;
        // -----------------------------------------------------------------------------
        // Angular motion
        // -----------------------------------------------------------------------------
        glm::vec3 mTorque              = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mAngularMomentum     = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mAngularVelocity     = glm::vec3(0.f, 0.f, 0.f);
        // mOrientation stored in Component::Transform
        float mInertia = 1;
        // -----------------------------------------------------------------------------

        void DrawImGui();
    };
}