#pragma once

#include "glm/vec3.hpp"

// Variable          Symbol     SI Unit
// -------------------------------------
//Force                F          (N)
//Momentum             p        (kg m/s)
//Acceleration         a        (m/s²)
//Velocity             v         (m/s)
//Mass                 m          (kg)
//Torque               T         (N m)
//Angular Momentum     L       (kg m²/s)
//Angular Velocity     ω        (rad/s)
//Inertia              I        (kg m²)

// All the Geometry functions expect all params and return all values in SI units.
namespace Geometry
{
    // Get the moment of inertia (I) for a solid cuboid in SI units (kg m²) about central axis along height.
    static glm::vec3 CuboidInertia(float pMass, float pHeight, float pWidth, float pDepth)
    {
        const float InertiaX = (1.f/12.f) * pMass * (std::pow(pDepth, 2.f) + std::pow(pHeight, 2.f));
        const float InertiaY = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pDepth, 2.f));
        const float InertiaZ = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pHeight, 2.f));
        return glm::vec3(InertiaX, InertiaY, InertiaZ);
    }
}