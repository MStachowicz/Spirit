#pragma once

#include "glm/vec3.hpp"

#include <chrono>

namespace System
{
    class SceneSystem;
    class CollisionSystem;

    // A numerical integrator, PhysicsSystem take Transform and RigidBody components and applies kinematic equations.
    // The system is force based and numerically integrates
    class PhysicsSystem
    {
    public:
        using DeltaTime = std::chrono::duration<float, std::ratio<1,1>>; // Represents a float precision duration in seconds.

        PhysicsSystem(SceneSystem& pSceneSystem, CollisionSystem& pCollisionSystem);
        void integrate(const DeltaTime& pDeltaTime);

        size_t mUpdateCount;
    private:
        SceneSystem& mSceneSystem;
        CollisionSystem& mCollisionSystem;

        DeltaTime mTotalSimulationTime; // Total time simulated using the integrate function.
        glm::vec3 mGravity;             // The acceleration due to gravity.
    };
} // namespace System