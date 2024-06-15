#pragma once

#include "GLState.hpp"

#include <string>
#include <vector>

namespace OpenGL
{
	// Variable is a struct that stores the data of a variable within a shader.
	// Variables can be contained withing InterfaceBlocks or be loose uniforms.
	struct Variable
	{
		enum class Type : uint8_t
		{
			Attribute,         // Attribute variables are used to pass per-vertex data to the vertex shader.
			Uniform,           // Loose uniform variables are used to pass data to the shader.
			UniformBlock,      // UniformBlock variables are used for UBOs.
			ShaderStorageBlock // ShaderStorageBlock variables are used for SSBOs.
		};
		Variable(GLHandle p_shader_program, GLuint p_uniform_index, Type p_type);

		std::string m_identifier; // The identifier used for the variable in the GLSL shader.
		ShaderDataType m_type;
		Type m_variable_type; // Whether the variable is an attribute, uniform, uniform block or shader storage block.

		GLint m_offset;        // The byte offset relative to the base of the buffer range.
		GLint m_array_size;    // For array variables the number of active array elements. 0 if not an array.
		GLint m_array_stride;  // Byte different between consecutive elements in an array type. 0 if not an array.
		GLint m_matrix_stride; // Stride between columns of a column-major matrix or rows of a row-major matrix. For non-matrix or array of matrices, 0. For UniformBlock variables -1.
		GLint m_is_row_major;  // whether an active variable is a row-major matrix. For non-matrix variables 0.

		// Loose uniform variable data.
		GLint m_location; // For variables defined with layout qualifier this is the specified location. For non-loose uniform -1.

		// ShaderStorageBlock variable data.

		// Number of active array elements of the top-level shader storage block member.
		// If the top-level block member is not an array, the value is 1.
		// If it is an array with no declared size, the value is 0, assignable size can be found calling array_size().
		GLint m_top_level_array_size;
		// Stride between array elements of the top-level shader storage block member.
		// For arrays, the value written is the difference, in basic machine units, between the offsets of the active variable for consecutive elements in the top-level array.
		// For top-level block members not declared as an array, value is 0.
		GLint m_top_level_array_stride;
	};

	// InterfaceBlocks are GLSL interface blocks which group variables.
	// Blocks declared with the GLSL shared keyword can be used with any program that defines a block with the same elements in the same order.
	// Matching blocks in different shader stages will, when linked into the same program, be presented as a single interface block.
	struct InterfaceBlock
	{
		enum class Type : uint8_t
		{
			UniformBlock,      // UniformBlock is used for UBOs.
			ShaderStorageBlock // ShaderStorageBlock is used for SSBOs.
		};
		InterfaceBlock(GLHandle p_shader_program, GLuint p_block_index, Type p_type);
		const Variable& get_variable(const char* p_identifier) const;

		std::string m_identifier; // Identifier of the block in m_parent_shader_program.
		std::vector<Variable> m_variables; // All the variables this block defines.
		GLuint m_block_index; // Index of the block in its m_parent_shader_program.
		Type m_type;
		// Minimum total buffer object size in basic machine units, required to hold all active variables associated with the block.
		// If the final member the block is array with no declared size (ShaderStorageBlock only), the m_data_size assumes the array was declared as an array with one element.
		GLsizei m_data_size;
		GLuint m_binding_point; // The binding point for the block (in GLSL: layout(binding = x)). UBOs or SSBOs can be bound to the same binding point to use their data.
	};

	// Handles the loading of GLSL shaders from file.
	// Provides set_uniform for setting the GLSL uniform variables.
	// m_vertex_attributes stores the VertexAttributes the shader depends on.
	class Shader
	{
		friend class DrawCall; // DrawCall needs to set the uniforms of the shader before draw.

		std::string m_name;
		GLHandle m_handle;
		std::vector<InterfaceBlock> m_uniform_blocks;
		std::vector<InterfaceBlock> m_shader_storage_blocks;
		std::vector<Variable> m_uniforms;
		bool is_compute_shader;

		// Uniform set functions are used only by the DrawCall class hence are private.
		void set_uniform(const char* p_identifier, bool p_value) const;
		void set_uniform(const char* p_identifier, int p_value) const;
		void set_uniform(const char* p_identifier, float p_value) const;
		void set_uniform(const char* p_identifier, const glm::vec2& p_value) const;
		void set_uniform(const char* p_identifier, const glm::vec3& p_value) const;
		void set_uniform(const char* p_identifier, const glm::vec4& p_value) const;
		void set_uniform(const char* p_identifier, const glm::mat2& p_value) const;
		void set_uniform(const char* p_identifier, const glm::mat3& p_value) const;
		void set_uniform(const char* p_identifier, const glm::mat4& p_value) const;

		void bind_sampler_2D(const char* p_identifier, GLuint p_texture_binding);
		void bind_uniform_block(const char* p_identifier, GLuint p_uniform_block_binding);
		void bind_shader_storage_block(const char* p_identifier, GLuint p_storage_block_binding);

	public:
		Shader(const char* p_name);
		// Get the index of the attribute with the given identifier.
		GLuint get_attribute_index(const char* attribute_identifier) const;

		const Variable& get_uniform_variable(const char* p_identifier) const;

		const InterfaceBlock& get_uniform_block(const char* p_identifier) const;
		InterfaceBlock& get_uniform_block(const char* p_identifier);
		const InterfaceBlock& get_shader_storage_block(const char* p_identifier) const;
		InterfaceBlock& get_shader_storage_block(const char* p_identifier);
	};
} // namespace OpenGL