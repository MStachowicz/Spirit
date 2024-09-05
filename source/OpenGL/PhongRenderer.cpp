#include "PhongRenderer.hpp"

#include "Component/Lights.hpp"

#include "ECS/Storage.hpp"
#include "System/SceneSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

namespace OpenGL
{
	PhongRenderer::PhongRenderer()
		: m_phong_texture{"phong-texture"}
		, m_phong_uniform_colour{"phong-uColour"}
		, m_directional_lights_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, m_directional_light_fixed_size{0}
		, m_directional_light_count_offset{0}
		, m_directional_light_array_stride{0}
		, m_directional_light_direction_offset{0}
		, m_directional_light_ambient_offset{0}
		, m_directional_light_diffuse_offset{0}
		, m_directional_light_specular_offset{0}
		, m_point_lights_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, m_point_light_fixed_size{0}
		, m_point_light_count_offset{0}
		, m_point_light_array_stride{0}
		, m_point_light_position_offset{0}
		, m_point_light_constant_offset{0}
		, m_point_light_linear_offset{0}
		, m_point_light_quadratic_offset{0}
		, m_point_light_ambient_offset{0}
		, m_point_light_diffuse_offset{0}
		, m_point_light_specular_offset{0}
		, m_spot_lights_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, m_spot_light_fixed_size{0}
		, m_spot_light_count_offset{0}
		, m_spot_light_array_stride{0}
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
		auto get_block_array_stride = [](const InterfaceBlock& p_block, const char* p_block_array_identifier)
		{
			// All block array members have the same stride, so we can just query the first one.
			auto it = std::find_if(p_block.m_variables.begin(), p_block.m_variables.end(), [p_block_array_identifier](const auto& var)
				{ return var.m_identifier.starts_with(p_block_array_identifier); });
			ASSERT_THROW(it != p_block.m_variables.end(), "[OPENGL][PHONG] Could not find the variable in the block belonging to the array.");
			return it->m_top_level_array_stride;
		};

		// TODO: Write an assertion that checks the ShaderStorageBlocks of the phong shaders match each other.
		//ASSERT(m_phong_texture.get_shader_storage_block("DirectionalLight") == m_phong_uniform_colour.get_shader_storage_block("DirectionalLight"), "[OPENGL][PHONG] DirectionalLight storage blocks do not match between the phong shaders.");
		//ASSERT(m_phong_texture.get_shader_storage_block("PointLight") == m_phong_uniform_colour.get_shader_storage_block("PointLight"), "[OPENGL][PHONG] PointLight storage blocks do not match between the phong shaders.");
		//ASSERT(m_phong_texture.get_shader_storage_block("SpotLight") == m_phong_uniform_colour.get_shader_storage_block("SpotLight"), "[OPENGL][PHONG] SpotLight storage blocks do not match between the phong shaders.");


		{ // Initialise offsets
			{ // DirectionalLight
				const auto& directional_light_block = m_phong_texture.get_shader_storage_block("DirectionalLightsBuffer");

				m_directional_light_array_stride = get_block_array_stride(directional_light_block, "directional_lights[0]");
				// Fixed portion of a shader storage block is the GL_BUFFER_DATA_SIZE (total size) minus GL_TOP_LEVEL_ARRAY_STRIDE (size of the variable-sized-array).
				m_directional_light_fixed_size = directional_light_block.m_data_size - m_directional_light_array_stride;

				ASSERT(directional_light_block.m_variables.size() == 5, "[OPENGL][PHONG] Expected 5 variables in the DirectionalLight storage block. If shader changed, update this code!");

				const auto& directional_light_count_var = directional_light_block.get_variable("number_of_directional_lights");
				m_directional_light_count_offset = directional_light_count_var.m_offset;
				ASSERT(directional_light_count_var.m_type == ShaderDataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_directional_lights to be a uint.");

				const auto& directional_light_direction_var = directional_light_block.get_variable("directional_lights[0].direction");
				m_directional_light_direction_offset = directional_light_direction_var.m_offset;
				ASSERT(directional_light_direction_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].direction to be a vec3.");

				const auto& directional_light_ambient_var = directional_light_block.get_variable("directional_lights[0].ambient");
				m_directional_light_ambient_offset = directional_light_ambient_var.m_offset;
				ASSERT(directional_light_ambient_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].ambient to be a vec3.");

				const auto& directional_light_diffuse_var = directional_light_block.get_variable("directional_lights[0].diffuse");
				m_directional_light_diffuse_offset = directional_light_diffuse_var.m_offset;
				ASSERT(directional_light_diffuse_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].diffuse to be a vec3.");

				const auto& directional_light_specular_var = directional_light_block.get_variable("directional_lights[0].specular");
				m_directional_light_specular_offset = directional_light_specular_var.m_offset;
				ASSERT(directional_light_specular_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].specular to be a vec3.");
			}
			{ // PointLight
				const auto& point_light_block = m_phong_texture.get_shader_storage_block("PointLightsBuffer");

				m_point_light_array_stride = get_block_array_stride(point_light_block, "point_lights[0]");
				// Fixed portion of a shader storage block is the GL_BUFFER_DATA_SIZE (total size) minus GL_TOP_LEVEL_ARRAY_STRIDE (size of the variable-sized-array).
				m_point_light_fixed_size = point_light_block.m_data_size - m_point_light_array_stride;

				ASSERT(point_light_block.m_variables.size() == 8, "[OPENGL][PHONG] Expected 8 variables in the PointLight storage block. If shader changed, update this code!");

				const auto& point_light_count_var = point_light_block.get_variable("number_of_point_lights");
				m_point_light_count_offset = point_light_count_var.m_offset;
				ASSERT(point_light_count_var.m_type == ShaderDataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_point_lights to be a uint.");

				const auto& point_light_position_var = point_light_block.get_variable("point_lights[0].position");
				m_point_light_position_offset = point_light_position_var.m_offset;
				ASSERT(point_light_position_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].position to be a vec3.");

				const auto& point_light_constant_var = point_light_block.get_variable("point_lights[0].constant");
				m_point_light_constant_offset = point_light_constant_var.m_offset;
				ASSERT(point_light_constant_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected point_lights[0].constant to be a float.");

				const auto& point_light_linear_var = point_light_block.get_variable("point_lights[0].linear");
				m_point_light_linear_offset = point_light_linear_var.m_offset;
				ASSERT(point_light_linear_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected point_lights[0].linear to be a float.");

				const auto& point_light_quadratic_var = point_light_block.get_variable("point_lights[0].quadratic");
				m_point_light_quadratic_offset = point_light_quadratic_var.m_offset;
				ASSERT(point_light_quadratic_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected point_lights[0].quadratic to be a float.");

				const auto& point_light_ambient_var = point_light_block.get_variable("point_lights[0].ambient");
				m_point_light_ambient_offset = point_light_ambient_var.m_offset;
				ASSERT(point_light_ambient_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].ambient to be a vec3.");

				const auto& point_light_diffuse_var = point_light_block.get_variable("point_lights[0].diffuse");
				m_point_light_diffuse_offset = point_light_diffuse_var.m_offset;
				ASSERT(point_light_diffuse_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].diffuse to be a vec3.");

				const auto& point_light_specular_var = point_light_block.get_variable("point_lights[0].specular");
				m_point_light_specular_offset = point_light_specular_var.m_offset;
				ASSERT(point_light_specular_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].specular to be a vec3.");
			}
			{ // SpotLight
				const auto& spot_light_block = m_phong_texture.get_shader_storage_block("SpotLightsBuffer");

				m_spot_light_array_stride = get_block_array_stride(spot_light_block, "spot_lights[0]");
				// Fixed portion of a shader storage block is the GL_BUFFER_DATA_SIZE (total size) minus GL_TOP_LEVEL_ARRAY_STRIDE (size of the variable-sized-array).
				m_spot_light_fixed_size = spot_light_block.m_data_size - m_spot_light_array_stride;

				ASSERT(spot_light_block.m_variables.size() == 11, "[OPENGL][PHONG] Expected 11 variables in the SpotLight storage block. If shader changed, update this code!");

				const auto& spot_light_count_var = spot_light_block.get_variable("number_of_spot_lights");
				m_spot_light_count_offset = spot_light_count_var.m_offset;
				ASSERT(spot_light_count_var.m_type == ShaderDataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_spot_lights to be a uint.");

				const auto& spot_light_position_var = spot_light_block.get_variable("spot_lights[0].position");
				m_spot_light_position_offset = spot_light_position_var.m_offset;
				ASSERT(spot_light_position_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].position to be a vec3.");

				const auto& spot_light_direction_var = spot_light_block.get_variable("spot_lights[0].direction");
				m_spot_light_direction_offset = spot_light_direction_var.m_offset;
				ASSERT(spot_light_direction_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].direction to be a vec3.");

				const auto& spot_light_cutoff_var = spot_light_block.get_variable("spot_lights[0].cutoff");
				m_spot_light_cutoff_offset = spot_light_cutoff_var.m_offset;
				ASSERT(spot_light_cutoff_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].cutoff to be a float.");

				const auto& spot_light_outer_cutoff_var = spot_light_block.get_variable("spot_lights[0].outer_cutoff");
				m_spot_light_outer_cutoff_offset = spot_light_outer_cutoff_var.m_offset;
				ASSERT(spot_light_outer_cutoff_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].outer_cutoff to be a float.");

				const auto& spot_light_constant_var = spot_light_block.get_variable("spot_lights[0].constant");
				m_spot_light_constant_offset = spot_light_constant_var.m_offset;
				ASSERT(spot_light_constant_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].constant to be a float.");

				const auto& spot_light_linear_var = spot_light_block.get_variable("spot_lights[0].linear");
				m_spot_light_linear_offset = spot_light_linear_var.m_offset;
				ASSERT(spot_light_linear_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].linear to be a float.");

				const auto& spot_light_quadratic_var = spot_light_block.get_variable("spot_lights[0].quadratic");
				m_spot_light_quadratic_offset = spot_light_quadratic_var.m_offset;
				ASSERT(spot_light_quadratic_var.m_type == ShaderDataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].quadratic to be a float.");

				const auto& spot_light_ambient_var = spot_light_block.get_variable("spot_lights[0].ambient");
				m_spot_light_ambient_offset = spot_light_ambient_var.m_offset;
				ASSERT(spot_light_ambient_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].ambient to be a vec3.");

				const auto& spot_light_diffuse_var = spot_light_block.get_variable("spot_lights[0].diffuse");
				m_spot_light_diffuse_offset = spot_light_diffuse_var.m_offset;
				ASSERT(spot_light_diffuse_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].diffuse to be a vec3.");

				const auto& spot_light_specular_var = spot_light_block.get_variable("spot_lights[0].specular");
				m_spot_light_specular_offset = spot_light_specular_var.m_offset;
				ASSERT(spot_light_specular_var.m_type == ShaderDataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].specular to be a vec3.");
			}
		}
	}

	void PhongRenderer::update_light_data(System::Scene& p_scene)
	{
		{ // Set DirectonalLight buffer data
			GLuint directional_light_count = static_cast<GLuint>(p_scene.m_entities.count_components<Component::DirectionalLight>());
			{
				const GLsizeiptr required_size = m_directional_light_fixed_size + (m_directional_light_array_stride * directional_light_count);
				{ // Resize the buffer to accomodate at least the directional_light_count
					if (required_size > m_directional_lights_buffer.size())
					{
						OpenGL::Buffer new_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}};
						new_buffer.resize(required_size);
						m_directional_lights_buffer = std::move(new_buffer);
						LOG("[OPENGL][PHONG] DirectionalLight count changed ({}), resized the directional light buffer to {}B", directional_light_count, required_size);
					}
				}
				m_directional_lights_buffer.buffer_sub_data(m_directional_light_count_offset, directional_light_count);

				GLuint i = 0;
				p_scene.m_entities.foreach([&](Component::DirectionalLight& p_directional_light)
				{
					const glm::vec3 diffuse  = p_directional_light.m_colour * p_directional_light.m_diffuse_intensity;
					const glm::vec3 ambient  = p_directional_light.m_colour * p_directional_light.m_ambient_intensity;
					const glm::vec3 specular = glm::vec3(p_directional_light.m_specular_intensity);

					m_directional_lights_buffer.buffer_sub_data(m_directional_light_direction_offset + (m_directional_light_array_stride * i), p_directional_light.m_direction);
					m_directional_lights_buffer.buffer_sub_data(m_directional_light_ambient_offset   + (m_directional_light_array_stride * i), ambient);
					m_directional_lights_buffer.buffer_sub_data(m_directional_light_diffuse_offset   + (m_directional_light_array_stride * i), diffuse);
					m_directional_lights_buffer.buffer_sub_data(m_directional_light_specular_offset  + (m_directional_light_array_stride * i), specular);

					i++;
				});
			}
		}
		{ // Set PointLight buffer data
			GLuint point_light_count = static_cast<GLuint>(p_scene.m_entities.count_components<Component::PointLight>());
			{
				const GLsizeiptr required_size = m_point_light_fixed_size + (m_point_light_array_stride * point_light_count);
				{ // Resize the buffer to accomodate at least the point_light_count
					if (required_size > m_point_lights_buffer.size())
					{
						OpenGL::Buffer new_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}};
						new_buffer.resize(required_size);
						m_point_lights_buffer = std::move(new_buffer);
						LOG("[OPENGL][PHONG] PointLight count changed ({}), resized the point light buffer to {}B", point_light_count, required_size);
					}
				}
				m_point_lights_buffer.buffer_sub_data(m_point_light_count_offset, point_light_count);

				GLuint i = 0;
				p_scene.m_entities.foreach([&](Component::PointLight& p_point_light)
				{
					const glm::vec3 diffuse  = p_point_light.m_colour * p_point_light.m_diffuse_intensity;
					const glm::vec3 ambient  = p_point_light.m_colour * p_point_light.m_ambient_intensity;
					const glm::vec3 specular = glm::vec3(p_point_light.m_specular_intensity);

					m_point_lights_buffer.buffer_sub_data(m_point_light_position_offset  + (m_point_light_array_stride * i), p_point_light.m_position);
					m_point_lights_buffer.buffer_sub_data(m_point_light_constant_offset  + (m_point_light_array_stride * i), p_point_light.m_constant);
					m_point_lights_buffer.buffer_sub_data(m_point_light_linear_offset    + (m_point_light_array_stride * i), p_point_light.m_linear);
					m_point_lights_buffer.buffer_sub_data(m_point_light_quadratic_offset + (m_point_light_array_stride * i), p_point_light.m_quadratic);
					m_point_lights_buffer.buffer_sub_data(m_point_light_ambient_offset   + (m_point_light_array_stride * i), ambient);
					m_point_lights_buffer.buffer_sub_data(m_point_light_diffuse_offset   + (m_point_light_array_stride * i), diffuse);
					m_point_lights_buffer.buffer_sub_data(m_point_light_specular_offset  + (m_point_light_array_stride * i), specular);

					i++;
				});
			}
		}
		{ // Set Spotlight buffer data
			GLuint spot_light_count = static_cast<GLuint>(p_scene.m_entities.count_components<Component::SpotLight>());
			{
				const GLsizeiptr required_size = m_spot_light_fixed_size + (m_spot_light_array_stride * spot_light_count);
				{ // Resize the buffer to accomodate at least the spot_light_count
					if (required_size > m_spot_lights_buffer.size())
					{
						OpenGL::Buffer new_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}};
						new_buffer.resize(required_size);
						m_spot_lights_buffer = std::move(new_buffer);
						LOG("[OPENGL][PHONG] SpotLight count changed ({}), resized the spot light buffer to {}B", spot_light_count, required_size);
					}
				}

				m_spot_lights_buffer.buffer_sub_data(m_spot_light_count_offset, spot_light_count);

				GLuint i = 0;
				p_scene.m_entities.foreach([&](Component::SpotLight& p_spotlight)
				{
					const glm::vec3 diffuse  = p_spotlight.m_colour * p_spotlight.m_diffuse_intensity;
					const glm::vec3 ambient  = p_spotlight.m_colour * p_spotlight.m_ambient_intensity;
					const glm::vec3 specular = glm::vec3(p_spotlight.m_specular_intensity);

					m_spot_lights_buffer.buffer_sub_data(m_spot_light_position_offset     + (m_spot_light_array_stride * i), p_spotlight.m_position);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_direction_offset    + (m_spot_light_array_stride * i), p_spotlight.m_direction);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_cutoff_offset       + (m_spot_light_array_stride * i), p_spotlight.m_cutoff);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_outer_cutoff_offset + (m_spot_light_array_stride * i), p_spotlight.m_outer_cutoff);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_constant_offset     + (m_spot_light_array_stride * i), p_spotlight.m_constant);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_linear_offset       + (m_spot_light_array_stride * i), p_spotlight.m_linear);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_quadratic_offset    + (m_spot_light_array_stride * i), p_spotlight.m_quadratic);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_ambient_offset      + (m_spot_light_array_stride * i), ambient);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_diffuse_offset      + (m_spot_light_array_stride * i), diffuse);
					m_spot_lights_buffer.buffer_sub_data(m_spot_light_specular_offset     + (m_spot_light_array_stride * i), specular);
					i++;
				});
			}
		}
	}
	void PhongRenderer::reload_shaders()
	{
		m_phong_texture.reload();
		m_phong_uniform_colour.reload();
	}
} // namespace OpenGL