#include "ParticleEmitter.hpp"

namespace Component
{
    ParticleEmitter::ParticleEmitter(const TextureRef& p_texture)
        : diffuse{p_texture}
        , emit_position{glm::vec3(0.f)}
        , emit_velocity_min{glm::vec3(-0.5f, 1.f, -0.5f)}
        , emit_velocity_max{glm::vec3( 0.5f, 1.f,  0.5f)}
        , spawn_period{1.f}
        , time_to_next_spawn{0.f} // Spawn on creation.
        , spawn_count{3u}
        , lifetime{7.f}
        , max_particle_count{1000}
        , particles{}
    {
        particles.reserve(max_particle_count);
    }
}