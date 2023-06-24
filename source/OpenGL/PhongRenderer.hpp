#pragma once

#include "Shader.hpp"
#include "Types.hpp"

namespace ECS
{
    class Storage;
}

namespace OpenGL
{
    class PhongRenderer
    {
        Shader m_phong_shader; // Used to fetch the Directional, Point and spot light buffers.

        Utility::ResourceRef<SSBO> m_directional_lights_buffer; // The SSBO used across shaders to bind DirectionalLight data.
        GLint m_directional_light_array_stride;
        GLint m_directional_light_array_start_offset;
        GLint m_directional_light_direction_offset;
        GLint m_directional_light_ambient_offset;
        GLint m_directional_light_diffuse_offset;
        GLint m_directional_light_specular_offset;

        Utility::ResourceRef<SSBO> m_point_lights_buffer; // The SSBO used across shaders to bind PointLight data.
        GLint m_point_light_array_stride;
        GLint m_point_light_array_start_offset;
        GLint m_point_light_position_offset;
        GLint m_point_light_constant_offset;
        GLint m_point_light_linear_offset;
        GLint m_point_light_quadratic_offset;
        GLint m_point_light_ambient_offset;
        GLint m_point_light_diffuse_offset;
        GLint m_point_light_specular_offset;

        Utility::ResourceRef<SSBO> m_spot_lights_buffer; // The SSBO used across shaders to bind SpotLight data.
        GLint m_spot_light_array_stride;
        GLint m_spot_light_array_start_offset;
        GLint m_spot_light_position_offset;
        GLint m_spot_light_direction_offset;
        GLint m_spot_light_cutoff_offset;
        GLint m_spot_light_outer_cutoff_offset;
        GLint m_spot_light_constant_offset;
        GLint m_spot_light_linear_offset;
        GLint m_spot_light_quadratic_offset;
        GLint m_spot_light_ambient_offset;
        GLint m_spot_light_diffuse_offset;
        GLint m_spot_light_specular_offset;

    public:
        PhongRenderer() noexcept;

        // Update the storage block buffer object data for all the lights in p_storage.
        // Only needs to happen once per frame or on changes to a light.
        void update_light_data(ECS::Storage& p_storage);

        // Set the uniform and material data related to a specific draw call.
        void set_draw_data(const glm::vec3& p_view_position, const glm::mat4& p_model, const Texture& p_diffuse_texture, const Texture& p_specular_texture, float p_shininess);
    };
} // namespace OpenGL