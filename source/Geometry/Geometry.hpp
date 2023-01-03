#pragma once

#include "glm/vec3.hpp"
#include "glm/glm.hpp"

#include <cmath>

// Variable          Symbol     SI Unit
// -------------------------------------
// Force                F     (N  = kg m/s²)
// Impulse              J     (N s = kg m/s)
// Momentum             p     (N s = kg m/s)
// Acceleration         a        (m/s²)
// Velocity             v         (m/s)
// Mass                 m          (kg)
// Torque               T         (N m)
// Angular Momentum     L       (kg m²/s)
// Angular Velocity     ω        (rad/s)
// Inertia              I        (kg m²)

// All the Geometry functions expect all params and return all values in SI units.
namespace Geometry
{
    // Returns the moment of inertia (I) for a solid cuboid in SI units (kg m²) about central axis along height.
    static glm::vec3 CuboidInertia(float pMass, float pHeight, float pWidth, float pDepth)
    {
        const float InertiaX = (1.f/12.f) * pMass * (std::pow(pDepth, 2.f) + std::pow(pHeight, 2.f));
        const float InertiaY = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pDepth, 2.f));
        const float InertiaZ = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pHeight, 2.f));
        return glm::vec3(InertiaX, InertiaY, InertiaZ);
    }

    static glm::mat3 cylinderInertiaTensor(const float& pMass, const float& pRadius, const float& pHeight)
    {
        // https://en.wikipedia.org/wiki/List_of_moments_of_inertia
        // Cylinder oriented with height along z-axis
        const float x = (1.f / 12.f) * pMass * ((3.f * std::pow(pRadius, 2.f)) + std::pow(pHeight, 2.f));
        const float y = ((1.f / 2.f) * pMass * std::pow(pRadius, 2.f));

        return glm::mat3(  x , 0.f, 0.f
                        , 0.f,  x , 0.f
                        , 0.f, 0.f,  y );
    }
    static glm::mat3 cuboidInertiaTensor(const float& pMass, const float& pWidth, const float& pHeight, const float& pDepth)
    {
        // https://en.wikipedia.org/wiki/List_of_moments_of_inertia

        const float x = (1.f / 12.f) * pMass;
        return glm::mat3( x * (std::pow(pHeight, 2.f) + std::pow(pDepth, 2.f)), 0.f, 0.f
                        , 0.f, x * (std::pow(pWidth, 2.f) + std::pow(pHeight, 2.f)), 0.f
                        , 0.f, 0.f,  x * (std::pow(pWidth, 2.f) + std::pow(pDepth, 2.f)) );
    }

    // Returns the magnitude of the impulse after a collision between bodies 1 and 2.
    // Impulse is a vector quantity, which can be derived by multiplying this magnitude with pCollisionNormal for each body.
    // pRestitution ranges from 0 to 1 where 1 is a perfectly elastic collision and 0 is perfectly inelastic.
    static float linearImpulseMagnitude(const float& pMass1, const glm::vec3& pVelocity1, const float& pMass2, const glm::vec3& pVelocity2, const glm::vec3& pCollisionNormal, const float& pRestitution)
    {
        // Adapted from: 3D Math Primer for Graphics and Games Development - 12.4.2 General collision response - pg 598
        const auto relativeVelocity = pVelocity1 - pVelocity2;
        return ((pRestitution + 1) * glm::dot(relativeVelocity, pCollisionNormal)) / ((1 / pMass1) + (1 / pMass2) * glm::dot(pCollisionNormal, pCollisionNormal));
    }
}