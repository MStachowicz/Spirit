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
	glm::vec3 cuboid_intertia(float p_mass, float p_height, float p_width, float p_depth);
	glm::mat3 cylinder_inertia_tensor(const float& p_mass, const float& p_radius, const float& p_height);
	glm::mat3 cuboid_inertia_tensor(const float& p_mass, const float& p_width, const float& p_height, const float& p_depth);

	// Returns the magnitude of the impulse after a collision between bodies 1 and 2.
	// Impulse is a vector quantity, which can be derived by multiplying this magnitude with p_collision_normal for each body.
	// p_restitution ranges from 0 to 1 where 1 is a perfectly elastic collision and 0 is perfectly inelastic.
	float linear_impulse_magnitude(const float& p_mass_1, const glm::vec3& p_velocity_1, const float& p_mass_2, const glm::vec3& p_velocity_2, const glm::vec3& p_collision_normal, const float& p_restitution);

	// Returns the angular impulse after a collision between bodies 1 and 2 at pCollisionPoint in p_collision_normal direction.
	// Conevntion adopted here is p_collision_normal points from body1 to body 2's surface. The returned impulse is applied in reverse to body 1 and directly to body 2.
	// p_restitution ranges from 0 to 1 where 1 is a perfectly elastic collision and 0 is perfectly inelastic.
	glm::vec3 angular_impulse(const glm::vec3& p_collision_point_world_space, const glm::vec3& p_collision_normal, const float& p_restitution
								, const glm::vec3& p_body_1_center_of_mass_position_world, const glm::vec3& p_body_1_linear_velocity, const glm::vec3& p_body_1_angular_velocity, const float& p_body_1_mass, const glm::mat3& p_body_1_inertia_tensor
								, const glm::vec3& p_body_2_center_of_mass_position_world, const glm::vec3& p_body_2_linear_velocity, const glm::vec3& p_body_2_angular_velocity, const float& p_body_2_mass, const glm::mat3& p_body_2_inertia_tensor);
}