#pragma once

#include "Geometry.hpp"

#include <cmath>

namespace Geometry
{
    glm::vec3 CuboidInertia(float pMass, float pHeight, float pWidth, float pDepth)
    {
        const float InertiaX = (1.f/12.f) * pMass * (std::pow(pDepth, 2.f) + std::pow(pHeight, 2.f));
        const float InertiaY = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pDepth, 2.f));
        const float InertiaZ = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pHeight, 2.f));
        return glm::vec3(InertiaX, InertiaY, InertiaZ);
    }

    glm::mat3 cylinderInertiaTensor(const float& pMass, const float& pRadius, const float& pHeight)
    {
        // https://en.wikipedia.org/wiki/List_of_moments_of_inertia
        // Cylinder oriented with height along z-axis
        const float x = (1.f / 12.f) * pMass * ((3.f * std::pow(pRadius, 2.f)) + std::pow(pHeight, 2.f));
        const float y = ((1.f / 2.f) * pMass * std::pow(pRadius, 2.f));

        return glm::mat3(  x , 0.f, 0.f
                        , 0.f,  x , 0.f
                        , 0.f, 0.f,  y );
    }
    glm::mat3 cuboidInertiaTensor(const float& pMass, const float& pWidth, const float& pHeight, const float& pDepth)
    {
        // https://en.wikipedia.org/wiki/List_of_moments_of_inertia

        const float x = (1.f / 12.f) * pMass;
        return glm::mat3( x * (std::pow(pHeight, 2.f) + std::pow(pDepth, 2.f)), 0.f, 0.f
                        , 0.f, x * (std::pow(pWidth, 2.f) + std::pow(pHeight, 2.f)), 0.f
                        , 0.f, 0.f,  x * (std::pow(pWidth, 2.f) + std::pow(pDepth, 2.f)) );
    }

    float linearImpulseMagnitude(const float& pMass1, const glm::vec3& pVelocity1, const float& pMass2, const glm::vec3& pVelocity2, const glm::vec3& pCollisionNormal, const float& pRestitution)
    {
        // Adapted from: 3D Math Primer for Graphics and Games Development - 12.4.2 General collision response - pg 598
        const auto relativeVelocity = pVelocity1 - pVelocity2;
        return ((pRestitution + 1) * glm::dot(relativeVelocity, pCollisionNormal)) / ((1 / pMass1) + (1 / pMass2) * glm::dot(pCollisionNormal, pCollisionNormal));
    }

    glm::vec3 angularImpulse(const glm::vec3& pCollisionPointWorldSpace, const glm::vec3& pCollisionNormal, const float& pRestitution
                                       , const glm::vec3& pBody1CenterOfMassPositionWorld, const glm::vec3& pBody1LinearVelocity, const glm::vec3& pBody1AngularVelocity, const float& pBody1Mass, const glm::mat3& pBody1InertiaTensor
                                       , const glm::vec3& pBody2CenterOfMassPositionWorld, const glm::vec3& pBody2LinearVelocity, const glm::vec3& pBody2AngularVelocity, const float& pBody2Mass, const glm::mat3& pBody2InertiaTensor)
    {
        // Adapted from: 3D Math Primer for Graphics and Games Development - 12.5.4 Collision Response with Rotations - pg 620

        // e = Restitution
        // u = Linear velocity at point of impact of the body
        // v = Linear velocity at center of mass of the body
        // m = Mass
        // r = Position of point of impact relative to center of mass (object space)
        // J = Inertia tensor
        // ω = Angular velocity

        const auto r1 = pCollisionPointWorldSpace - pBody1CenterOfMassPositionWorld;
        const auto r2 = pCollisionPointWorldSpace - pBody2CenterOfMassPositionWorld;

        // u = v + ω x p (The velocity(u) of a point(p) on a body is equal to linear velocity(v) + the CROSS of angular velocity(ω) AND p) (p must be in object space)
        const auto u1           = pBody1LinearVelocity + glm::cross(pBody1AngularVelocity, r1);
        const auto u2           = pBody2LinearVelocity + glm::cross(pBody2AngularVelocity, r2);
        const auto uRelative    = u1 - u2;

        const auto inverseJ1    = glm::inverse(pBody1InertiaTensor);
        const auto inverseJ2    = glm::inverse(pBody2InertiaTensor);
        const auto inverseMass1 = 1.f / pBody1Mass;
        const auto inverseMass2 = 1.f / pBody2Mass;

        const auto impulseMagnitude =                                              ((pRestitution + 1) * glm::dot(uRelative, pCollisionNormal)) /
                glm::dot(((inverseMass1 + inverseMass2) * pCollisionNormal)   +   (glm::cross((glm::cross(r1, pCollisionNormal) * inverseJ1), r1))   +   (glm::cross((glm::cross(r2, pCollisionNormal) * inverseJ2), r2)), pCollisionNormal);

        return impulseMagnitude * pCollisionNormal;
    }
}