#include "PhongRenderer.hpp"

#include "Component/Lights.hpp"

#include "ECS/Storage.hpp"
#include "System/SceneSystem.hpp"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace OpenGL
{
	PhongRenderer::PhongRenderer()
		: m_phong_shader{"phong"}
		, m_directional_lights_buffer{m_phong_shader.get_SSBO_backing("DirectionalLightsBuffer")}
		, m_directional_light_fixed_size{0}
		, m_directional_light_count_offset{0}
		, m_directional_light_array_stride{0}
		, m_directional_light_array_start_offset{0}
		, m_directional_light_direction_offset{0}
		, m_directional_light_ambient_offset{0}
		, m_directional_light_diffuse_offset{0}
		, m_directional_light_specular_offset{0}
		, m_point_lights_buffer{m_phong_shader.get_SSBO_backing("PointLightsBuffer")}
		, m_point_light_fixed_size{0}
		, m_point_light_count_offset{0}
		, m_point_light_array_stride{0}
		, m_point_light_array_start_offset{0}
		, m_point_light_position_offset{0}
		, m_point_light_constant_offset{0}
		, m_point_light_linear_offset{0}
		, m_point_light_quadratic_offset{0}
		, m_point_light_ambient_offset{0}
		, m_point_light_diffuse_offset{0}
		, m_point_light_specular_offset{0}
		, m_spot_lights_buffer{m_phong_shader.get_SSBO_backing("SpotLightsBuffer")}
		, m_spot_light_fixed_size{0}
		, m_spot_light_count_offset{0}
		, m_spot_light_array_stride{0}
		, m_spot_light_array_start_offset{0}
		, m_spot_light_position_offset{0}
		, m_spot_light_direction_offset{0}
		, m_spot_light_cutoff_offset{0}
		, m_spot_light_outer_cutoff_offset{0}
		, m_spot_light_constant_offset{0}
		, m_spot_light_linear_offset{0}
		, m_spot_light_quadratic_offset{0}
		, m_spot_light_ambient_offset{0}
		, m_spot_light_diffuse_offset{0}
		, m_spot_light_specular_offset{0}
	{
		ASSERT(m_directional_lights_buffer.has_value(), "[OPENGL][PHONG] DirectionalLightsBuffer not found in phong shader");
		ASSERT(m_point_lights_buffer.has_value(), "[OPENGL][PHONG] PointLightsBuffer not found in phong shader");
		ASSERT(m_spot_lights_buffer.has_value(), "[OPENGL][PHONG] SpotLightsBuffer not found in phong shader");

		{ // Initialise offsets
			{ // DirectionalLight
				for (auto& var : m_directional_lights_buffer->m_variables)
				{ // Find the first variable in the directional lights array
					if (var.m_identifier.starts_with("directional_lights[0]"))
					{
						m_directional_light_array_start_offset = var.m_offset;
						// GL_TOP_LEVEL_ARRAY_SIZE is the same for all array members, first is arbitrarily chosen here.
						m_directional_light_array_stride = var.m_top_level_array_stride;

						break;
					}
				}
				ASSERT_THROW(m_directional_light_array_stride != 0, "[OPENGL][PHONG] directional_lights array stride cannot be 0");

				// Assuming here the storage block is initialised with a variable array size of 1.
				// In constructor for ShaderStorageBlock we query GL_BUFFER_DATA_SIZE stipulating:
				// "If the final member of an active ShaderStorageBlock is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element."
				m_directional_light_fixed_size = m_directional_lights_buffer->m_size - m_directional_light_array_stride;

				for (auto& var : m_directional_lights_buffer->m_variables)
				{
					if (var.m_identifier == "number_of_directional_lights")
					{
						ASSERT(var.m_type == ShaderDataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_directional_lights to be a uint.");
						m_directional_light_count_offset = var.m_offset;
					}
					else if (var.m_identifier == "directional_lights[0].direction")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].direction to be a vec3.");
						m_directional_light_direction_offset = var.m_offset;
					}
					else if (var.m_identifier == "directional_lights[0].ambient")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].ambient to be a vec3.");
						m_directional_light_ambient_offset = var.m_offset;
					}
					else if (var.m_identifier == "directional_lights[0].diffuse")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].diffuse to be a vec3.");
						m_directional_light_diffuse_offset = var.m_offset;
					}
					else if (var.m_identifier == "directional_lights[0].specular")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].specular to be a vec3.");
						m_directional_light_specular_offset = var.m_offset;
					}
					else
					{
						ASSERT(false, "[OPENGL][PHONG] Unexpected variable in the directional lights buffer.");
					}
				}
			}
			{ // PointLight
				for (auto& var : m_point_lights_buffer->m_variables)
				{ // Find the first variable in the point lights array
					if (var.m_identifier.starts_with("point_lights[0]"))
					{
						m_point_light_array_start_offset = var.m_offset;
						// GL_TOP_LEVEL_ARRAY_SIZE is the same for all array members, first is arbitrarily chosen here.
						m_point_light_array_stride = var.m_top_level_array_stride;

						break;
			}
				}
				ASSERT_THROW(m_point_light_array_stride != 0, "[OPENGL][PHONG] point_lights array stride cannot be 0");

				// Assuming here the storage block is initialised with a variable array size of 1.
				// In constructor for ShaderStorageBlock we query GL_BUFFER_DATA_SIZE stipulating:
				// "If the final member of an active ShaderStorageBlock is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element."
				m_point_light_fixed_size = m_point_lights_buffer->m_size - m_point_light_array_stride;

				for (auto& var : m_point_lights_buffer->m_variables)
				{
					if (var.m_identifier == "number_of_point_lights")
					{
						ASSERT(var.m_type == ShaderDataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_point_lights to be a uint.");
						m_point_light_count_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].position")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].position to be a vec3.");
						m_point_light_position_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].constant")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected point_lights[0].constant to be a float.");
						m_point_light_constant_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].linear")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected point_lights[0].linear to be a float.");
						m_point_light_linear_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].quadratic")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected point_lights[0].quadratic to be a float.");
						m_point_light_quadratic_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].ambient")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].ambient to be a vec3.");
						m_point_light_ambient_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].diffuse")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].diffuse to be a vec3.");
						m_point_light_diffuse_offset = var.m_offset;
					}
					else if (var.m_identifier == "point_lights[0].specular")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].specular to be a vec3.");
						m_point_light_specular_offset = var.m_offset;
					}
					else
					{
						ASSERT(false, "[OPENGL][PHONG] Unexpected variable in the point lights buffer.");
					}
				}
			}
			{ // SpotLight
				for (auto& var : m_spot_lights_buffer->m_variables)
				{ // Find the first variable in the spot lights array
					if (var.m_identifier.starts_with("spot_lights[0]"))
					{
						m_spot_light_array_start_offset = var.m_offset;
						// GL_TOP_LEVEL_ARRAY_SIZE is the same for all array members, first is arbitrarily chosen here.
						m_spot_light_array_stride = var.m_top_level_array_stride;

						break;
					}
				}
				ASSERT_THROW(m_spot_light_array_stride != 0, "[OPENGL][PHONG] spot_lights array stride cannot be 0");

				// Assuming here the storage block is initialised with a variable array size of 1.
				// In constructor for ShaderStorageBlock we query GL_BUFFER_DATA_SIZE stipulating:
				// "If the final member of an active ShaderStorageBlock is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element."
				m_spot_light_fixed_size = m_spot_lights_buffer->m_size - m_spot_light_array_stride;

				for (auto& var : m_spot_lights_buffer->m_variables)
				{
					if (var.m_identifier == "number_of_spot_lights")
					{
						ASSERT(var.m_type == ShaderDataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_spot_lights to be a uint.");
						m_spot_light_count_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].position")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].position to be a vec3.");
						m_spot_light_position_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].direction")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].direction to be a vec3.");
						m_spot_light_direction_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].cutoff")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].cutoff to be a float.");
						m_spot_light_cutoff_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].outer_cutoff")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].outer_cutoff to be a float.");
						m_spot_light_outer_cutoff_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].constant")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].constant to be a float.");
						m_spot_light_constant_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].linear")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].linear to be a float.");
						m_spot_light_linear_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].quadratic")
					{
						ASSERT(var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].quadratic to be a float.");
						m_spot_light_quadratic_offset = var.m_offset;
					}
					else if (var.m_identifier == "spot_lights[0].ambient")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].ambient to be a vec3.");
						m_spot_light_ambient_offset = var.m_offset;
			}
					else if (var.m_identifier == "spot_lights[0].diffuse")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].diffuse to be a vec3.");
						m_spot_light_diffuse_offset = var.m_offset;
			}
					else if (var.m_identifier == "spot_lights[0].specular")
					{
						ASSERT(var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].specular to be a vec3.");
						m_spot_light_specular_offset = var.m_offset;
		}
					else
					{
						ASSERT(false, "[OPENGL][PHONG] Unexpected variable in the spot lights buffer.");
			}
			}
			}
		}
	}

	void PhongRenderer::update_light_data(System::Scene& p_scene, const Texture& p_shadow_map)
	{
		m_phong_shader.use();

		{ // Set Directional light shadow data
			p_scene.m_entities.foreach([this, &p_scene](Component::DirectionalLight& p_light)
			{
				m_phong_shader.set_uniform("light_proj_view", p_light.get_view_proj(p_scene.m_bound));
			});

			m_phong_shader.set_uniform("PCF_bias", Component::DirectionalLight::PCF_bias);

			active_texture(2);
			m_phong_shader.set_uniform("shadow_map", 2);
			p_shadow_map.bind();
		}

		{ // Set DirectonalLight buffer data
			GLuint directional_light_count = static_cast<GLuint>(p_scene.m_entities.count_components<Component::DirectionalLight>());

			m_directional_lights_buffer->bind();
			{
				const GLsizeiptr required_size = m_directional_light_fixed_size + (m_directional_light_array_stride * directional_light_count);
				{ // Resize the buffer to accomodate at least the directional_light_count
					if (required_size > m_directional_lights_buffer->m_size)
					{
						auto grow_size = required_size - m_directional_lights_buffer->m_size;

						LOG("[OPENGL][PHONG] DirectionalLight count changed ({}), resized the directional light buffer to {}B", directional_light_count , required_size);
						buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
						m_directional_lights_buffer->m_size = required_size;
						bind_buffer_range(BufferType::ShaderStorageBuffer, m_directional_lights_buffer->m_binding_point, m_directional_lights_buffer->m_handle, 0, m_directional_lights_buffer->m_size);

						if (m_directional_light_count_offset > m_directional_light_array_start_offset)
						{ // The var is after the variable sized array, update its offset by the growth of the variable-sized-array.
							m_directional_light_count_offset += grow_size;
						}
					}
				}

				buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_count_offset, sizeof(directional_light_count), &directional_light_count);

				GLuint i = 0;
				p_scene.m_entities.foreach([this, &i](Component::DirectionalLight& p_directional_light)
				{
					const glm::vec3 diffuse  = p_directional_light.m_colour * p_directional_light.m_diffuse_intensity;
					const glm::vec3 ambient  = p_directional_light.m_colour * p_directional_light.m_ambient_intensity;
					const glm::vec3 specular = glm::vec3(p_directional_light.m_specular_intensity);

					buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_direction_offset + (m_directional_light_array_stride * i), sizeof(p_directional_light.m_direction), &p_directional_light.m_direction);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_ambient_offset   + (m_directional_light_array_stride * i), sizeof(ambient),                        &ambient);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_diffuse_offset   + (m_directional_light_array_stride * i), sizeof(diffuse),                        &diffuse);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_specular_offset  + (m_directional_light_array_stride * i), sizeof(specular),                       &specular);

					i++;
				});
			}
		}
		{ // Set PointLight buffer data
			GLuint point_light_count = static_cast<GLuint>(p_scene.m_entities.count_components<Component::PointLight>());
			m_point_lights_buffer->bind();
			{
				const GLsizeiptr required_size = m_point_light_fixed_size + (m_point_light_array_stride * point_light_count);
				{ // Resize the buffer to accomodate at least the point_light_count
					if (required_size > m_point_lights_buffer->m_size)
					{
						auto grow_size = required_size - m_point_lights_buffer->m_size;

						LOG("[OPENGL][PHONG] PointLight count changed ({}), resized the point light buffer to {}B", point_light_count , required_size);
						buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
						m_point_lights_buffer->m_size = required_size;
						bind_buffer_range(BufferType::ShaderStorageBuffer, m_point_lights_buffer->m_binding_point, m_point_lights_buffer->m_handle, 0, m_point_lights_buffer->m_size);

						if (m_point_light_count_offset > m_point_light_array_start_offset)
						{ // The var is after the variable sized array, update its offset by the growth of the variable-sized-array.
							m_point_light_count_offset += grow_size;
						}
					}
				}

				buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_count_offset, sizeof(point_light_count), &point_light_count);

				GLuint i = 0;
				p_scene.m_entities.foreach([this, &i](Component::PointLight& p_point_light)
				{
					const glm::vec3 diffuse  = p_point_light.m_colour * p_point_light.m_diffuse_intensity;
					const glm::vec3 ambient  = p_point_light.m_colour * p_point_light.m_ambient_intensity;
					const glm::vec3 specular = glm::vec3(p_point_light.m_specular_intensity);

					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_position_offset  + (m_point_light_array_stride * i), sizeof(p_point_light.m_position),  &p_point_light.m_position);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_constant_offset  + (m_point_light_array_stride * i), sizeof(p_point_light.m_constant),  &p_point_light.m_constant);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_linear_offset    + (m_point_light_array_stride * i), sizeof(p_point_light.m_linear),    &p_point_light.m_linear);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_quadratic_offset + (m_point_light_array_stride * i), sizeof(p_point_light.m_quadratic), &p_point_light.m_quadratic);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_ambient_offset   + (m_point_light_array_stride * i), sizeof(ambient),  &ambient);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_diffuse_offset   + (m_point_light_array_stride * i), sizeof(diffuse),  &diffuse);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_specular_offset  + (m_point_light_array_stride * i), sizeof(specular), &specular);

					i++;
				});
			}
		}
		{ // Set Spotlight buffer data
			GLuint spot_light_count = static_cast<GLuint>(p_scene.m_entities.count_components<Component::SpotLight>());
			m_spot_lights_buffer->bind();
			{
				const GLsizeiptr required_size = m_spot_light_fixed_size + (m_spot_light_array_stride * spot_light_count);
				{ // Resize the buffer to accomodate at least the spot_light_count
					if (required_size > m_spot_lights_buffer->m_size)
					{
						auto grow_size = required_size - m_spot_lights_buffer->m_size;

						LOG("[OPENGL][PHONG] SpotLight count changed ({}), resized the spot light buffer to {}B", spot_light_count , required_size);
						buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
						m_spot_lights_buffer->m_size = required_size;
						bind_buffer_range(BufferType::ShaderStorageBuffer, m_spot_lights_buffer->m_binding_point, m_spot_lights_buffer->m_handle, 0, m_spot_lights_buffer->m_size);

						if (m_spot_light_count_offset > m_spot_light_array_start_offset)
						{ // The var is after the variable sized array, update its offset by the growth of the variable-sized-array.
							m_spot_light_count_offset += grow_size;
						}
					}
				}

				buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_count_offset, sizeof(spot_light_count), &spot_light_count);

				GLuint i = 0;
				p_scene.m_entities.foreach([this, &i](Component::SpotLight& p_spotlight)
				{
					const glm::vec3 diffuse  = p_spotlight.m_colour * p_spotlight.m_diffuse_intensity;
					const glm::vec3 ambient  = p_spotlight.m_colour * p_spotlight.m_ambient_intensity;
					const glm::vec3 specular = glm::vec3(p_spotlight.m_specular_intensity);

					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_position_offset     + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_position),    &p_spotlight.m_position);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_direction_offset    + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_direction),   &p_spotlight.m_direction);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_cutoff_offset       + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_cutoff),      &p_spotlight.m_cutoff);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_outer_cutoff_offset + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_outer_cutoff), &p_spotlight.m_outer_cutoff);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_constant_offset     + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_constant),    &p_spotlight.m_constant);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_linear_offset       + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_linear),      &p_spotlight.m_linear);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_quadratic_offset    + (m_spot_light_array_stride * i), sizeof(p_spotlight.m_quadratic),   &p_spotlight.m_quadratic);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_ambient_offset      + (m_spot_light_array_stride * i), sizeof(ambient),                  &ambient);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_diffuse_offset      + (m_spot_light_array_stride * i), sizeof(diffuse),                  &diffuse);
					buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_specular_offset     + (m_spot_light_array_stride * i), sizeof(specular),                 &specular);
					i++;
				});
			}
		}
	}
} // namespace OpenGL