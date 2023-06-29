#include "PhongRenderer.hpp"

#include "Component/DirectionalLight.hpp"
#include "Component/PointLight.hpp"
#include "Component/SpotLight.hpp"

#include "ECS/Storage.hpp"

namespace OpenGL
{
    PhongRenderer::PhongRenderer() noexcept
        : m_phong_shader{"phong"}
        , m_directional_lights_buffer{m_phong_shader.get_SSBO_backing("DirectionalLightsBuffer").value()}
        , m_directional_light_fixed_size{0}
        , m_directional_light_count_offset{0}
        , m_directional_light_array_stride{0}
        , m_directional_light_array_start_offset{0}
        , m_directional_light_direction_offset{0}
        , m_directional_light_ambient_offset{0}
        , m_directional_light_diffuse_offset{0}
        , m_directional_light_specular_offset{0}
        , m_point_lights_buffer{m_phong_shader.get_SSBO_backing("PointLightsBuffer").value()}
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
        , m_spot_lights_buffer{m_phong_shader.get_SSBO_backing("SpotLightsBuffer").value()}
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
                ASSERT(m_directional_light_array_stride != 0, "[OPENGL][PHONG] directional_lights array stride cannot be 0"); // always

                // Assuming here the storage block is initialised with a variable array size of 1.
                // In constructor for ShaderStorageBlock we query GL_BUFFER_DATA_SIZE stipulating:
                // "If the final member of an active ShaderStorageBlock is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element."
                m_directional_light_fixed_size = m_directional_lights_buffer->m_size - m_directional_light_array_stride;

                for (auto& var : m_directional_lights_buffer->m_variables)
                {
                    if (var.m_identifier == "number_of_directional_lights")
                    {
                        ASSERT(var.m_type == DataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_directional_lights to be a uint.");
                        m_directional_light_count_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "directional_lights[0].direction")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].direction to be a vec3.");
                        m_directional_light_direction_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "directional_lights[0].ambient")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].ambient to be a vec3.");
                        m_directional_light_ambient_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "directional_lights[0].diffuse")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].diffuse to be a vec3.");
                        m_directional_light_diffuse_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "directional_lights[0].specular")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected directional_lights[0].specular to be a vec3.");
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
                ASSERT(m_point_light_array_stride != 0, "[OPENGL][PHONG] point_lights array stride cannot be 0"); // always

                // Assuming here the storage block is initialised with a variable array size of 1.
                // In constructor for ShaderStorageBlock we query GL_BUFFER_DATA_SIZE stipulating:
                // "If the final member of an active ShaderStorageBlock is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element."
                m_point_light_fixed_size = m_point_lights_buffer->m_size - m_point_light_array_stride;

                for (auto& var : m_point_lights_buffer->m_variables)
                {
                    if (var.m_identifier == "number_of_point_lights")
                    {
                        ASSERT(var.m_type == DataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_point_lights to be a uint.");
                        m_point_light_count_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].position")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].position to be a vec3.");
                        m_point_light_position_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].constant")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected point_lights[0].constant to be a float.");
                        m_point_light_constant_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].linear")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected point_lights[0].linear to be a float.");
                        m_point_light_linear_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].quadratic")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected point_lights[0].quadratic to be a float.");
                        m_point_light_quadratic_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].ambient")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].ambient to be a vec3.");
                        m_point_light_ambient_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].diffuse")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].diffuse to be a vec3.");
                        m_point_light_diffuse_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "point_lights[0].specular")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected point_lights[0].specular to be a vec3.");
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
                ASSERT(m_spot_light_array_stride != 0, "[OPENGL][PHONG] spot_lights array stride cannot be 0"); // always

                // Assuming here the storage block is initialised with a variable array size of 1.
                // In constructor for ShaderStorageBlock we query GL_BUFFER_DATA_SIZE stipulating:
                // "If the final member of an active ShaderStorageBlock is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element."
                m_spot_light_fixed_size = m_spot_lights_buffer->m_size - m_spot_light_array_stride;

                for (auto& var : m_spot_lights_buffer->m_variables)
                {
                    if (var.m_identifier == "number_of_spot_lights")
                    {
                        ASSERT(var.m_type == DataType::UnsignedInt, "[OPENGL][PHONG] Expected number_of_spot_lights to be a uint.");
                        m_spot_light_count_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].position")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].position to be a vec3.");
                        m_spot_light_position_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].direction")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].direction to be a vec3.");
                        m_spot_light_direction_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].cutoff")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].cutoff to be a float.");
                        m_spot_light_cutoff_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].outer_cutoff")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].outer_cutoff to be a float.");
                        m_spot_light_outer_cutoff_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].constant")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].constant to be a float.");
                        m_spot_light_constant_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].linear")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].linear to be a float.");
                        m_spot_light_linear_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].quadratic")
                    {
                        ASSERT(var.m_type == DataType::Float, "[OPENGL][PHONG] Expected spot_lights[0].quadratic to be a float.");
                        m_spot_light_quadratic_offset = var.m_offset;
                    }
                    else if (var.m_identifier == "spot_lights[0].ambient")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].ambient to be a vec3.");
                        m_spot_light_ambient_offset = var.m_offset;
            }
                    else if (var.m_identifier == "spot_lights[0].diffuse")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].diffuse to be a vec3.");
                        m_spot_light_diffuse_offset = var.m_offset;
            }
                    else if (var.m_identifier == "spot_lights[0].specular")
                    {
                        ASSERT(var.m_type == DataType::Vec3, "[OPENGL][PHONG] Expected spot_lights[0].specular to be a vec3.");
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

    void PhongRenderer::update_light_data(ECS::Storage& p_storage)
    {
        { // Set DirectonalLight buffer data
            GLuint directional_light_count = 0;
            p_storage.foreach([this, &directional_light_count](Component::DirectionalLight& p_directional_light) { directional_light_count++; });

            m_directional_lights_buffer->bind();
            {
                const auto required_size = m_directional_light_fixed_size + (m_directional_light_array_stride * directional_light_count);
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
                p_storage.foreach([this, &i](Component::DirectionalLight& p_directional_light)
                {
                    const glm::vec3 diffuse  = p_directional_light.mColour * p_directional_light.mDiffuseIntensity;
                    const glm::vec3 ambient  = p_directional_light.mColour * p_directional_light.mAmbientIntensity;
                    const glm::vec3 specular = glm::vec3(p_directional_light.mSpecularIntensity);

                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_direction_offset + (m_directional_light_array_stride * i), sizeof(p_directional_light.mDirection), &p_directional_light.mDirection);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_ambient_offset   + (m_directional_light_array_stride * i), sizeof(ambient),                        &ambient);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_diffuse_offset   + (m_directional_light_array_stride * i), sizeof(diffuse),                        &diffuse);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_directional_light_specular_offset  + (m_directional_light_array_stride * i), sizeof(specular),                       &specular);

                    i++;
                });
            }
        }
        { // Set PointLight buffer data
            GLuint point_light_count = 0;
            p_storage.foreach([this, &point_light_count](Component::PointLight& p_point_light) { point_light_count++; });

            m_point_lights_buffer->bind();
            {
                const auto required_size = m_point_light_fixed_size + (m_point_light_array_stride * point_light_count);
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
                p_storage.foreach([this, &i](Component::PointLight& p_point_light)
                {
                    const glm::vec3 diffuse  = p_point_light.mColour * p_point_light.mDiffuseIntensity;
                    const glm::vec3 ambient  = p_point_light.mColour * p_point_light.mAmbientIntensity;
                    const glm::vec3 specular = glm::vec3(p_point_light.mSpecularIntensity);

                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_position_offset  + (m_point_light_array_stride * i), sizeof(p_point_light.mPosition), &p_point_light.mPosition);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_constant_offset  + (m_point_light_array_stride * i), sizeof(p_point_light.mConstant), &p_point_light.mConstant);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_linear_offset    + (m_point_light_array_stride * i), sizeof(p_point_light.mLinear), &p_point_light.mLinear);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_quadratic_offset + (m_point_light_array_stride * i), sizeof(p_point_light.mQuadratic), &p_point_light.mQuadratic);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_ambient_offset   + (m_point_light_array_stride * i), sizeof(ambient),  &ambient);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_diffuse_offset   + (m_point_light_array_stride * i), sizeof(diffuse),  &diffuse);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_point_light_specular_offset  + (m_point_light_array_stride * i), sizeof(specular), &specular);

                    i++;
                });
            }
        }
        { // Set Spotlight buffer data
            GLuint spot_light_count = 0;
            p_storage.foreach([this, &spot_light_count](Component::SpotLight& p_spot_light) { spot_light_count++; });

            m_spot_lights_buffer->bind();
            {
                const auto required_size = m_spot_light_fixed_size + (m_spot_light_array_stride * spot_light_count);
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
                p_storage.foreach([this, &i](Component::SpotLight& p_spotlight)
                {
                    const glm::vec3 diffuse  = p_spotlight.mColour * p_spotlight.mDiffuseIntensity;
                    const glm::vec3 ambient  = p_spotlight.mColour * p_spotlight.mAmbientIntensity;
                    const glm::vec3 specular = glm::vec3(p_spotlight.mSpecularIntensity);

                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_position_offset     + (m_spot_light_array_stride * i), sizeof(p_spotlight.mPosition),    &p_spotlight.mPosition);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_direction_offset    + (m_spot_light_array_stride * i), sizeof(p_spotlight.mDirection),   &p_spotlight.mDirection);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_cutoff_offset       + (m_spot_light_array_stride * i), sizeof(p_spotlight.mCutOff),      &p_spotlight.mCutOff);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_outer_cutoff_offset + (m_spot_light_array_stride * i), sizeof(p_spotlight.mOuterCutOff), &p_spotlight.mOuterCutOff);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_constant_offset     + (m_spot_light_array_stride * i), sizeof(p_spotlight.mConstant),    &p_spotlight.mConstant);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_linear_offset       + (m_spot_light_array_stride * i), sizeof(p_spotlight.mLinear),      &p_spotlight.mLinear);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_quadratic_offset    + (m_spot_light_array_stride * i), sizeof(p_spotlight.mQuadratic),   &p_spotlight.mQuadratic);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_ambient_offset      + (m_spot_light_array_stride * i), sizeof(ambient),                  &ambient);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_diffuse_offset      + (m_spot_light_array_stride * i), sizeof(diffuse),                  &diffuse);
                    buffer_sub_data(BufferType::ShaderStorageBuffer, m_spot_light_specular_offset     + (m_spot_light_array_stride * i), sizeof(specular),                 &specular);
                    i++;
                });
            }
        }
    }

    void PhongRenderer::set_draw_data(const glm::vec3& p_view_position, const glm::mat4& p_model, const Texture& p_diffuse_texture, const Texture& p_specular_texture, float p_shininess)
    {
        m_phong_shader.use();
        m_phong_shader.set_uniform("view_position", p_view_position); // #TODO set only once per frame not every draw
        m_phong_shader.set_uniform("model", p_model);
        m_phong_shader.set_uniform("shininess", p_shininess);

        // For both textures, set the texture unit the samplers belong to.
        m_phong_shader.set_uniform("diffuse", 0);
        set_active_texture(0);
        p_diffuse_texture.bind();

        m_phong_shader.set_uniform("specular", 1);
        set_active_texture(1);
        p_specular_texture.bind();
    }
} // namespace OpenGL