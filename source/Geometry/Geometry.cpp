#include "Geometry.hpp"

#include <cmath>

namespace Geometry
{
	glm::vec3 cuboid_intertia(float p_mass, float p_height, float p_width, float p_depth)
	{
		const float InertiaX = (1.f/12.f) * p_mass * (std::pow(p_depth, 2.f) + std::pow(p_height, 2.f));
		const float InertiaY = (1.f/12.f) * p_mass * (std::pow(p_width, 2.f) + std::pow(p_depth, 2.f));
		const float InertiaZ = (1.f/12.f) * p_mass * (std::pow(p_width, 2.f) + std::pow(p_height, 2.f));
		return glm::vec3(InertiaX, InertiaY, InertiaZ);
	}

	glm::mat3 cylinder_inertia_tensor(const float& p_mass, const float& p_radius, const float& p_height)
	{
		// https://en.wikipedia.org/wiki/List_of_moments_of_inertia
		// Cylinder oriented with height along z-axis
		const float x = (1.f / 12.f) * p_mass * ((3.f * std::pow(p_radius, 2.f)) + std::pow(p_height, 2.f));
		const float y = ((1.f / 2.f) * p_mass * std::pow(p_radius, 2.f));

		return glm::mat3(  x , 0.f, 0.f
						, 0.f,  x , 0.f
						, 0.f, 0.f,  y );
	}
	glm::mat3 cuboid_inertia_tensor(const float& p_mass, const float& p_width, const float& p_height, const float& p_depth)
	{
		// https://en.wikipedia.org/wiki/List_of_moments_of_inertia

		const float x = (1.f / 12.f) * p_mass;
		return glm::mat3( x * (std::pow(p_height, 2.f) + std::pow(p_depth, 2.f)), 0.f, 0.f
						, 0.f, x * (std::pow(p_width, 2.f) + std::pow(p_height, 2.f)), 0.f
						, 0.f, 0.f,  x * (std::pow(p_width, 2.f) + std::pow(p_depth, 2.f)) );
	}

	float linear_impulse_magnitude(const float& p_mass_1, const glm::vec3& p_velocity_1, const float& p_mass_2, const glm::vec3& p_velocity_2, const glm::vec3& p_collision_normal, const float& p_restitution)
	{
		// Adapted from: 3D Math Primer for Graphics and Games Development - 12.4.2 General collision response - pg 598
		const auto relative_velocity = p_velocity_1 - p_velocity_2;
		return ((p_restitution + 1) * glm::dot(relative_velocity, p_collision_normal)) / ((1 / p_mass_1) + (1 / p_mass_2) * glm::dot(p_collision_normal, p_collision_normal));
	}

	glm::vec3 angular_impulse(const glm::vec3& p_collision_point_world_space, const glm::vec3& p_collision_normal, const float& p_restitution
								, const glm::vec3& p_body_1_center_of_mass_position_world, const glm::vec3& p_body_1_linear_velocity, const glm::vec3& p_body_1_angular_velocity, const float& p_body_1_mass, const glm::mat3& p_body_1_inertia_tensor
								, const glm::vec3& p_body_2_center_of_mass_position_world, const glm::vec3& p_body_2_linear_velocity, const glm::vec3& p_body_2_angular_velocity, const float& p_body_2_mass, const glm::mat3& p_body_2_inertia_tensor)
	{
		// Adapted from: 3D Math Primer for Graphics and Games Development - 12.5.4 Collision Response with Rotations - pg 620

		// e = Restitution
		// u = Linear velocity at point of impact of the body
		// v = Linear velocity at center of mass of the body
		// m = Mass
		// r = Position of point of impact relative to center of mass (object space)
		// J = Inertia tensor
		// ω = Angular velocity

		const auto r1 = p_collision_point_world_space - p_body_1_center_of_mass_position_world;
		const auto r2 = p_collision_point_world_space - p_body_2_center_of_mass_position_world;

		// u = v + ω x p (The velocity(u) of a point(p) on a body is equal to linear velocity(v) + the CROSS of angular velocity(ω) AND p) (p must be in object space)
		const auto u1           = p_body_1_linear_velocity + glm::cross(p_body_1_angular_velocity, r1);
		const auto u2           = p_body_2_linear_velocity + glm::cross(p_body_2_angular_velocity, r2);
		const auto u_relative   = u1 - u2;

		const auto inverseJ_1      = glm::inverse(p_body_1_inertia_tensor);
		const auto inverseJ_2      = glm::inverse(p_body_2_inertia_tensor);
		const auto inverse_mass_1 = 1.f / p_body_1_mass;
		const auto inverse_mass_2 = 1.f / p_body_2_mass;

		const auto impulseMagnitude = ((p_restitution + 1) * glm::dot(u_relative, p_collision_normal)) /
			glm::dot(((inverse_mass_1 + inverse_mass_2) * p_collision_normal) + (glm::cross((glm::cross(r1, p_collision_normal) * inverseJ_1), r1)) + (glm::cross((glm::cross(r2, p_collision_normal) * inverseJ_2), r2)), p_collision_normal);

		return impulseMagnitude * p_collision_normal;
	}
}