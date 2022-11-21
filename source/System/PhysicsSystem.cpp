#include "PhysicsSystem.hpp"

#include "Storage.hpp"
#include "RigidBody.hpp"
#include "Transform.hpp"

namespace System
{
    PhysicsSystem::PhysicsSystem(ECS::Storage& pStorage)
        : mUpdateCount{0}
        , mStorage{pStorage}
        , mTotalSimulationTime{DeltaTime::zero()}
        , mGravity{glm::vec3(0.f, -9.81f, 0.f)}
    {}

    void PhysicsSystem::integrate(const DeltaTime& pDeltaTime)
    {
        mUpdateCount++;
        mTotalSimulationTime += pDeltaTime;

        mStorage.foreach([this, &pDeltaTime]( Component::RigidBody& pRigidBody, Component::Transform& pTransform)
        {
            // F = ma
            if (pRigidBody.mApplyGravity)
                pRigidBody.mForce = pRigidBody.mMass * mGravity;

            // Change in momentum is equal to the force = dp/dt = F
            const auto changeInMomentum = pRigidBody.mForce * pDeltaTime.count(); // dp = F dt
            pRigidBody.mMomentum += changeInMomentum;

            // Convert momentum to velocity by dividing by mass: p = mv
            pRigidBody.mVelocity = pRigidBody.mMomentum / pRigidBody.mMass; // v = p/v

            // Integrate velocity to find new position: dx/dt = v
            const auto changeInPosition = pRigidBody.mVelocity * pDeltaTime.count(); // dx = v dt
            pTransform.mPosition += changeInPosition;
        });
    }
} // namespace System