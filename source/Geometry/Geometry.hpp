#pragma once

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"

#include <array>
#include <numbers>
#include <utility>

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

    // Get a pair of arrays defining a standard icosahedron. A platonic solid with 20 faces, 30 edges and 12 vertices.
    // Use get_icosahedron_points to return a flat list of points.
    //@return A pair of arrays, =points, second=indices.
    [[nodiscard]] consteval std::pair<std::array<glm::vec3, 12>, std::array<unsigned int, 60> > get_icosahedron_points_and_indices()
    {
        // create 12 vertices of a icosahedron
        constexpr auto t = std::numbers::phi_v<float>; // golden ratio
        return {
        {
            glm::vec3(-1.f,  t,  0.f),
            glm::vec3( 1.f,  t,  0.f),
            glm::vec3(-1.f, -t,  0.f),
            glm::vec3( 1.f, -t,  0.f),

            glm::vec3( 0.f, -1.f,  t),
            glm::vec3( 0.f,  1.f,  t),
            glm::vec3( 0.f, -1.f, -t),
            glm::vec3( 0.f,  1.f, -t),

            glm::vec3( t,  0.f, -1.f),
            glm::vec3( t,  0.f,  1.f),
            glm::vec3(-t,  0.f, -1.f),
            glm::vec3(-t,  0.f,  1.f)
        },
        {
            0, 11, 5,
            0, 5, 1,
            0, 1, 7,
            0, 7, 10,
            0, 10, 11,

            1, 5, 9,
            5, 11, 4,
            11, 10, 2,
            10, 7, 6,
            7, 1, 8,

            3, 9, 4,
            3, 4, 2,
            3, 2, 6,
            3, 6, 8,
            3, 8, 9,

            4, 9, 5,
            2, 4, 11,
            6, 2, 10,
            8, 6, 7,
            9, 8, 1
        }
    };
    }
    // Get an array defining a standard icosahedron. A platonic solid with 20 faces, 30 edges and 12 vertices.
    // Use get_icosahedron_points_and_indices to return a lisr of unique points and indices.
    //@return An array defining the vertex positions of the icosahedron
    [[nodiscard]] consteval std::array<glm::vec3, 60> get_icosahedron_points()
    {
        constexpr auto standard_points  = Geometry::get_icosahedron_points_and_indices().first;
        constexpr auto indices = Geometry::get_icosahedron_points_and_indices().second;

        std::array<glm::vec3, 60> points = {};
        for (auto i = 0; i < indices.size(); i++)
            points[i] = standard_points[indices[i]];
        return points;
    }
}