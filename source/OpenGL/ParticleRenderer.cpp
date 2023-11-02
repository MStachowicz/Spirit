#include "ParticleRenderer.hpp"

#include "System/SceneSystem.hpp"

#include "OpenGL/GLState.hpp"
#include "OpenGL/Types.hpp"

#include "glm/gtx/norm.hpp"

#include <random>

namespace OpenGL
{
    ParticleRenderer::ParticleRenderer()
        : m_particle_shader{"particle"}
        , m_quad_VAO{}
        , m_quad_VBO{}
        , m_quad_EBO{}
        , m_particle_buffer{m_particle_shader.get_SSBO_backing("ParticlesBuffer")}
    {// Prepare the quad buffer for rendering.
        m_quad_VAO.bind();
        m_quad_VBO.bind();
        constexpr auto size = sizeof(float) * quad_vertices.size();
        buffer_data(BufferType::ArrayBuffer, size, &quad_vertices.front(), BufferUsage::StaticDraw);
        vertex_attrib_pointer(0, 3, ShaderDataType::Float, false, sizeof(float) * 5, (void*)0);                   // Position
        enable_vertex_attrib_array(0);
        vertex_attrib_pointer(3, 2, ShaderDataType::Float, false, sizeof(float) * 5, (void*)(3 * sizeof(float))); // Texture
        enable_vertex_attrib_array(3);
        m_quad_EBO.bind();
        buffer_data(BufferType::ElementArrayBuffer, sizeof(quad_indices), &quad_indices.front(), BufferUsage::StaticDraw);
    }

    void ParticleRenderer::update(const DeltaTime& p_delta_time, System::Scene& p_scene, const glm::vec3& p_camera_position)
    {
        p_scene.m_entities.foreach([&](Component::ParticleEmitter& p_emitter)
        {
            constexpr auto zero_seconds = std::chrono::seconds(0);

            {// Spawning new particles every spawn_period
                p_emitter.time_to_next_spawn -= p_delta_time;
                if (p_emitter.time_to_next_spawn <= zero_seconds)
                {
                    p_emitter.time_to_next_spawn = p_emitter.spawn_period; // Reset time.

                    auto current_particle_count = static_cast<unsigned int>(p_emitter.particles.size()); // TODO: Casting down, check size_t fits into the uint
                    if (current_particle_count < p_emitter.max_particle_count)
                    {
                        auto remaining_size     = p_emitter.max_particle_count - current_particle_count;
                        auto new_particle_count = std::min(remaining_size, p_emitter.spawn_count);

                        if (new_particle_count > 0)
                        {// Create random number generators for each component
                            ASSERT(p_emitter.emit_velocity_min.x < p_emitter.emit_velocity_max.x
                                && p_emitter.emit_velocity_min.y < p_emitter.emit_velocity_max.y
                                && p_emitter.emit_velocity_min.z < p_emitter.emit_velocity_max.z, "ParticleEmitter min not smaller than max"); // Always

                            auto rd           = std::random_device();
                            auto gen          = std::mt19937(rd());
                            auto distribution = std::uniform_real_distribution<float>(0.f, 1.f);

                            for (auto i = 0; i < new_particle_count; i++)
                            {
                                auto vel = glm::vec4{ // Scale distribution(gen) from [0 - 1] to [min - max]
                                    p_emitter.emit_velocity_min.x + (distribution(gen) * (p_emitter.emit_velocity_max.x - p_emitter.emit_velocity_min.x)),
                                    p_emitter.emit_velocity_min.y + (distribution(gen) * (p_emitter.emit_velocity_max.y - p_emitter.emit_velocity_min.y)),
                                    p_emitter.emit_velocity_min.z + (distribution(gen) * (p_emitter.emit_velocity_max.z - p_emitter.emit_velocity_min.z)),
                                    1.f
                                };
                                p_emitter.particles.push_back({glm::vec4(p_emitter.emit_position, 1.f), vel, p_emitter.lifetime});
                            }
                        }
                    }
                }
            }
            {// Update particle lifetimes and positions.
                // When the lifetime of a particle is below zero, remove it from the vector.
                // Otherwise, integrate its position by the velocity.
                for (auto it = p_emitter.particles.begin(); it != p_emitter.particles.end();)
                {
                    it->lifetime -= p_delta_time;

                    if (it->lifetime <= zero_seconds)
                        it = p_emitter.particles.erase(it);
                    else
                    {
                        it->position += (it->velocity * p_delta_time.count());
                        ++it;
                    }
                }
            }
            if (p_emitter.sort_by_distance_to_camera)
            {
                const auto camera_position = glm::vec4(p_camera_position, 0.f);
                for (auto& particle : p_emitter.particles)
                    particle.distance_to_camera = glm::distance2(camera_position, particle.position);

                std::sort(p_emitter.particles.begin(), p_emitter.particles.end(), [](const auto& lhs, const auto& rhs)
                {
                    return lhs.distance_to_camera > rhs.distance_to_camera;
                });
            }

            {// Upload the particle data to the SSBO in particle_shader
                m_particle_buffer->bind();

                // TODO: set these by querying the SSBO using GL introspection.
                GLint particle_count_offset       = 0;
                GLint particle_count_size         = 16; // Size of the uint number_of_particles variable.
                GLint particle_array_start_offset = 16; // Starts after number_of_particles + padding.
                GLint particle_position_offset    = 0;  // Offset from the start of an index to the position var.
                GLint particle_velocity_offset    = 16; // Offset from the start of an index to the velocity var.
                GLintptr particle_stride          = 32; // Size of one particle

                auto particle_count = p_emitter.particles.size();
                const auto required_size = particle_count_size + particle_count * particle_stride;
                // Resize the buffer to accomodate at least the directional_light_count
                if (required_size > m_particle_buffer->m_size)
                {
                    LOG("[OPENGL][PARTICLE RENDERER] ParticleEmitter particle count changed ({}), resized the particles buffer to {}B", particle_count, required_size);
                    auto grow_size = required_size - m_particle_buffer->m_size;

                    buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
                    m_particle_buffer->m_size = required_size;
                    // Binding point = 3?
                    bind_buffer_range(BufferType::ShaderStorageBuffer, m_particle_buffer->m_binding_point, m_particle_buffer->m_handle, 0, m_particle_buffer->m_size);

                    if (particle_count_offset > particle_array_start_offset)
                    { // The var is after the variable sized array, update its offset by the growth of the variable-sized-array.
                        particle_count_offset += grow_size;
                    }
                }

                // Set the count
                auto uint_particle_count = static_cast<unsigned int>(particle_count);
                buffer_sub_data(BufferType::ShaderStorageBuffer, particle_count_offset, sizeof(unsigned int), &uint_particle_count);

                GLuint i = 0;
                for (auto i = 0; i < p_emitter.particles.size(); i++)
                {
                    buffer_sub_data(BufferType::ShaderStorageBuffer, particle_array_start_offset + particle_position_offset + (particle_stride * i), sizeof(glm::vec4), &p_emitter.particles[i].position);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, particle_array_start_offset + particle_velocity_offset + (particle_stride * i), sizeof(glm::vec4), &p_emitter.particles[i].velocity);
                }
            }
            { // Draw the particles
                m_particle_shader.use();
                m_particle_shader.set_uniform("diffuse", 0);
                active_texture(0);
                p_emitter.diffuse->m_GL_texture.bind();
                m_quad_VAO.bind();
                draw_elements_instanced(PrimitiveMode::Triangles, 6, p_emitter.particles.size());
            }
        });
    }
}