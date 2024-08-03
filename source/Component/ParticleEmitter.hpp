#pragma once

#include "Component/Texture.hpp"
#include "Utility/Config.hpp"

#include "OpenGL/Types.hpp"

#include "glm/vec3.hpp"

namespace System
{
	class TextureSystem;
}
namespace Component
{
	struct Particle // Matches the Particle struct in particle shaders.
	{
		glm::vec4 position;
		glm::vec4 velocity;
	};

	class ParticleEmitter
	{
	public:
		constexpr static size_t Persistent_ID = 8;

		TextureRef diffuse;              // Texture applied to all the particles.
		glm::vec3 emit_position;         // Position particles emit from.
		glm::vec3 emit_velocity_min;     // Minimum starting velocity in m/s.
		glm::vec3 emit_velocity_max;     // Maximum starting velocity in m/s.
		DeltaTime spawn_period;          // Duration between particle spawn attempts.
		DeltaTime time_to_next_spawn;    // Duration remaining to next spawn attempt.
		DeltaTime lifetime;              // Duration in seconds a particle stays alive before being removed.
		GLsizei spawn_count;             // How many particles to spawn every spawn_period.
		GLsizei max_particle_count;      // Max number of particles that can be alive concurrently.
		GLsizei alive_count;             // Number of particles currently alive. This value is out of date when particles expire after particle_update kernel runs.
		float particle_size = 1.f;       // Size of each particle.
		bool sort_by_distance_to_camera; // Whether to sort the particles and draw the most distant particles last.

		OpenGL::Buffer particle_buf; // Contains instances of Particle struct.

		ParticleEmitter(const TextureRef& p_texture);
		void draw_UI(System::TextureSystem& p_texture_system);
	};
}