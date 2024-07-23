#include "ParticleRenderer.hpp"
#include "GLState.hpp"
#include "DrawCall.hpp"

#include "System/SceneSystem.hpp"
#include "Component/ParticleEmitter.hpp"
#include "Utility/Utility.hpp"

#include "glm/gtx/norm.hpp"

#include <array>
#include <random>

namespace OpenGL
{
	struct Vert
	{
		glm::vec3 pos;
		glm::vec2 tex;
	};
	constexpr std::array<Vert, 4> vertices = {
		Vert{glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}, // top right
		Vert{glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)}, // bottom right
		Vert{glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f)}, // top left
		Vert{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}, // bottom left
	};
	constexpr std::array<GLuint, 6> indices = {
		0, 1, 2, // first triangle
		2, 3, 1  // second triangle
	};

	constexpr GLsizeiptr particle_position_offset  = static_cast<GLsizeiptr>(offsetof(Component::Particle, position));
	constexpr GLsizeiptr particle_velocity_offset  = static_cast<GLsizeiptr>(offsetof(Component::Particle, velocity));
	constexpr GLsizeiptr particle_stride           = sizeof(Component::Particle);

	ParticleRenderer::ParticleRenderer()
		: m_particle_draw_shader{"particle"}
		, m_particle_update_shader{"particle_update"}
		, m_quad_VAO{}
		, m_quad_VBO{{OpenGL::BufferStorageFlag::DynamicStorageBit}} // TODO: Can we use non-dynamic storage for this?
		, m_quad_EBO{{OpenGL::BufferStorageFlag::DynamicStorageBit}} // TODO: Can we use non-dynamic storage for this?
	{
		{// Confirm the sizes and offsets of members in the ParticlesBuffer
		#ifdef Z_DEBUG // Ensure the particles Shader Storage Block matches the particles struct for direct memory copy.
		{
			const auto& particles_SSB = m_particle_draw_shader.get_shader_storage_block("ParticlesBuffer");
			ASSERT(particles_SSB.m_data_size == particle_stride, "ParticlesBuffer size mismatch");
			ASSERT(particles_SSB.m_variables.size() == 2, "ParticlesBuffer variable count mismatch");

			const auto& particle_position = particles_SSB.get_variable("particles[0].position");
			ASSERT(particle_position.m_offset == particle_position_offset, "ParticlesBuffer member offset mismatch");
			ASSERT(particle_position.m_type == ShaderDataType::Vec4, "ParticlesBuffer member type mismatch");

			const auto& particle_velocity = particles_SSB.get_variable("particles[0].velocity");
			ASSERT(particle_velocity.m_offset == particle_velocity_offset, "ParticlesBuffer member offset mismatch");
			ASSERT(particle_velocity.m_type == ShaderDataType::Vec4, "ParticlesBuffer member type mismatch");
		}
		#endif

			constexpr GLuint vertex_buffer_binding_point = 0;
			m_quad_VBO.upload_data(vertices);
			m_quad_EBO.upload_data(indices);
			m_quad_VAO.attach_buffer(m_quad_VBO, 0, vertex_buffer_binding_point, sizeof(Vert));
			m_quad_VAO.attach_element_buffer(m_quad_EBO, (GLsizei)indices.size());
			m_quad_VAO.set_vertex_attrib_pointers(PrimitiveMode::Triangles, {{
					VertexAttributeMeta{m_particle_draw_shader.get_attribute_index("VertexPosition"), 3, BufferDataType::Float, offsetof(Vert, pos), vertex_buffer_binding_point, false},
					VertexAttributeMeta{m_particle_draw_shader.get_attribute_index("VertexTexCoord"), 2, BufferDataType::Float, offsetof(Vert, tex), vertex_buffer_binding_point, false}
				}});
		}
	}

	void ParticleRenderer::update(const DeltaTime& p_delta_time, System::Scene& p_scene, const glm::vec3& p_camera_position, const Buffer& p_view_properties, const FBO& p_target_FBO)
	{ (void)p_camera_position;
		p_scene.m_entities.foreach([&](Component::ParticleEmitter& p_emitter)
		{
			constexpr auto zero_seconds = std::chrono::seconds(0);

			{// Spawning new particles every spawn_period
				p_emitter.time_to_next_spawn -= p_delta_time;

				if (p_emitter.time_to_next_spawn <= zero_seconds)
				{
					p_emitter.time_to_next_spawn = p_emitter.spawn_period; // Reset time.

					if (p_emitter.alive_count < p_emitter.max_particle_count)
					{
						auto remaining_size     = p_emitter.max_particle_count - p_emitter.alive_count;
						auto new_particle_count = std::min(remaining_size, p_emitter.spawn_count);

						if (new_particle_count > 0)
						{
							auto rd           = std::random_device();
							auto gen          = std::mt19937(rd());
							auto distribution = std::uniform_real_distribution<float>(0.f, 1.f);

							std::vector<Component::Particle> new_particles;
							new_particles.reserve(new_particle_count);
							for (int i = 0; i < new_particle_count; i++)
							{
								auto vel = glm::vec4{ // Scale distribution(gen) from [0 - 1] to [min - max]
									p_emitter.emit_velocity_min.x + (distribution(gen) * (p_emitter.emit_velocity_max.x - p_emitter.emit_velocity_min.x)),
									p_emitter.emit_velocity_min.y + (distribution(gen) * (p_emitter.emit_velocity_max.y - p_emitter.emit_velocity_min.y)),
									p_emitter.emit_velocity_min.z + (distribution(gen) * (p_emitter.emit_velocity_max.z - p_emitter.emit_velocity_min.z)),
									1.f
								};
								new_particles.emplace_back<Component::Particle>({glm::vec4{p_emitter.emit_position, p_emitter.lifetime.count()}, vel});
							}

							const GLsizeiptr new_size = (particle_stride * p_emitter.alive_count) + (particle_stride * new_particle_count);
							if (p_emitter.particle_buf.size() < new_size)
							{
								const GLsizeiptr new_size_pwr_2 = Utility::next_power_of_2(new_size);
								LOG("Resizing particle buffer from {}B to {}B", p_emitter.particle_buf.size(), new_size_pwr_2)

								auto new_particle_buff = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
								new_particle_buff.resize(new_size_pwr_2);
								new_particle_buff.copy_sub_data(p_emitter.particle_buf, 0, 0, p_emitter.particle_buf.size());
								p_emitter.particle_buf = std::move(new_particle_buff);
							}

							p_emitter.particle_buf.buffer_sub_data(particle_stride * p_emitter.alive_count, new_particles);
							p_emitter.alive_count += new_particle_count;
						}
					}
				}
			}

			// p_emitter.alive_count is incorrect after particle lifetimes expire in the particle_update kernel
			if (p_emitter.alive_count > 0)
			{
				// Update the particles
				DrawCall comp;
				comp.set_SSBO("ParticlesBuffer", p_emitter.particle_buf);
				comp.set_uniform<float>("delta_time", p_delta_time.count());
				comp.submit_compute(m_particle_update_shader, p_emitter.alive_count, 1, 1);
				OpenGL::memory_barrier({OpenGL::MemoryBarrierFlag::ShaderStorageBarrierBit});

				// Draw the particles
				DrawCall dc;
				dc.m_cull_face_enabled  = false;
				dc.m_blending_enabled   = true;
				// Additive blending.
				dc.m_source_factor      = BlendFactorType::SourceAlpha;
				dc.m_destination_factor = BlendFactorType::One;
				dc.set_texture("diffuse", p_emitter.diffuse->m_GL_texture);
				dc.set_SSBO("ParticlesBuffer", p_emitter.particle_buf);
				dc.set_UBO("ViewProperties", p_view_properties);
				dc.submit_instanced(m_particle_draw_shader, m_quad_VAO, p_target_FBO, p_emitter.alive_count);
			}
		});
	}
} // namespace OpenGL