#pragma once

#include "Component/Texture.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include "Utility/Config.hpp"

namespace Component
{
    struct Particle
    {
        glm::vec4 position;
        glm::vec4 velocity;
        DeltaTime lifetime;
    };

    class ParticleEmitter
    {
    public:
        ParticleEmitter(const TextureRef& p_texture);

        TextureRef diffuse;           // Texture applied to all the particles.
        glm::vec3 emit_position;      // Position particles emit from.
        glm::vec3 emit_velocity_min;  // Minimum starting velocity in m/s.
        glm::vec3 emit_velocity_max;  // Maximum starting velocity in m/s.
        DeltaTime spawn_period;       // Duration between particle spawn attempts.
        DeltaTime time_to_next_spawn; // Duration remaining to next spawn attempt.
        size_t spawn_count;           // How many particles to spawn every spawn_period.
        DeltaTime lifetime;           // Duration in seconds a particle stays alive before being removed.
        size_t max_particle_count;    // Max number of particles that can be alive concurrently.

        std::vector<Particle> particles;
    };
}