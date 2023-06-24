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
        , m_directional_light_array_stride{0}
        , m_directional_light_array_start_offset{0}
        , m_directional_light_direction_offset{0}
        , m_directional_light_ambient_offset{0}
        , m_directional_light_diffuse_offset{0}
        , m_directional_light_specular_offset{0}
        , m_point_lights_buffer{m_phong_shader.get_SSBO_backing("PointLightsBuffer").value()}
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
        {// PhongRenderer makes a few assumptions about the layout of the buffers in order to cache offsets. Make sure these are correct here
            { // DirectionalLight asserts
                ASSERT(m_directional_lights_buffer->m_variables.size() == 5, "[OPENGL][PHONG] Expected 5 variables in DirectionalLightsBuffer");

                ASSERT(m_directional_lights_buffer->m_variables[0].m_identifier == "number_of_directional_lights"
                    && assert_type<GLuint>(m_directional_lights_buffer->m_variables[0].m_type)
                    && m_directional_lights_buffer->m_variables[0].m_offset == 0,
                    "[OPENGL][PHONG] Expected DirectionalLightsBuffer variable 1 tp be uint number_of_directional_lights at offset 0");

                ASSERT(m_directional_lights_buffer->m_variables[1].m_identifier == "directional_lights[0].direction"
                    && assert_type<glm::vec3>(m_directional_lights_buffer->m_variables[1].m_type),
                    "[OPENGL][PHONG] Expected DirectionalLightsBuffer variable 2 to be vec3 directional_lights[0].direction");

                ASSERT(m_directional_lights_buffer->m_variables[2].m_identifier == "directional_lights[0].ambient"
                    && assert_type<glm::vec3>(m_directional_lights_buffer->m_variables[2].m_type),
                    "[OPENGL][PHONG] Expected DirectionalLightsBuffer variable 2 to be vec3 directional_lights[0].ambient");

                ASSERT(m_directional_lights_buffer->m_variables[3].m_identifier == "directional_lights[0].diffuse"
                    && assert_type<glm::vec3>(m_directional_lights_buffer->m_variables[3].m_type),
                    "[OPENGL][PHONG] Expected DirectionalLightsBuffer variable 3 to be vec3 directional_lights[0].diffuse");

                ASSERT(m_directional_lights_buffer->m_variables[4].m_identifier == "directional_lights[0].specular"
                    && assert_type<glm::vec3>(m_directional_lights_buffer->m_variables[4].m_type),
                    "[OPENGL][PHONG] Expected DirectionalLightsBuffer variable 4 to be vec3 directional_lights[0].specular");
            }
            { // PointLight asserts
                ASSERT(m_point_lights_buffer->m_variables.size() == 8, "[OPENGL][PHONG] Expected 8 variables in PointLightsBuffer");

                ASSERT(m_point_lights_buffer->m_variables[0].m_identifier == "number_of_point_lights"
                    && assert_type<GLuint>(m_point_lights_buffer->m_variables[0].m_type)
                    && m_point_lights_buffer->m_variables[0].m_offset == 0,
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 1 variable be uint number_of_point_lights at offset 0");

                ASSERT(m_point_lights_buffer->m_variables[1].m_identifier == "point_lights[0].position"
                    && assert_type<glm::vec3>(m_point_lights_buffer->m_variables[1].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 2 to be vec3 point_lights[0].position");

                ASSERT(m_point_lights_buffer->m_variables[2].m_identifier == "point_lights[0].constant"
                    && assert_type<float>(m_point_lights_buffer->m_variables[2].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 3 to be float point_lights[0].constant");

                ASSERT(m_point_lights_buffer->m_variables[3].m_identifier == "point_lights[0].linear"
                    && assert_type<float>(m_point_lights_buffer->m_variables[3].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 4 to be float point_lights[0].linear");

                ASSERT(m_point_lights_buffer->m_variables[4].m_identifier == "point_lights[0].quadratic"
                    && assert_type<float>(m_point_lights_buffer->m_variables[4].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 5 to be float point_lights[0].quadratic");

                ASSERT(m_point_lights_buffer->m_variables[5].m_identifier == "point_lights[0].ambient"
                    && assert_type<glm::vec3>(m_point_lights_buffer->m_variables[5].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 6 to be vec3 point_lights[0].ambient");

                ASSERT(m_point_lights_buffer->m_variables[6].m_identifier == "point_lights[0].diffuse"
                    && assert_type<glm::vec3>(m_point_lights_buffer->m_variables[6].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 7 to be vec3 point_lights[0].diffuse");

                ASSERT(m_point_lights_buffer->m_variables[7].m_identifier == "point_lights[0].specular"
                    && assert_type<glm::vec3>(m_point_lights_buffer->m_variables[7].m_type),
                    "[OPENGL][PHONG] Expected PointLightsBuffer variable 8 to be vec3 point_lights[0].specular");
            }
            { // SpotLight asserts
                ASSERT(m_spot_lights_buffer->m_variables.size() == 11, "[OPENGL][PHONG] Expected 8 variables in SpotLightsBuffer");

                ASSERT(m_spot_lights_buffer->m_variables[0].m_identifier == "number_of_spot_lights"
                    && assert_type<GLuint>(m_spot_lights_buffer->m_variables[0].m_type)
                    && m_spot_lights_buffer->m_variables[0].m_offset == 0,
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 1 to be uint number_of_spot_lights at offset 0");

                ASSERT(m_spot_lights_buffer->m_variables[1].m_identifier == "spot_lights[0].position"
                    && assert_type<glm::vec3>(m_spot_lights_buffer->m_variables[1].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 2 to be vec3 spot_lights[0].position");

                ASSERT(m_spot_lights_buffer->m_variables[2].m_identifier == "spot_lights[0].direction"
                    && assert_type<glm::vec3>(m_spot_lights_buffer->m_variables[2].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 3 to be vec3 spot_lights[0].direction");

                ASSERT(m_spot_lights_buffer->m_variables[3].m_identifier == "spot_lights[0].cutoff"
                    && assert_type<float>(m_spot_lights_buffer->m_variables[3].m_type),
                    "[OPENGL][PHONG] Expected SpotLigthsBuffer variable 4 to be float spot_lights[0].cutoff");

                ASSERT(m_spot_lights_buffer->m_variables[4].m_identifier == "spot_lights[0].outer_cutoff"
                    && assert_type<float>(m_spot_lights_buffer->m_variables[4].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 5 to be float spot_lights[0].outer_cutoff");

                ASSERT(m_spot_lights_buffer->m_variables[5].m_identifier == "spot_lights[0].constant"
                    && assert_type<float>(m_spot_lights_buffer->m_variables[5].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 6 to be float spot_lights[0].constant");

                ASSERT(m_spot_lights_buffer->m_variables[6].m_identifier == "spot_lights[0].linear"
                    && assert_type<float>(m_spot_lights_buffer->m_variables[6].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 7 to be float spot_lights[0].linear");

                ASSERT(m_spot_lights_buffer->m_variables[7].m_identifier == "spot_lights[0].quadratic"
                    && assert_type<float>(m_spot_lights_buffer->m_variables[7].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 8 to be float spot_lights[0].quadratic");

                ASSERT(m_spot_lights_buffer->m_variables[8].m_identifier == "spot_lights[0].ambient"
                    && assert_type<glm::vec3>(m_spot_lights_buffer->m_variables[8].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 9 to be vec3 spot_lights[0].ambient");

                ASSERT(m_spot_lights_buffer->m_variables[9].m_identifier == "spot_lights[0].diffuse"
                    && assert_type<glm::vec3>(m_spot_lights_buffer->m_variables[9].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 10 to be vec3 spot_lights[0].diffuse");

                ASSERT(m_spot_lights_buffer->m_variables[10].m_identifier == "spot_lights[0].specular"
                    && assert_type<glm::vec3>(m_spot_lights_buffer->m_variables[10].m_type),
                    "[OPENGL][PHONG] Expected SpotLightsBuffer variable 11 to be vec3 spot_lights[0].specular");
            }
        }

        { // Initialise offsets
            { // DirectionalLight
                // Arbitrary choice of back variable. Any of the array variables would suffice as they share the same m_top_level_array_stride value.
                m_directional_light_array_stride = m_directional_lights_buffer->m_variables.back().m_top_level_array_stride;
                ASSERT(m_directional_light_array_stride != 0, "[OPENGL][PHONG] Array stride cannot be 0"); // always

                m_directional_light_array_start_offset = m_directional_lights_buffer->m_variables[1].m_offset;
                m_directional_light_direction_offset   = m_directional_lights_buffer->m_variables[1].m_offset;
                m_directional_light_ambient_offset     = m_directional_lights_buffer->m_variables[2].m_offset;
                m_directional_light_diffuse_offset     = m_directional_lights_buffer->m_variables[3].m_offset;
                m_directional_light_specular_offset    = m_directional_lights_buffer->m_variables[4].m_offset;
            }
            { // Point lights
                // Arbitrary choice of back variable. Any of the array variables would suffice as they share the same m_top_level_array_stride value.
                m_point_light_array_stride = m_point_lights_buffer->m_variables.back().m_top_level_array_stride;
                ASSERT(m_point_light_array_stride != 0, "[OPENGL][PHONG] Array stride cannot be 0"); // always

                m_point_light_position_offset    = m_point_lights_buffer->m_variables[1].m_offset;
                m_point_light_array_start_offset = m_point_lights_buffer->m_variables[1].m_offset;
                m_point_light_constant_offset    = m_point_lights_buffer->m_variables[2].m_offset;
                m_point_light_linear_offset      = m_point_lights_buffer->m_variables[3].m_offset;
                m_point_light_quadratic_offset   = m_point_lights_buffer->m_variables[4].m_offset;
                m_point_light_ambient_offset     = m_point_lights_buffer->m_variables[5].m_offset;
                m_point_light_diffuse_offset     = m_point_lights_buffer->m_variables[6].m_offset;
                m_point_light_specular_offset    = m_point_lights_buffer->m_variables[7].m_offset;
            }
            { // Spot lights
                // Arbitrary choice of back variable. Any of the array variables would suffice as they share the same m_top_level_array_stride value.
                m_spot_light_array_stride        = m_spot_lights_buffer->m_variables.back().m_top_level_array_stride;
                ASSERT(m_spot_light_array_stride != 0, "[OPENGL][PHONG] Array stride cannot be 0"); // always

                m_spot_light_array_start_offset  = m_spot_lights_buffer->m_variables[1].m_offset;
                m_spot_light_position_offset     = m_spot_lights_buffer->m_variables[1].m_offset;
                m_spot_light_direction_offset    = m_spot_lights_buffer->m_variables[2].m_offset;
                m_spot_light_cutoff_offset       = m_spot_lights_buffer->m_variables[3].m_offset;
                m_spot_light_outer_cutoff_offset = m_spot_lights_buffer->m_variables[4].m_offset;
                m_spot_light_constant_offset     = m_spot_lights_buffer->m_variables[5].m_offset;
                m_spot_light_linear_offset       = m_spot_lights_buffer->m_variables[6].m_offset;
                m_spot_light_quadratic_offset    = m_spot_lights_buffer->m_variables[7].m_offset;
                m_spot_light_ambient_offset      = m_spot_lights_buffer->m_variables[8].m_offset;
                m_spot_light_diffuse_offset      = m_spot_lights_buffer->m_variables[9].m_offset;
                m_spot_light_specular_offset     = m_spot_lights_buffer->m_variables[10].m_offset;
            }
        }
    }

    void PhongRenderer::update_light_data(ECS::Storage& p_storage)
    {
        { // Set DirectonalLight buffer data
            GLuint directional_light_count = 0;
            p_storage.foreach([this, &directional_light_count](Component::DirectionalLight& p_directional_light) { directional_light_count++; });

            m_directional_lights_buffer->bind();
            buffer_sub_data(BufferType::ShaderStorageBuffer, 0, sizeof(directional_light_count), &directional_light_count);

            if (directional_light_count > 0)
            {
                { // Resize the buffer to accomodate at least the directional_light_count
                    const auto required_size = m_directional_light_array_start_offset + (m_directional_light_array_stride * directional_light_count);
                    if (required_size > m_directional_lights_buffer->m_size)
                    {
                        LOG("[OPENGL][PHONG] DirectionalLight count changed ({}), resized the directional light buffer to {}B", directional_light_count , required_size);
                        buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
                        m_directional_lights_buffer->m_size = required_size;
                        bind_buffer_range(BufferType::ShaderStorageBuffer, m_directional_lights_buffer->m_binding_point, m_directional_lights_buffer->m_handle, 0, m_directional_lights_buffer->m_size);
                    }
                }

                GLuint i = 0;
                p_storage.foreach([this, &i](Component::DirectionalLight& p_directional_light)
                {
                    const glm::vec3 diffuse  = p_directional_light.mColour * p_directional_light.mDiffuseIntensity;
                    const glm::vec3 ambient  = diffuse * p_directional_light.mAmbientIntensity;
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
            GLuint point_lights_count = 0;
            p_storage.foreach([this, &point_lights_count](Component::PointLight& p_point_light) { point_lights_count++; });

            m_point_lights_buffer->bind();
            buffer_sub_data(BufferType::ShaderStorageBuffer, 0, sizeof(point_lights_count), &point_lights_count);

            if (point_lights_count > 0)
            {
                { // Resize the buffer to accomodate at least the point_lights_count
                    const auto required_size = m_point_light_array_start_offset + (m_point_light_array_stride * point_lights_count);
                    if (required_size > m_point_lights_buffer->m_size)
                    {
                        LOG("[OPENGL][PHONG] Point light count changed ({}), resized the point light buffer to {}B", point_lights_count , required_size);
                        buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
                        m_point_lights_buffer->m_size = required_size;
                        bind_buffer_range(BufferType::ShaderStorageBuffer, m_point_lights_buffer->m_binding_point, m_point_lights_buffer->m_handle, 0, m_point_lights_buffer->m_size);
                    }
                }

                GLuint i = 0;
                p_storage.foreach([this, &i](Component::PointLight& p_point_light)
                {
                    const glm::vec3 diffuse  = p_point_light.mColour * p_point_light.mDiffuseIntensity;
                    const glm::vec3 ambient  = diffuse * p_point_light.mAmbientIntensity;
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
            p_storage.foreach([this, &spot_light_count](Component::SpotLight& p_spotlight) { spot_light_count++; });

            m_spot_lights_buffer->bind();
            buffer_sub_data(BufferType::ShaderStorageBuffer, 0, sizeof(spot_light_count), &spot_light_count);

            if (spot_light_count > 0)
            {
                { // Resize the buffer to accomodate at least the spot_light_count
                    const auto required_size = m_spot_light_array_start_offset + (m_spot_light_array_stride * spot_light_count);
                    if (required_size > m_spot_lights_buffer->m_size)
                    {
                        LOG("[OPENGL][PHONG] Spotlight count changed ({}), resized the spotlight buffer to {}B", spot_light_count , required_size);
                        buffer_data(BufferType::ShaderStorageBuffer, required_size, NULL, BufferUsage::StaticDraw);
                        m_spot_lights_buffer->m_size = required_size;
                        bind_buffer_range(BufferType::ShaderStorageBuffer, m_spot_lights_buffer->m_binding_point, m_spot_lights_buffer->m_handle, 0, m_spot_lights_buffer->m_size);
                    }
                }

                GLuint i = 0;
                p_storage.foreach([this, &i](Component::SpotLight& p_spotlight)
                {
                    const glm::vec3 diffuse  = p_spotlight.mColour * p_spotlight.mDiffuseIntensity;
                    const glm::vec3 ambient  = diffuse * p_spotlight.mAmbientIntensity;
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