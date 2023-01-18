#include "PhysicsSystem.hpp"

// System
#include "SceneSystem.hpp"
#include "CollisionSystem.hpp"

// Component
#include "Collider.hpp"
#include "RigidBody.hpp"
#include "Storage.hpp"
#include "Transform.hpp"

#include "Utility.hpp"

namespace System
{
    PhysicsSystem::PhysicsSystem(SceneSystem& pSceneSystem, CollisionSystem& pCollisionSystem)
        : mUpdateCount{0}
        , mSceneSystem{pSceneSystem}
        , mCollisionSystem{pCollisionSystem}
        , mTotalSimulationTime{DeltaTime::zero()}
        , mGravity{glm::vec3(0.f, -9.81f, 0.f)}
    {}

    void PhysicsSystem::integrate(const DeltaTime& pDeltaTime)
    {
        mUpdateCount++;
        mTotalSimulationTime += pDeltaTime;

        auto& scene = mSceneSystem.getCurrentScene();
        scene.foreach([this, &pDeltaTime, &scene](ECS::EntityID& pEntity, Component::RigidBody& pRigidBody, Component::Transform& pTransform)
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
                pRigidBody.mAngularVelocity = pRigidBody.mAngularMomentum / pRigidBody.mInertiaTensor; // ω = L / I

                // To integrate the new quat orientation we convert the angular velocity into quaternion form - spin.
                // Spin represents a time derivative of orientation. https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
                const glm::quat spin = 0.5f * glm::quat(0.f, (pRigidBody.mAngularVelocity * pDeltaTime.count())) * pTransform.mOrientation;

                // Integrate spin to find the new orientation
                pTransform.mOrientation += spin;
                pTransform.mOrientation = glm::normalize(pTransform.mOrientation);
                // Recalculate the direction and rotation mat
                pTransform.mDirection = glm::normalize(pTransform.mOrientation * Component::Transform::StartingDirection);
                pTransform.mRollPitchYaw = glm::degrees(Utility::toRollPitchYaw(pTransform.mOrientation));
            }

            const auto rotationMatrix = glm::mat4_cast(pTransform.mOrientation);

            pTransform.mModel = glm::translate(glm::identity<glm::mat4>(), pTransform.mPosition);
            pTransform.mModel *= rotationMatrix;
            pTransform.mModel = glm::scale(pTransform.mModel, pTransform.mScale);

            // Update the collider AABB to new world space position
            if (scene.hasComponents<Component::Collider>(pEntity))
            {
                auto& collider = scene.getComponentMutable<Component::Collider>(pEntity);
                collider.mWorldAABB = Geometry::AABB::transform(collider.mObjectAABB, pTransform.mPosition, rotationMatrix, pTransform.mScale);
                collider.mCollided = false;
                // After moving, check for collisions and respond
                auto collision = mCollisionSystem.getCollision(pEntity, pTransform, collider);
                if (collision)
                {
                    collider.mCollided = true;
                    LOG_INFO("Collision occurred for entity {}", pEntity);
                }
            }
        });
    }
} // namespace System