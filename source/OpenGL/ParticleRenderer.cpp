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
	constexpr GLsizeiptr particle_position_offset  = static_cast<GLsizeiptr>(offsetof(Component::Particle, position));
	constexpr GLsizeiptr particle_velocity_offset  = static_cast<GLsizeiptr>(offsetof(Component::Particle, velocity));
	constexpr GLsizeiptr particle_stride           = sizeof(Component::Particle);

	ParticleRenderer::ParticleRenderer()
	    : m_particle_draw_constant_colour_fixed_size{"particle", {"CONSTANT_COLOUR", "FIXED_SIZE", "HAS_COLOUR", "STYLE_CONSTANT_COLOUR"}}
	    , m_particle_draw_constant_texture_fixed_size{"particle", {"CONSTANT_TEXTURE", "FIXED_SIZE", "HAS_TEXTURE", "STYLE_CONSTANT_TEXTURE"}}
	    , m_particle_draw_constant_colour_and_texture_fixed_size{"particle", {"CONSTANT_COLOUR", "CONSTANT_TEXTURE", "FIXED_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "STYLE_CONSTANT_COLOUR_AND_TEXTURE"}}
	    , m_particle_draw_varying_colour_fixed_size{"particle", {"VARYING_COLOUR", "FIXED_SIZE", "HAS_COLOUR", "HAS_VARYING", "STYLE_VARYING_COLOUR"}}
	    , m_particle_draw_varying_texture_fixed_size{"particle", {"VARYING_TEXTURE", "FIXED_SIZE", "HAS_TEXTURE", "HAS_VARYING", "STYLE_VARYING_TEXTURE"}}
	    , m_particle_draw_varying_colour_constant_texture_fixed_size{"particle", {"VARYING_COLOUR", "CONSTANT_TEXTURE", "FIXED_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_VARYING_COLOUR_CONSTANT_TEXTURE"}}
	    , m_particle_draw_constant_colour_varying_texture_fixed_size{"particle", {"CONSTANT_COLOUR", "VARYING_TEXTURE", "FIXED_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_CONSTANT_COLOUR_VARYING_TEXTURE"}}
	    , m_particle_draw_varying_colour_and_texture_fixed_size{"particle", {"VARYING_COLOUR", "VARYING_TEXTURE", "FIXED_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_VARYING_COLOUR_AND_TEXTURE"}}
	    , m_particle_draw_constant_colour_varying_size{"particle", {"CONSTANT_COLOUR", "VARYING_SIZE", "HAS_COLOUR", "HAS_VARYING", "STYLE_CONSTANT_COLOUR"}}
	    , m_particle_draw_constant_texture_varying_size{"particle", {"CONSTANT_TEXTURE", "VARYING_SIZE", "HAS_TEXTURE", "HAS_VARYING", "STYLE_CONSTANT_TEXTURE"}}
	    , m_particle_draw_constant_colour_and_texture_varying_size{"particle", {"CONSTANT_COLOUR", "CONSTANT_TEXTURE", "VARYING_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_CONSTANT_COLOUR_AND_TEXTURE"}}
	    , m_particle_draw_varying_colour_varying_size{"particle", {"VARYING_COLOUR", "VARYING_SIZE", "HAS_COLOUR", "HAS_VARYING", "STYLE_VARYING_COLOUR"}}
	    , m_particle_draw_varying_texture_varying_size{"particle", {"VARYING_TEXTURE", "VARYING_SIZE", "HAS_TEXTURE", "HAS_VARYING", "STYLE_VARYING_TEXTURE"}}
	    , m_particle_draw_varying_colour_constant_texture_varying_size{"particle", {"VARYING_COLOUR", "CONSTANT_TEXTURE", "VARYING_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_VARYING_COLOUR_CONSTANT_TEXTURE"}}
	    , m_particle_draw_constant_colour_varying_texture_varying_size{"particle", {"CONSTANT_COLOUR", "VARYING_TEXTURE", "VARYING_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_CONSTANT_COLOUR_VARYING_TEXTURE"}}
	    , m_particle_draw_varying_colour_and_texture_varying_size{"particle", {"VARYING_COLOUR", "VARYING_TEXTURE", "VARYING_SIZE", "HAS_COLOUR", "HAS_TEXTURE", "HAS_VARYING", "STYLE_VARYING_COLOUR_AND_TEXTURE"}}
	    , m_particle_update{"particle_update"}
	    , m_particle_VAO{}
	{
		{// Confirm the sizes and offsets of members in the ParticlesBuffer
		#ifdef Z_DEBUG // Ensure the particles Shader Storage Block matches the particles struct for direct memory copy.
		{
			const auto& particles_SSB = m_particle_update.get_shader_storage_block("ParticlesBuffer");
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
		}
	}

	void ParticleRenderer::reload_shaders()
	{
		m_particle_draw_constant_colour_fixed_size.reload();
		m_particle_draw_constant_texture_fixed_size.reload();
		m_particle_draw_constant_colour_and_texture_fixed_size.reload();
		m_particle_draw_varying_colour_fixed_size.reload();
		m_particle_draw_varying_texture_fixed_size.reload();
		m_particle_draw_varying_colour_constant_texture_fixed_size.reload();
		m_particle_draw_constant_colour_varying_texture_fixed_size.reload();
		m_particle_draw_varying_colour_and_texture_fixed_size.reload();
		m_particle_draw_constant_colour_varying_size.reload();
		m_particle_draw_constant_texture_varying_size.reload();
		m_particle_draw_constant_colour_and_texture_varying_size.reload();
		m_particle_draw_varying_colour_varying_size.reload();
		m_particle_draw_varying_texture_varying_size.reload();
		m_particle_draw_varying_colour_constant_texture_varying_size.reload();
		m_particle_draw_constant_colour_varying_texture_varying_size.reload();
		m_particle_draw_varying_colour_and_texture_varying_size.reload();
		m_particle_update.reload();
	}

	void ParticleRenderer::update(const DeltaTime& p_delta_time, System::Scene& p_scene, const glm::vec3& p_camera_position, const Buffer& p_view_properties, const FBO& p_target_FBO)
	{ (void)p_camera_position;
		p_scene.m_entities.foreach([&](Component::ParticleEmitter& p_emitter)
		{
			constexpr auto zero_seconds = std::chrono::seconds(0);
			p_emitter.spawn_debt += p_delta_time.count() * p_emitter.spawn_per_second;

			if (p_emitter.spawn_debt > 1.f)
			{
				const unsigned int particles_to_spawn = (unsigned int)std::floor(p_emitter.spawn_debt);
				// Decrement as if the spawn exceeded to prevent the spawn_debt from growing indefinitely while the alive count limits spawn.
				p_emitter.spawn_debt -= particles_to_spawn;

				const unsigned int remaining_size     = p_emitter.max_particle_count - p_emitter.alive_count;
				const unsigned int new_particle_count = std::min(remaining_size, particles_to_spawn);

				auto rd           = std::random_device();
				auto gen          = std::mt19937(rd());
				auto distribution = std::uniform_real_distribution<float>(0.f, 1.f);

				std::vector<Component::Particle> new_particles;
				new_particles.reserve(new_particle_count);
				for (unsigned int i = 0; i < new_particle_count; i++)
				{
					auto lifetime = p_emitter.lifetime_min + (distribution(gen) * (p_emitter.lifetime_max - p_emitter.lifetime_min));
					auto vel = glm::vec4{ // Scale distribution(gen) from [0 - 1] to [min - max]
						p_emitter.emit_velocity_min.x + (distribution(gen) * (p_emitter.emit_velocity_max.x - p_emitter.emit_velocity_min.x)),
						p_emitter.emit_velocity_min.y + (distribution(gen) * (p_emitter.emit_velocity_max.y - p_emitter.emit_velocity_min.y)),
						p_emitter.emit_velocity_min.z + (distribution(gen) * (p_emitter.emit_velocity_max.z - p_emitter.emit_velocity_min.z)),
						lifetime.count() // Store the starting lifetime in the w component of the velocity.
					};
					auto pos = glm::vec4{
						p_emitter.emit_position_min.x + (distribution(gen) * (p_emitter.emit_position_max.x - p_emitter.emit_position_min.x)),
						p_emitter.emit_position_min.y + (distribution(gen) * (p_emitter.emit_position_max.y - p_emitter.emit_position_min.y)),
						p_emitter.emit_position_min.z + (distribution(gen) * (p_emitter.emit_position_max.z - p_emitter.emit_position_min.z)),
						lifetime.count() // Store the starting lifetime in the w component of the velocity.
					};
					new_particles.emplace_back<Component::Particle>({pos, vel});
				}

				const size_t required_capacity = (particle_stride * p_emitter.alive_count) + (particle_stride * new_particle_count);
				if (p_emitter.particle_buf.capacity() < required_capacity)
				{
					const GLsizeiptr new_size_pwr_2 = Utility::next_power_of_2(required_capacity);
					p_emitter.particle_buf.reserve(new_size_pwr_2);
					LOG("Resizing particle buffer from {}B to {}B", p_emitter.particle_buf.capacity(), new_size_pwr_2);
				}

				p_emitter.particle_buf.set_data(new_particles, particle_stride * p_emitter.alive_count);
				p_emitter.alive_count += new_particle_count;
			}

			// p_emitter.alive_count is incorrect after particle lifetimes expire in the particle_update kernel
			if (p_emitter.alive_count > 0)
			{
				// Update the particles
				DrawCall comp;
				comp.set_SSBO("ParticlesBuffer", p_emitter.particle_buf);
				comp.set_uniform<float>("delta_time", p_delta_time.count());
				comp.set_uniform("u_acceleration", p_emitter.acceleration);
				comp.submit_compute(m_particle_update, p_emitter.alive_count, 1, 1);
				OpenGL::memory_barrier({OpenGL::MemoryBarrierFlag::ShaderStorageBarrierBit});

				// Draw the particles
				DrawCall dc;
				dc.m_cull_face_enabled  = false;
				dc.m_depth_test_enabled = false;
				dc.m_blending_enabled   = true;

				if (p_emitter.blending_style == Component::ParticleEmitter::BlendingStyle::AlphaBlended)
				{
					dc.m_source_factor      = BlendFactorType::SourceAlpha;
					dc.m_destination_factor = BlendFactorType::OneMinusSourceAlpha;
				}
				else if (p_emitter.blending_style == Component::ParticleEmitter::BlendingStyle::Additive)
				{
					dc.m_source_factor      = BlendFactorType::SourceAlpha;
					dc.m_destination_factor = BlendFactorType::One;
				}
				else
					ASSERT_FAIL("Unknown blending style.");

				dc.set_UBO("ViewProperties", p_view_properties);

				auto emitter_colour_source = p_emitter.get_colour_source();
				auto size_source           = p_emitter.get_size_source();

				switch (emitter_colour_source)
				{
					case Component::ParticleEmitter::ColourSource::ConstantColour:
						dc.set_uniform("colour", p_emitter.start_colour.value());
						break;
					case Component::ParticleEmitter::ColourSource::ConstantTexture:
						dc.set_texture("diffuse", p_emitter.start_texture->m_GL_texture);
						break;
					case Component::ParticleEmitter::ColourSource::ConstantColourAndTexture:
						dc.set_uniform("colour", p_emitter.start_colour.value());
						dc.set_texture("diffuse", p_emitter.start_texture->m_GL_texture);
						break;
					case Component::ParticleEmitter::ColourSource::VaryingColour:
						dc.set_uniform("start_colour", p_emitter.start_colour.value());
						dc.set_uniform("end_colour", p_emitter.end_colour.value());
						break;
					case Component::ParticleEmitter::ColourSource::VaryingTexture:
						dc.set_texture("start_diffuse", p_emitter.start_texture->m_GL_texture);
						dc.set_texture("end_diffuse", p_emitter.end_texture->m_GL_texture);
						break;
					case Component::ParticleEmitter::ColourSource::VaryingColourConstantTexture:
						dc.set_uniform("start_colour", p_emitter.start_colour.value());
						dc.set_uniform("end_colour", p_emitter.end_colour.value());
						dc.set_texture("diffuse", p_emitter.start_texture->m_GL_texture);
						break;
					case Component::ParticleEmitter::ColourSource::ConstantColourVaryingTexture:
						dc.set_uniform("colour", p_emitter.start_colour.value());
						dc.set_texture("start_diffuse", p_emitter.start_texture->m_GL_texture);
						dc.set_texture("end_diffuse", p_emitter.end_texture->m_GL_texture);
						break;
					case Component::ParticleEmitter::ColourSource::VaryingColourAndTexture:
						dc.set_uniform("start_colour", p_emitter.start_colour.value());
						dc.set_uniform("end_colour", p_emitter.end_colour.value());
						dc.set_texture("start_diffuse", p_emitter.start_texture->m_GL_texture);
						dc.set_texture("end_diffuse", p_emitter.end_texture->m_GL_texture);
						break;
					default:
						ASSERT_FAIL("Unknown colour source")
				}
				switch (size_source)
				{
					case Component::ParticleEmitter::SizeSource::Constant:
						dc.set_uniform("size", p_emitter.start_size);
						break;
					case Component::ParticleEmitter::SizeSource::Varying:
						dc.set_uniform("start_size", p_emitter.start_size);
						dc.set_uniform("end_size", p_emitter.end_size.value());
						break;
					default:
						ASSERT_FAIL("Unknown size source")
				}

				constexpr GLuint vertex_buffer_binding_point = 0;

				// Could be using the Shader::get_attribute_index API.
				// particle_velocity is unused in the vertex shader so not using the Shader::get_attribute_index API here.
				constexpr GLuint particle_position_vertex_attribute_index = 0; // Matches the layout(location = 0) in the particle.vert shader.
				constexpr GLuint particle_velocity_vertex_attribute_index = 1;

				m_particle_VAO.attach_buffer(p_emitter.particle_buf, 0, vertex_buffer_binding_point, particle_stride, p_emitter.alive_count);
				m_particle_VAO.set_vertex_attrib_pointers(PrimitiveMode::Points, {{
						VertexAttributeMeta{particle_position_vertex_attribute_index, 4, BufferDataType::Float, particle_position_offset, vertex_buffer_binding_point, false},
						VertexAttributeMeta{particle_velocity_vertex_attribute_index, 4, BufferDataType::Float, particle_velocity_offset, vertex_buffer_binding_point, false}
					}});

				// Get the correct shader for the emitter's colour and size source.
				auto get_draw_shader = [&](Component::ParticleEmitter::ColourSource p_colour_source, Component::ParticleEmitter::SizeSource p_size_source) -> Shader&
				{
					switch (p_colour_source)
					{
						case Component::ParticleEmitter::ColourSource::ConstantColour:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_constant_colour_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_constant_colour_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::ConstantTexture:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_constant_texture_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_constant_texture_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::ConstantColourAndTexture:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_constant_colour_and_texture_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_constant_colour_and_texture_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::VaryingColour:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_varying_colour_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_varying_colour_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::VaryingTexture:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_varying_texture_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_varying_texture_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::VaryingColourConstantTexture:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_varying_colour_constant_texture_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_varying_colour_constant_texture_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::ConstantColourVaryingTexture:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_constant_colour_varying_texture_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_constant_colour_varying_texture_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						case Component::ParticleEmitter::ColourSource::VaryingColourAndTexture:
							switch (p_size_source)
							{
								case Component::ParticleEmitter::SizeSource::Constant:
									return m_particle_draw_varying_colour_and_texture_fixed_size;
								case Component::ParticleEmitter::SizeSource::Varying:
									return m_particle_draw_varying_colour_and_texture_varying_size;
								default:
									ASSERT_FAIL("Unknown size source")
							}
						default:
							ASSERT_FAIL("Unknown colour source")
					}
				};

				Shader& m_particle_draw_shader = get_draw_shader(emitter_colour_source, size_source);
				dc.submit(m_particle_draw_shader, m_particle_VAO, p_target_FBO);
			}
		});
	}
} // namespace OpenGL