#pragma once

#include "GLState.hpp"
#include "Types.hpp"

#include "Logger.hpp"

#include <string>
#include <vector>

namespace OpenGL
{
    // Handles the loading of GLSL shaders from file.
    // Provides set_uniform for setting the GLSL uniform variables.
    // m_vertex_attributes stores the VertexAttributes the shader depends on.
    class Shader
    {
        static inline const size_t maxTextureUnits = 2; // The limit on the number of texture units available in the shaders

        std::string m_name;
        GLHandle m_handle;
        int m_texture_units; // The number of available textures to the shader. Found in shader file as 'uniform sampler2D textureX'

        std::vector<VertexAttribute> m_vertex_attributes;        // List vertex attributes shader program depends on.
        std::vector<UniformBlock> m_uniform_blocks;              // Interface blocks containing UniformBlockVariables. Settings these is global if the block is marked 'shared'.
        std::vector<ShaderStorageBlock> m_shader_storage_blocks; // Interface blocks containing ShaderStorageBlockVariables. Settings these is global if the block is marked 'shared'.
        std::vector<UniformBlockVariable> m_uniforms;            // Loose uniforms, setting these is per shader program.

        // Search source code for any per-vertex attributes a Mesh will require to be drawn by this shader.
        void scanForAttributes(const std::string& p_source_code);
        // Implementations to set the uniform variables belonging to this shader.
        // The client facing set_uniform is templated and calls one of these.
        void set_uniform(GLint p_location, bool p_value);
        void set_uniform(GLint p_location, int p_value);
        void set_uniform(GLint p_location, float p_value);
        void set_uniform(GLint p_location, const glm::vec2& p_value);
        void set_uniform(GLint p_location, const glm::vec3& p_value);
        void set_uniform(GLint p_location, const glm::vec4& p_value);
        void set_uniform(GLint p_location, const glm::mat2& p_value);
        void set_uniform(GLint p_location, const glm::mat3& p_value);
        void set_uniform(GLint p_location, const glm::mat4& p_value);

    public:

        Shader(const char* p_name);
        void use() const; // Set this shader as the currently active one in OpenGL state.

        // Set the data for a loose-uniform in this shader program. Call Shader::use() before set_uniform.
        template<typename T>
        inline void set_uniform(const char* p_name, const T& p_data)
        {
            ASSERT(get_current_shader_program() == m_handle, "Calling set uniform without calling Shader::use() first", p_name, m_name);

            auto it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [&p_name](const auto& p_variable){ return p_variable.m_name == p_name; });
            ASSERT(it != m_uniforms.end(),     "[OPENGL][SHADER] Could not find uniform variable '{}' in {} shader", p_name, m_name);
            ASSERT(assert_type<T>(it->m_type), "[OPENGL][SHADER] set uniform data type missmatch!");
            set_uniform(it->m_location, p_data);
        }

        // Set the value for a variable in a UniformBlock. If the UniformBlock is shared this sets the variable in all Shader programs using the block.
        template<typename T>
        static inline void set_block_uniform(const char* p_name, const T& p_data)
        {
            UniformBlock::uniform_block_binding_points.for_each([&p_name, &p_data](const UBO& p_UBO)
            {
                for (const auto& variable : p_UBO.m_variables)
                {
                    if (variable.m_name == p_name)
                    {
                        ASSERT(assert_type<T>(variable.m_type), "[OPENGL][SHADER] set uniform block data type missmatch!");

                        p_UBO.bind();
                        buffer_sub_data(BufferType::UniformBuffer, variable.m_offset, sizeof(T), &p_data);
                        return;
                    }
                }

                ASSERT(false, "[OPENGL][SHADER] Could not find UniformBlock variable '{}' in any shader buffer backings", p_name);
            });
        }
        // Set the value for a variable in a Shader storage block. If the Shader storage block is shared this sets the variable in all Shader programs using the block.
        template<typename T>
        static inline void set_shader_block_uniform(const char* p_name, const T& p_data)
        {
            ShaderStorageBlock::s_shader_storage_block_binding_points.for_each([&p_name, &p_data](const SSBO& p_SSBO)
            {
                for (const auto& variable : p_SSBO.m_variables)
                {
                    if (variable.m_identifier == p_name)
                    {
                        ASSERT(assert_type<T>(variable.m_type), "[OPENGL][SHADER] set shader storage block data type missmatch!");

                        p_SSBO.bind();
                        buffer_sub_data(BufferType::ShaderStorageBuffer, variable.m_offset, sizeof(T), &p_data);
                        return;
                    }
                }

                ASSERT(false, "[OPENGL][SHADER] Could not find ShaderStorageBlock variable '{}' in any shader buffer backings", p_name);
            });
        }

        int getTexturesUnitsCount() const { return m_texture_units; };
    };
} // namespace OpenGL