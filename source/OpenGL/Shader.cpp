#include "Shader.hpp"

#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#include <glad/glad.h>
#include "glm/gtc/type_ptr.hpp"

#include <sstream>

namespace OpenGL
{
	Shader::Shader(const char* p_name)
		: m_name{p_name}
		, m_handle{0}
		, m_uniform_blocks{}
		, m_shader_storage_blocks{}
		, m_uniforms{}
	{
		const auto shaderPath = Config::GLSL_Shader_Directory / p_name;

		unsigned int vertexShader;
		auto vertexShaderPath = shaderPath;
		vertexShaderPath.replace_extension("vert");
		ASSERT(Utility::File::exists(vertexShaderPath), "[OPENGL][SHADER] Vertex shader does not exist at path '{}'", vertexShaderPath.string());
		std::string vertexSource = Utility::File::read_from_file(vertexShaderPath);
		{
			vertexShader = create_shader(ShaderProgramType::Vertex);
			shader_source(vertexShader, vertexSource);
			compile_shader(vertexShader);
		}

		unsigned int fragmentShader;
		{
			auto fragmentShaderPath = shaderPath;
			fragmentShaderPath.replace_extension("frag");
			ASSERT(Utility::File::exists(fragmentShaderPath), "[OPENGL][SHADER] Fragment shader does not exist at path {}", fragmentShaderPath.string());
			fragmentShader     = create_shader(ShaderProgramType::Fragment);
			std::string source = Utility::File::read_from_file(fragmentShaderPath);
			shader_source(fragmentShader, source);
			compile_shader(fragmentShader);
		}

		std::optional<unsigned int> geometryShader;
		{
			auto geomShaderPath = shaderPath;
			geomShaderPath.replace_extension("geom");
			if (Utility::File::exists(shaderPath))
			{
				geometryShader     = create_shader(ShaderProgramType::Geometry);
				std::string source = Utility::File::read_from_file(shaderPath);
				shader_source(geometryShader.value(), source);
				compile_shader(geometryShader.value());
			}
		}
		{
			m_handle = create_program();
			attach_shader(m_handle, vertexShader);
			attach_shader(m_handle, fragmentShader);
			if (geometryShader.has_value())
				attach_shader(m_handle, geometryShader.value());

			link_program(m_handle);
		}

		// After we have linked the program we can query the uniform and shader storage blocks.
		{
			{ // Setup loose uniforms (not belonging to the interface blocks or shader storage blocks)
				const GLint uniform_count = get_uniform_count(m_handle);

				for (int uniform_index = 0; uniform_index < uniform_count; ++uniform_index)
				{
					constexpr GLenum properties[1] = {GL_BLOCK_INDEX};
					GLint values[1];
					glGetProgramResourceiv(m_handle, GL_UNIFORM, uniform_index, 1, properties, 1, NULL, values);

					// If the variable is not the member of an interface block, the value is -1.
					if (values[0] == -1)
						m_uniforms.emplace_back(m_handle, uniform_index, Variable::Type::Uniform);
				}
			}
			{ // m_uniform_blocks setup
				const GLint block_count = get_uniform_block_count(m_handle);
				for (int block_index = 0; block_index < block_count; block_index++)
					m_uniform_blocks.emplace_back(m_handle, block_index, InterfaceBlock::Type::UniformBlock);
			}
			{ // m_shader_storage_blocks
				const GLint block_count = get_shader_storage_block_count(m_handle);
				m_shader_storage_blocks.reserve(block_count);
				for (int block_index = 0; block_index < block_count; block_index++)
					m_shader_storage_blocks.emplace_back(m_handle, block_index, InterfaceBlock::Type::ShaderStorageBlock);
			}
		}

		// Delete the shaders after linking as they're no longer needed, they will be flagged for deletion,
		// but will not be deleted until they are no longer attached to any shader program object.
		delete_shader(vertexShader);
		delete_shader(fragmentShader);
		if (geometryShader.has_value())
			delete_shader(geometryShader.value());

		LOG("OpenGL::Shader '{}' loaded given ID: {}", m_name, m_handle);
	}
	Variable::Variable(GLHandle p_shader_program, GLuint p_uniform_index, Type p_type)
		: m_identifier{""}
		, m_type{ShaderDataType::Unknown}
		, m_variable_type{p_type}
		, m_offset{-1}
		, m_array_size{-1}
		, m_array_stride{-1}
		, m_matrix_stride{-1}
		, m_is_row_major{false}
		, m_location{-1}
		, m_top_level_array_size{-1}
		, m_top_level_array_stride{-1}
	{
		GLenum type_query = [p_type]
		{
			switch (p_type)
			{
				case Type::Uniform:            return GL_UNIFORM;
				case Type::UniformBlock:       return GL_UNIFORM;
				case Type::ShaderStorageBlock: return GL_BUFFER_VARIABLE;
				default: ASSERT(false, "Unknown shader variable Type"); return GL_INVALID_ENUM;
			}
		}();

		// Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
		// https://www.khronos.org/opengl/wiki/Program_Introspection
		static constexpr GLenum property_query[7] = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR};
		GLint property_values[7] = {-1};
		glGetProgramResourceiv(p_shader_program, type_query, p_uniform_index, 7, &property_query[0], 7, NULL, &property_values[0]);

		m_identifier.resize(property_values[0]);
		glGetProgramResourceName(p_shader_program, type_query, p_uniform_index, property_values[0], NULL, m_identifier.data());
		ASSERT(!m_identifier.empty(), "Failed to get name of the interface block variable in shader with handle {}", p_shader_program);
		m_identifier.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

		m_type          = convert(property_values[1]);
		m_offset        = property_values[2];
		m_array_size    = property_values[3];
		m_array_stride  = property_values[4];
		m_matrix_stride = property_values[5];
		m_is_row_major  = property_values[6];

		if (type_query == GL_UNIFORM) // GL_LOCATION is only valid for Uniforms
		{
			GLenum location_query = GL_LOCATION;
			glGetProgramResourceiv(p_shader_program, GL_UNIFORM, p_uniform_index, 1, &location_query, 1, NULL, &m_location);
		}
		else if (type_query == GL_BUFFER_VARIABLE) // GL_TOP_LEVEL_ARRAY_SIZE and GL_TOP_LEVEL_ARRAY_STRIDE are only valid for GL_BUFFER_VARIABLE
		{
			GLenum buffer_var_query[2] = {GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE};
			GLint buffer_var_vals[2] = {-1};
			glGetProgramResourceiv(p_shader_program, GL_BUFFER_VARIABLE, p_uniform_index, 2, &buffer_var_query[0], 2, NULL, &buffer_var_vals[0]);

			m_top_level_array_size   = buffer_var_vals[0];
			m_top_level_array_stride = buffer_var_vals[1];
		}
	}
	InterfaceBlock::InterfaceBlock(GLHandle p_shader_program, GLuint p_block_index, Type p_type)
	: m_identifier{""}
	, m_variables{}
	, m_block_index{p_block_index}
	, m_type{p_type}
	, m_data_size{-1}
	, m_binding_point{0}
	{
		GLenum block_type_query = [p_type]
		{
			switch (p_type)
			{
				case Type::UniformBlock:       return GL_UNIFORM_BLOCK;
				case Type::ShaderStorageBlock: return GL_SHADER_STORAGE_BLOCK;
				default: ASSERT(false, "Unknown interface block Type"); return GL_INVALID_ENUM;
			}
		}();

		static constexpr size_t Property_Count = 4;
		static constexpr GLenum property_query[Property_Count] = {GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
		GLint property_values[Property_Count] = {-1};
		glGetProgramResourceiv(p_shader_program, block_type_query, m_block_index, Property_Count, &property_query[0], Property_Count, NULL, &property_values[0]);

		m_identifier.resize(property_values[0]);
		glGetProgramResourceName(p_shader_program, block_type_query, m_block_index, property_values[0], NULL, m_identifier.data());
		ASSERT(!m_identifier.empty(), "Failed to get name of the interface block in shader with handle {}", p_shader_program);
		m_identifier.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

		m_binding_point = property_values[2];
		m_data_size     = property_values[3];
		const GLint active_variables_count = property_values[1];
		if (active_variables_count > 0)
		{
			// Get the array of active variable indices associated with the the interface block. (GL_ACTIVE_VARIABLES)
			// The indices correspond in size to GL_NUM_ACTIVE_VARIABLES
			std::vector<GLint> variable_indices(active_variables_count);
			static constexpr GLenum active_variable_query[1] = {GL_ACTIVE_VARIABLES};
			glGetProgramResourceiv(p_shader_program, block_type_query, m_block_index, 1, active_variable_query, active_variables_count, NULL, &variable_indices[0]);

			Variable::Type variable_type = [p_type]
			{
				switch (p_type)
				{
					case Type::UniformBlock:       return Variable::Type::UniformBlock;
					case Type::ShaderStorageBlock: return Variable::Type::ShaderStorageBlock;
					default: ASSERT(false, "Unknown interface block Type"); return Variable::Type::UniformBlock;
				}
			}();

			m_variables.reserve(active_variables_count);
			for (GLint variable_index : variable_indices)
				m_variables.emplace_back(p_shader_program, static_cast<GLuint>(variable_index), variable_type);
		}
		ASSERT(m_variables.size() == (size_t)active_variables_count, "Failed to retrieve all the UniformBlockVariables of block '{}'", m_identifier);
	}

	void Shader::set_uniform(const char* p_identifier, bool p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1i(location, (GLint)p_value); // Setting a boolean is treated as integer
	}
	void Shader::set_uniform(const char* p_identifier, int p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1i(location, (GLint)p_value);
	}
	void Shader::set_uniform(const char* p_identifier, float p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1f(location, p_value);
	}
	void Shader::set_uniform(const char* p_identifier, const glm::vec2& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform2fv(location, 1, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::vec3& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform3fv(location, 1, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::vec4& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform4fv(location, 1, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::mat2& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::mat3& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::mat4& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(p_value));
	}

	void Shader::bind_sampler_2D(const char* p_identifier, GLuint p_texture_binding)
	{
		GLint index = static_cast<GLint>(p_texture_binding);
		set_uniform(p_identifier, index);
	}
	void Shader::bind_uniform_block(const char* p_identifier, GLuint p_uniform_block_binding)
	{
		auto& block = get_uniform_block(p_identifier);
		if (block.m_binding_point == p_uniform_block_binding)
			return;

		glUniformBlockBinding(m_handle, block.m_block_index, p_uniform_block_binding);
		block.m_binding_point = p_uniform_block_binding;
	}
	void Shader::bind_shader_storage_block(const char* p_identifier, GLuint p_storage_block_binding)
	{
		auto& block = get_shader_storage_block(p_identifier);
		if (block.m_binding_point == p_storage_block_binding)
			return;

		glShaderStorageBlockBinding(m_handle, block.m_block_index, p_storage_block_binding);
		block.m_binding_point = p_storage_block_binding;
	}
	GLuint Shader::get_attribute_index(const char* attribute_identifier) const
	{
		// queries the previously linked program object specified by program for the attribute variable specified by name and returns the index of the
		// generic vertex attribute that is bound to that attribute variable.
		// If name is a matrix attribute variable, the index of the first column of the matrix is returned.
		// If the named attribute variable is not an active attribute in the specified program a value of -1 is returned.
		GLint index = glGetAttribLocation(m_handle, attribute_identifier);
		ASSERT_THROW(index != -1, "Attribute '{}' not found in shader '{}'", attribute_identifier, m_name);
		return static_cast<GLuint>(index);
	}
	const Variable& Shader::get_uniform_variable(const char* p_identifier) const
	{
		auto it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [p_identifier](const auto& uniform)
			{ return uniform.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_uniforms.end(), "Uniform '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	const InterfaceBlock& Shader::get_uniform_block(const char* p_identifier) const
	{
		auto it = std::find_if(m_uniform_blocks.begin(), m_uniform_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_uniform_blocks.end(), "UniformBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	InterfaceBlock& Shader::get_uniform_block(const char* p_identifier)
	{
		auto it = std::find_if(m_uniform_blocks.begin(), m_uniform_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_uniform_blocks.end(), "UniformBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	const Variable& Shader::get_uniform_block_variable(const char* p_block_identifier, const char* p_variable_identifier) const
	{
		const InterfaceBlock& block = get_uniform_block(p_block_identifier);
		auto it = std::find_if(block.m_variables.begin(), block.m_variables.end(), [p_variable_identifier](const auto& variable)
			{ return variable.m_identifier == p_variable_identifier; });

		ASSERT_THROW(it != block.m_variables.end(), "UniformBlockVariable '{}' not found in UniformBlock '{}' in shader '{}'", p_variable_identifier, p_block_identifier, m_name);
		return *it;
	}
	const InterfaceBlock& Shader::get_shader_storage_block(const char* p_identifier) const
	{
		auto it = std::find_if(m_shader_storage_blocks.begin(), m_shader_storage_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_shader_storage_blocks.end(), "ShaderStorageBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	InterfaceBlock& Shader::get_shader_storage_block(const char* p_identifier)
	{
		auto it = std::find_if(m_shader_storage_blocks.begin(), m_shader_storage_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_shader_storage_blocks.end(), "ShaderStorageBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	const Variable& Shader::get_shader_storage_block_variable(const char* p_block_identifier, const char* p_variable_identifier) const
	{
		const InterfaceBlock& block = get_shader_storage_block(p_block_identifier);
		auto it = std::find_if(block.m_variables.begin(), block.m_variables.end(), [p_variable_identifier](const auto& variable)
			{ return variable.m_identifier == p_variable_identifier; });

		ASSERT_THROW(it != block.m_variables.end(), "ShaderStorageBlockVariable '{}' not found in ShaderStorageBlock '{}' in shader '{}'", p_variable_identifier, p_block_identifier, m_name);
		return *it;
	}


} // namespace OpenGL