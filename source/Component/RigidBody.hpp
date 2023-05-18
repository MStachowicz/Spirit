#pragma once

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"

namespace Component
{
    // An idealised body that exhibits 0 deformation. All units are in SI.
    struct RigidBody
    {
        // Linear motion
        // -----------------------------------------------------------------------------
        glm::vec3 mForce;        // Linear force F in Newtons (kg m/s²), This force is applied on a PhysicsSystem tick and gets reset to 0.
        glm::vec3 mMomentum;     // Linear momentum p in Newton seconds (kg m/s)
        glm::vec3 mAcceleration; // Linear acceleration a (m/s²)
        glm::vec3 mVelocity;     // Linear velocity v (m/s)

        // Angular motion
        // -----------------------------------------------------------------------------
        glm::vec3 mTorque;          // Angular force T in Newton meters producing a change in rotational motion (kg m²/s²)
        glm::vec3 mAngularMomentum; // Angular momentum L in Newton meter seconds, a conserved quantity if no external torque is applied (kg m²/s)
        glm::vec3 mAngularVelocity; // Angular velocity ω representing how quickly (Hz) this body revolves relative to it's axis (/s)
        glm::mat3 mInertiaTensor;   // Moment of inertia tensor J, a symmetric matrix determining the torque needed for a desired angular acceleration about a rotational axis (kg m2)

        float mMass; // Inertial mass measuring the body's resistance to acceleration when a force is applied (kg)
        bool mApplyGravity;
        // Position and orientation are stored in Component::Transform.

        RigidBody();
        // Apply a linear p_force (kg m/s²) on the body. Force is applied on a PhysicsSystem::update tick.
        void apply_linear_force(const glm::vec3& p_force);
        void DrawImGui();
    };
}