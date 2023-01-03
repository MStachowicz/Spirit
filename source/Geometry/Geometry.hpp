#pragma once

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"

// Variable          Symbol     SI Unit
// -------------------------------------
// Force                F     (N  = kg m/s²)
// Impulse              J     (N s = kg m/s)
// Momentum             p     (N s = kg m/s)
// Acceleration         a        (m/s²)
// Velocity             v         (m/s)
// Mass                 m          (kg)
// Torque               T    (N m = kg m²/s²)
// Angular Momentum     L    (N m s kg m²/s)
// Angular Velocity     ω        (rad/s)
// Inertia             J/I       (kg m²)

// All the Geometry functions expect all params and return all values in SI units.
namespace Geometry
{
    // Returns the moment of inertia (I) for a solid cuboid in SI units (kg m²) about central axis along height.
    glm::vec3 CuboidInertia(float pMass, float pHeight, float pWidth, float pDepth);
    glm::mat3 cylinderInertiaTensor(const float& pMass, const float& pRadius, const float& pHeight);
    glm::mat3 cuboidInertiaTensor(const float& pMass, const float& pWidth, const float& pHeight, const float& pDepth);

    // Returns the magnitude of the impulse after a collision between bodies 1 and 2.
    // Impulse is a vector quantity, which can be derived by multiplying this magnitude with pCollisionNormal for each body.
    // pRestitution ranges from 0 to 1 where 1 is a perfectly elastic collision and 0 is perfectly inelastic.
    float linearImpulseMagnitude(const float& pMass1, const glm::vec3& pVelocity1, const float& pMass2, const glm::vec3& pVelocity2, const glm::vec3& pCollisionNormal, const float& pRestitution);

    // Returns the angular impulse after a collision between bodies 1 and 2 at pCollisionPoint in pCollisionNormal direction.
    // Conevntion adopted here is pCollisionNormal points from body1 to body 2's surface. The returned impulse is applied in reverse to body 1 and directly to body 2.
    // pRestitution ranges from 0 to 1 where 1 is a perfectly elastic collision and 0 is perfectly inelastic.
    glm::vec3 angularImpulse(const glm::vec3& pCollisionPointWorldSpace, const glm::vec3& pCollisionNormal, const float& pRestitution
                                , const glm::vec3& pBody1CenterOfMassPositionWorld, const glm::vec3& pBody1LinearVelocity, const glm::vec3& pBody1AngularVelocity, const float& pBody1Mass, const glm::mat3& pBody1InertiaTensor
                                , const glm::vec3& pBody2CenterOfMassPositionWorld, const glm::vec3& pBody2LinearVelocity, const glm::vec3& pBody2AngularVelocity, const float& pBody2Mass, const glm::mat3& pBody2InertiaTensor);
}