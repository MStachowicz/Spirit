#pragma once

#include "glm/vec3.hpp"

#include <chrono>

namespace ECS
{
    class Storage;
}
namespace Component
{
    struct RigidBody;
    struct Transform;
}

namespace System
{
    class SceneSystem;

    // A numerical integrator, PhysicsSystem take Transform and RigidBody components and applies kinematic equations.
    // The system is force based and numerically integrates
    class PhysicsSystem
    {
    public:
        using DeltaTime = std::chrono::duration<float, std::ratio<1,1>>; // Represents a float precision duration in seconds.

        PhysicsSystem(SceneSystem& pSceneSystem);
        void integrate(const DeltaTime& pDeltaTime);

        size_t mUpdateCount;
    private:
        SceneSystem& mSceneSystem;
        DeltaTime mTotalSimulationTime; // Total time simulated using the integrate function.
        glm::vec3 mGravity; // The acceleration due to gravity.
    };
} // namespace System