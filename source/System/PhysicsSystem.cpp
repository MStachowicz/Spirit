#include "PhysicsSystem.hpp"

// System
#include "SceneSystem.hpp"

// Component
#include "Storage.hpp"
#include "RigidBody.hpp"
#include "Transform.hpp"

#include "Utility.hpp"

namespace System
{
    PhysicsSystem::PhysicsSystem(SceneSystem& pSceneSystem)
        : mUpdateCount{0}
        , mSceneSystem{pSceneSystem}
        , mTotalSimulationTime{DeltaTime::zero()}
        , mGravity{glm::vec3(0.f, -9.81f, 0.f)}
    {}

    void PhysicsSystem::integrate(const DeltaTime& pDeltaTime)
    {
        mUpdateCount++;
        mTotalSimulationTime += pDeltaTime;

        mSceneSystem.getCurrentScene().foreach([this, &pDeltaTime](Component::RigidBody& pRigidBody, Component::Transform& pTransform)
        {
            // F = ma
            if (pRigidBody.mApplyGravity)
                pRigidBody.mForce = pRigidBody.mMass * mGravity;

            { // Linear motion
                // Change in momentum is equal to the force = dp/dt = F
                const auto changeInMomentum = pRigidBody.mForce * pDeltaTime.count(); // dp = F dt
                pRigidBody.mMomentum += changeInMomentum;

                // Convert momentum to velocity by dividing by mass: p = mv
                pRigidBody.mVelocity = pRigidBody.mMomentum / pRigidBody.mMass; // v = p/v

                // Integrate velocity to find new position: dx/dt = v
                const auto changeInPosition = pRigidBody.mVelocity * pDeltaTime.count(); // dx = v dt
                pTransform.mPosition += changeInPosition;
            }

            { // Angular motion
                // http://physics.bu.edu/~redner/211-sp06/class-rigid-body/angularmo.html
                const auto changeInAngularMomentum = pRigidBody.mTorque * pDeltaTime.count(); // dL = T dt
                pRigidBody.mAngularMomentum += changeInAngularMomentum;

                // Convert angular momentum to angular velocity by dividing by inertia tensor: L = Iω
                pRigidBody.mAngularVelocity = pRigidBody.mAngularMomentum / pRigidBody.mInertia; // ω = L / I
                //pRigidBody.mAngularVelocity = glm::vec3(0.f, glm::radians(360.f), 0.f) * pDeltaTime.count();

                // To integrate the new quat orientation we convert the angular velocity into quaternion form - spin.
                // Spin represents a time derivative of orientation. https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
                const glm::quat spin =
                    0.5f * glm::quat(0.f, pRigidBody.mAngularVelocity.x, pRigidBody.mAngularVelocity.y, pRigidBody.mAngularVelocity.z) * pTransform.mOrientation;

                // Integrate spin to find the new orientation
                pTransform.mOrientation += spin;
                pTransform.mOrientation = glm::normalize(pTransform.mOrientation);
                // Recalculate the direction and rotation mat
                pTransform.mDirection = glm::normalize(pTransform.mOrientation * Component::Transform::StartingDirection);
                pTransform.mRollPitchYaw = glm::degrees(Utility::toRollPitchYaw(pTransform.mOrientation));
            }

            pTransform.mModel = glm::translate(glm::identity<glm::mat4>(), pTransform.mPosition);
            pTransform.mModel *= glm::mat4_cast(pTransform.mOrientation);
            pTransform.mModel = glm::scale(pTransform.mModel, pTransform.mScale);
        });
    }
} // namespace System