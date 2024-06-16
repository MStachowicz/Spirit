#include "GLState.hpp"

#include "Utility/Logger.hpp"

#include "glad/glad.h" // OpenGL functions

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLOBAL STATE FUNCTIONS
namespace OpenGL
{
	State::State()
		: write_to_depth_buffer{true}
		, depth_test_enabled{true}
		, depth_test_type{DepthTestType::Less}
		, polygon_offset_enabled{false}
		, polygon_offset_factor{0.0f}
		, polygon_offset_units{0.0f}
		, blending_enabled{false}
		, source_factor{BlendFactorType::SourceAlpha}
		, destination_factor{BlendFactorType::OneMinusSourceAlpha}
		, cull_face_enabled{true}
		, cull_face_type{CullFaceType::Back}
		, front_face_orientation{FrontFaceOrientation::CounterClockwise}
		, polygon_mode{PolygonMode::Fill}
		, viewport_position{0, 0}
		, viewport_size{0, 0}
		, current_bound_shader_program{0}
		, current_bound_VAO{0}
		, current_bound_FBO{0}
		, current_bound_SSBO{std::vector<std::optional<GLHandle>>(get_max_shader_storage_buffer_bindings(), std::nullopt)}
		, current_bound_UBO{std::vector<std::optional<GLHandle>>(get_max_uniform_buffer_bindings(), std::nullopt)}
		, current_bound_texture{std::vector<std::optional<GLHandle>>(get_max_combined_texture_image_units(), std::nullopt)} // #BUG #TODO: Combined is the sum of all texture units for every shader stage. Maybe we want the min of all the stages?
	{
		glDepthMask(write_to_depth_buffer ? GL_TRUE : GL_FALSE);

		if (depth_test_enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (polygon_offset_enabled)
			glEnable(GL_POLYGON_OFFSET_FILL);
		else
			glDisable(GL_POLYGON_OFFSET_FILL);

		if (blending_enabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

		if (cull_face_enabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		glDepthFunc(convert(depth_test_type));
		glPolygonOffset(polygon_offset_factor, polygon_offset_units);
		glBlendFunc(convert(source_factor), convert(destination_factor));
		glCullFace(convert(cull_face_type));
		glFrontFace(convert(front_face_orientation));
		glPolygonMode(GL_FRONT_AND_BACK, convert(polygon_mode));
		glViewport(viewport_position.x, viewport_position.y, viewport_size.x, viewport_size.y);

		glUseProgram(current_bound_shader_program);
		glBindVertexArray(current_bound_VAO);
		glBindFramebuffer(GL_FRAMEBUFFER, current_bound_FBO);
	}
	void State::bind_VAO(GLHandle p_VAO)
	{
		if (current_bound_VAO == p_VAO)
			return;

		glBindVertexArray(p_VAO);
		current_bound_VAO = p_VAO;
	}
	void State::unbind_VAO()
	{
		bind_VAO(0);
	}

	void State::bind_FBO(GLHandle p_FBO)
	{
		if (current_bound_FBO == p_FBO)
			return;

		glBindFramebuffer(GL_FRAMEBUFFER, p_FBO);
		current_bound_FBO = p_FBO;
	}
	void State::unbind_FBO()
	{
		bind_FBO(0);
	}


	void State::bind_shader_storage_buffer(GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size)
	{
		if (current_bound_SSBO[p_index] == p_buffer)
			return;

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, p_index, p_buffer, p_offset, p_size);
		current_bound_SSBO[p_index] = p_buffer;
	}
	void State::bind_uniform_buffer(GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size)
	{
		if (current_bound_UBO[p_index] == p_buffer)
			return;

		glBindBufferRange(GL_UNIFORM_BUFFER, p_index, p_buffer, p_offset, p_size);
		current_bound_UBO[p_index] = p_buffer;
	}
	void State::unbind_buffer(GLHandle p_buffer)
	{
		// Find which target the buffer might be bound to already
		// If the buffer is bound to a target, we need to unbind it from that target.

		for (auto& SSBO : current_bound_SSBO)
		{
			if (SSBO == p_buffer)
				SSBO.reset();
		}
		for (auto& UBO : current_bound_UBO)
		{
			if (UBO == p_buffer)
				UBO.reset();
		}
	}

	void State::bind_texture_unit(GLuint p_texture_unit, GLHandle p_texture)
	{
		if (current_bound_texture[p_texture_unit] == p_texture)
			return;

		glBindTextureUnit(p_texture_unit, p_texture);

		current_bound_texture[p_texture_unit] = p_texture;
	}
	void State::unbind_texture_unit(GLHandle p_texture)
	{
		auto texture_it = std::find(current_bound_texture.begin(), current_bound_texture.end(), p_texture);
		if (texture_it != current_bound_texture.end())
		{
			(*texture_it).reset();
			return;
		}
	}

	void State::use_program(GLHandle p_shader_program)
	{
		current_bound_shader_program = p_shader_program;
		glUseProgram(p_shader_program);
	}
	void State::delete_program(GLHandle p_shader_program)
	{
		if (current_bound_shader_program == p_shader_program)
			current_bound_shader_program = 0;

		glDeleteProgram(p_shader_program);
	}

	void State::set_depth_write(bool p_write_to_depth_buffer)
	{
		if (p_write_to_depth_buffer == write_to_depth_buffer)
			return;

		glDepthMask(p_write_to_depth_buffer ? GL_TRUE : GL_FALSE);
		write_to_depth_buffer = p_write_to_depth_buffer;
	}

	void State::set_depth_test(bool p_depth_test)
	{
		if (p_depth_test == depth_test_enabled)
			return;

		if (p_depth_test)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		depth_test_enabled = p_depth_test;
	}
	void State::set_depth_test_type(DepthTestType p_type)
	{
		if (p_type == depth_test_type)
			return;

		glDepthFunc(convert(p_type));
		depth_test_type = p_type;
	}
	void State::set_polygon_offset(bool p_polygon_offset)
	{
		if (p_polygon_offset == polygon_offset_enabled)
			return;

		if (p_polygon_offset)
			glEnable(GL_POLYGON_OFFSET_FILL);
		else
			glDisable(GL_POLYGON_OFFSET_FILL);
		polygon_offset_enabled = p_polygon_offset;
	}
	void State::set_polygon_offset_factor(GLfloat p_polygon_offset_factor, GLfloat p_polygon_offset_units)
	{
		if (p_polygon_offset_factor != polygon_offset_factor || p_polygon_offset_units != polygon_offset_units)
		{
			glPolygonOffset(p_polygon_offset_factor, p_polygon_offset_units);
			polygon_offset_factor = p_polygon_offset_factor;
			polygon_offset_units  = p_polygon_offset_units;
		}
	}
	void State::set_blending(bool p_blend)
	{
		if (p_blend == blending_enabled)
			return;

		if (p_blend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

		blending_enabled = p_blend;
	}
	void State::set_blend_func(BlendFactorType p_source_factor, BlendFactorType p_destination_factor)
	{
		ASSERT(blending_enabled, "Blending has to be enabled to set blend function.");

		if (p_source_factor != source_factor || p_destination_factor != destination_factor)
		{
			glBlendFunc(convert(p_source_factor), convert(p_destination_factor)); // It is also possible to set individual RGBA factors using glBlendFuncSeparate().
			source_factor      = p_source_factor;
			destination_factor = p_destination_factor;
		}
	}
	void State::set_cull_face(bool p_cull)
	{
		if (p_cull == cull_face_enabled)
			return;

		if (p_cull)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		cull_face_enabled = p_cull;
	}
	void State::set_cull_face_type(CullFaceType p_cull_face_type)
	{
		if (p_cull_face_type == cull_face_type)
			return;

		glCullFace(convert(p_cull_face_type));
		cull_face_type = p_cull_face_type;
	}
	void State::set_front_face_orientation(FrontFaceOrientation p_front_face_orientation)
	{
		if (p_front_face_orientation == front_face_orientation)
			return;

		glFrontFace(convert(p_front_face_orientation));
		front_face_orientation = p_front_face_orientation;
	}
	void State::set_polygon_mode(PolygonMode p_polygon_mode)
	{
		if (p_polygon_mode == polygon_mode)
			return;

		glPolygonMode(GL_FRONT_AND_BACK, convert(p_polygon_mode));
		polygon_mode = p_polygon_mode;
	}
	void State::set_viewport(GLint p_x, GLint p_y, GLsizei p_width, GLsizei p_height)
	{
		if (p_x != viewport_position.x || p_y != viewport_position.y || p_width != viewport_size.x || p_height != viewport_size.y)
		{
			glViewport(p_x, p_y, p_width, p_height);
			viewport_position = { p_x, p_y };
			viewport_size     = { p_width, p_height };
		}
	}
} // namespace OpenGL

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TEXTURE FUNCTIONS
namespace OpenGL
{
	GLint max_texture_size()
	{
		GLint max_texture_size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		return max_texture_size;
	}
	GLint max_3D_texture_size()
	{
		GLint max_3D_texture_size;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max_3D_texture_size);
		return max_3D_texture_size;
	}
	GLint max_cube_map_texture_size()
	{
		GLint max_cube_map_texture_size;
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &max_cube_map_texture_size);
		return max_cube_map_texture_size;
	}
	GLint max_texture_image_units()
	{
		GLint max_texture_image_units;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units);
		return max_texture_image_units;
	}
	GLint max_vertex_texture_image_units()
	{
		GLint max_vertex_texture_image_units;
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &max_vertex_texture_image_units);
		return max_vertex_texture_image_units;
	}
	GLint max_geometry_texture_image_units()
	{
		GLint max_geometry_texture_image_units;
		glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &max_geometry_texture_image_units);
		return max_geometry_texture_image_units;
	}
	GLint max_combined_texture_image_units()
	{
		GLint max_combined_texture_image_units;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_combined_texture_image_units);
		return max_combined_texture_image_units;
	}
	GLint max_array_texture_layers()
	{
		GLint max_array_texture_layers;
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_array_texture_layers);
		return max_array_texture_layers;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DRAW FUNCTIONS
namespace OpenGL
{
	void draw_arrays(PrimitiveMode p_primitive_mode, GLint p_first, GLsizei p_count)
	{
		glDrawArrays(convert(p_primitive_mode), p_first, p_count);
	}
	void draw_arrays_instanced(PrimitiveMode p_primitive_mode, GLint p_first, GLsizei p_array_size, GLsizei p_instance_count)
	{
		glDrawArraysInstanced(convert(p_primitive_mode), p_first, p_array_size, p_instance_count);
	}
	void draw_elements(PrimitiveMode p_primitive_mode, GLsizei p_elements_size)
	{
		glDrawElements(convert(p_primitive_mode), p_elements_size, GL_UNSIGNED_INT, 0);
	}
	void draw_elements_instanced(PrimitiveMode p_primitive_mode, GLsizei p_elements_size, GLsizei p_instance_count)
	{
		glDrawElementsInstanced(convert(p_primitive_mode), p_elements_size, GL_UNSIGNED_INT, 0, p_instance_count);
	}
	void dispatch_compute(GLuint p_num_groups_x, GLuint p_num_groups_y, GLuint p_num_groups_z)
	{
		glDispatchCompute(p_num_groups_x, p_num_groups_y, p_num_groups_z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHADER FUNCTIONS
namespace OpenGL
{
	GLHandle create_program()
	{
		GLHandle handle = glCreateProgram();
		ASSERT(handle != 0, "[OPENGL] Error occurred creating the shader program object");
		return handle;
	}
	void attach_shader(GLHandle p_shader_program, GLHandle p_shader)
	{
		glAttachShader(p_shader_program, p_shader);
	}
	void link_program(GLHandle p_shader_program)
	{
		// If program contains shader objects of type GL_VERTEX_SHADER, and optionally of type GL_GEOMETRY_SHADER, but does not contain shader objects of type GL_FRAGMENT_SHADER,
		// the vertex shader executable will be installed on the programmable vertex processor,
		// the geometry shader executable, if present, will be installed on the programmable geometry processor,
		// but no executable will be installed on the fragment processor.
		// The results of rasterizing primitives with such a program will be UNDEFINED.
		glLinkProgram(p_shader_program);

		GLint success;
		glGetProgramiv(p_shader_program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLint infoLogCharCount = 0;
			glGetShaderiv(p_shader_program, GL_INFO_LOG_LENGTH, &infoLogCharCount);
			std::string infoLog = "";
			infoLog.resize(infoLogCharCount);
			glGetShaderInfoLog(p_shader_program, infoLogCharCount, NULL, infoLog.data());
			infoLog.pop_back();
			ASSERT(false, "[OPENGL] Shader program linking failed\n{}", infoLog);
		}
	}
	GLHandle create_shader(ShaderProgramType p_program_type)
	{
		GLHandle handle = glCreateShader(convert(p_program_type));
		ASSERT(handle != 0, "[OPENGL] Error occurred creating the shader object");
		return handle;
	}
	void delete_shader(GLHandle p_shader)
	{
		glDeleteShader(p_shader);
	}
	void shader_source(GLHandle p_shader, const std::string& p_shader_source)
	{
		const char* programSource = p_shader_source.c_str();
		glShaderSource(p_shader, 1, &programSource, NULL);
	}
	void compile_shader(GLHandle p_shader)
	{
		glCompileShader(p_shader);

		// Compilation status will be set to true if the shader was compiled without errors and is ready for use  and false otherwise.
		// Compilation status can be queried by calling glGetShader with arguments shader and GL_COMPILE_STATUS.

		GLint success;
		glGetShaderiv(p_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLint infoLogCharCount = 0;
			glGetShaderiv(p_shader, GL_INFO_LOG_LENGTH, &infoLogCharCount);
			std::string infoLog = "";
			infoLog.resize(infoLogCharCount);
			glGetShaderInfoLog(p_shader, infoLogCharCount, NULL, infoLog.data());
			infoLog.pop_back();
			ASSERT(false, "[OPENGL] [OPENGL] Shader compilation failed:\n{}", infoLog);
		}
	}
	GLint get_uniform_location(GLHandle p_shader_program, const char* p_name)
	{
		return glGetUniformLocation(p_shader_program, p_name);
	}
	GLint get_uniform_count(GLHandle p_shader_program)
	{
		GLint uniformCount = 0;
		glGetProgramInterfaceiv(p_shader_program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);
		return uniformCount;
	}
	GLint get_uniform_block_count(GLHandle p_shader_program)
	{
		GLint blockCount = 0;
		glGetProgramInterfaceiv(p_shader_program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &blockCount);
		return blockCount;
	}

	GLint get_shader_storage_block_count(GLHandle p_shader_program)
	{
		GLint blockCount = 0;
		glGetProgramInterfaceiv(p_shader_program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &blockCount);
		return blockCount;
	}
	GLint get_max_uniform_buffer_bindings()
	{
		GLint max_uniform_buffer_bindings = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_uniform_buffer_bindings);
		return max_uniform_buffer_bindings;
	}
	GLint get_max_shader_storage_buffer_bindings()
	{
		GLint max_uniform_buffer_bindings = 0;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_uniform_buffer_bindings);
		return max_uniform_buffer_bindings;
	}
	GLint get_max_uniform_block_size()
	{
		GLint size;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
		return size;
	}
	GLint get_max_shader_storage_block_size()
	{
		GLint size;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
		return size;
	}
	GLint get_max_combined_texture_image_units()
	{
		GLint texture_units;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texture_units);
		return texture_units;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA/BUFFER FUNCTIONS
namespace OpenGL
{
	void uniform_block_binding(GLHandle p_shader_program, GLuint p_uniform_block_index, GLuint p_uniform_block_binding)
	{
		glUniformBlockBinding(p_shader_program, p_uniform_block_index, p_uniform_block_binding);
	}
	void shader_storage_block_binding(GLHandle p_shader_program, GLuint p_storage_block_index, GLuint p_storage_block_binding)
	{
		glShaderStorageBlockBinding(p_shader_program, p_storage_block_index, p_storage_block_binding);
	}
	void vertex_array_vertex_buffer(GLHandle p_VAO, GLuint p_binding_index, GLHandle p_buffer, GLintptr p_offset, GLsizei p_stride)
	{
		glVertexArrayVertexBuffer(p_VAO, p_binding_index, p_buffer, p_offset, p_stride);
	}
	void vertex_array_element_buffer(GLHandle p_VAO, GLHandle p_buffer)
	{
		glVertexArrayElementBuffer(p_VAO, p_buffer);
	}

	void named_buffer_storage(GLHandle p_buffer, GLsizeiptr p_size, const void* p_data, BufferStorageBitfield p_flags)
	{
		glNamedBufferStorage(p_buffer, p_size, p_data, p_flags.bitfield);
	}
	void get_named_buffer_sub_data(GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size, void* p_data)
	{
		glGetNamedBufferSubData(p_buffer, p_offset, p_size, p_data);
	}
	void named_buffer_sub_data(GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size, const void* p_data)
	{
		glNamedBufferSubData(p_buffer, p_offset, p_size, p_data);
	}
	void copy_named_buffer_sub_data(GLHandle p_source_buffer, GLHandle p_destination_buffer, GLintptr p_source_offset, GLintptr p_destination_offset, GLsizeiptr p_size)
	{
		glCopyNamedBufferSubData(p_source_buffer, p_destination_buffer, p_source_offset, p_destination_offset, p_size);
	}
	void bind_buffer_range(BufferType p_target, GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size)
	{
		glBindBufferRange(convert(p_target), p_index, p_buffer, p_offset, p_size);
	}
	void memory_barrier(MemoryBarrierBitfield p_barrier_bitfield)
	{
		glMemoryBarrier(p_barrier_bitfield.bitfield);
	}
} // namespace OpenGL

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CONVERSION FUNCS
namespace OpenGL
{
	GLenum convert(BufferType p_buffer_type)
	{
		switch (p_buffer_type)
		{
			case BufferType::ArrayBuffer:                                    return GL_ARRAY_BUFFER;
			case BufferType::AtomicCounterBuffer:                            return GL_ATOMIC_COUNTER_BUFFER;
			case BufferType::CopyReadBuffer:                                 return GL_COPY_READ_BUFFER;
			case BufferType::CopyWriteBuffer:                                return GL_COPY_WRITE_BUFFER;
			case BufferType::DispatchIndirectBuffer:                         return GL_DISPATCH_INDIRECT_BUFFER;
			case BufferType::DrawIndirectBuffer:                             return GL_DRAW_INDIRECT_BUFFER;
			case BufferType::ElementArrayBuffer:                             return GL_ELEMENT_ARRAY_BUFFER;
			case BufferType::PixelPackBuffer:                                return GL_PIXEL_PACK_BUFFER;
			case BufferType::PixelUnpackBuffer:                              return GL_PIXEL_UNPACK_BUFFER;
			case BufferType::QueryBuffer:                                    return GL_QUERY_BUFFER;
			case BufferType::ShaderStorageBuffer:                            return GL_SHADER_STORAGE_BUFFER;
			case BufferType::TextureBuffer:                                  return GL_TEXTURE_BUFFER;
			case BufferType::TransformFeedbackBuffer:                        return GL_TRANSFORM_FEEDBACK_BUFFER;
			case BufferType::UniformBuffer:                                  return GL_UNIFORM_BUFFER;
			default: ASSERT(false, "[OPENGL] Unknown BufferType requested"); return 0;
		}
	}
	GLenum convert(BufferDataType p_data_type)
	{
		switch (p_data_type)
		{
			case BufferDataType::Byte:          return GL_BYTE;
			case BufferDataType::UnsignedByte:  return GL_UNSIGNED_BYTE;
			case BufferDataType::Short:         return GL_SHORT;
			case BufferDataType::UnsignedShort: return GL_UNSIGNED_SHORT;
			case BufferDataType::Int:           return GL_INT;
			case BufferDataType::UnsignedInt:   return GL_UNSIGNED_INT;
			case BufferDataType::Float:         return GL_FLOAT;
			case BufferDataType::Double:        return GL_DOUBLE;
			default: ASSERT_THROW(false, "[OPENGL] Unknown BufferDataType requested");
		}
	}
	GLenum convert(BufferStorageFlag p_flag)
	{
		switch (p_flag)
		{
			case BufferStorageFlag::DynamicStorageBit: return GL_DYNAMIC_STORAGE_BIT;
			case BufferStorageFlag::MapReadBit:        return GL_MAP_READ_BIT;
			case BufferStorageFlag::MapWriteBit:       return GL_MAP_WRITE_BIT;
			case BufferStorageFlag::MapPersistentBit:  return GL_MAP_PERSISTENT_BIT;
			case BufferStorageFlag::MapCoherentBit:    return GL_MAP_COHERENT_BIT;
			case BufferStorageFlag::ClientStorageBit:  return GL_CLIENT_STORAGE_BIT;
			default: ASSERT_THROW(false, "[OPENGL] Unknown BufferStorageFlag requested"); return 0;
		}
	}
	GLenum convert(MemoryBarrierFlag p_barrier_bitfield)
	{
		switch (p_barrier_bitfield)
		{
			case MemoryBarrierFlag::VertexAttribArrayBarrierBit:  return GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
			case MemoryBarrierFlag::ElementArrayBarrierBit:       return GL_ELEMENT_ARRAY_BARRIER_BIT;
			case MemoryBarrierFlag::UniformBarrierBit:            return GL_UNIFORM_BARRIER_BIT;
			case MemoryBarrierFlag::TextureFetchBarrierBit:       return GL_TEXTURE_FETCH_BARRIER_BIT;
			case MemoryBarrierFlag::ShaderImageAccessBarrierBit:  return GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
			case MemoryBarrierFlag::CommandBarrierBit:            return GL_COMMAND_BARRIER_BIT;
			case MemoryBarrierFlag::PixelBufferBarrierBit:        return GL_PIXEL_BUFFER_BARRIER_BIT;
			case MemoryBarrierFlag::TextureUpdateBarrierBit:      return GL_TEXTURE_UPDATE_BARRIER_BIT;
			case MemoryBarrierFlag::BufferUpdateBarrierBit:       return GL_BUFFER_UPDATE_BARRIER_BIT;
			case MemoryBarrierFlag::FramebufferBarrierBit:        return GL_FRAMEBUFFER_BARRIER_BIT;
			case MemoryBarrierFlag::TransformFeedbackBarrierBit:  return GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
			case MemoryBarrierFlag::AtomicCounterBarrierBit:      return GL_ATOMIC_COUNTER_BARRIER_BIT;
			case MemoryBarrierFlag::ShaderStorageBarrierBit:      return GL_SHADER_STORAGE_BARRIER_BIT;
			default: ASSERT_THROW(false, "[OPENGL] Unknown MemoryBarrierFlag requested"); return 0;
		}

	}
	BufferStorageBitfield::BufferStorageBitfield(std::initializer_list<BufferStorageFlag> flags)
		: bitfield(0)
	{
		for (BufferStorageFlag flag : flags)
			bitfield |= convert(flag);
	}

	MemoryBarrierBitfield::MemoryBarrierBitfield(std::initializer_list<MemoryBarrierFlag> flags)
		: bitfield(0)
	{
		for (MemoryBarrierFlag flag : flags)
			bitfield |= convert(flag);
	}


	ShaderDataType convert(GLenum p_data_type)
	{
		switch (p_data_type)
		{
			case GL_FLOAT:                                     return ShaderDataType::Float;
			case GL_FLOAT_VEC2:                                return ShaderDataType::Vec2;
			case GL_FLOAT_VEC3:                                return ShaderDataType::Vec3;
			case GL_FLOAT_VEC4:                                return ShaderDataType::Vec4;
			case GL_DOUBLE:                                    return ShaderDataType::Double;
			case GL_DOUBLE_VEC2:                               return ShaderDataType::DVec2;
			case GL_DOUBLE_VEC3:                               return ShaderDataType::DVec3;
			case GL_DOUBLE_VEC4:                               return ShaderDataType::DVec4;
			case GL_INT:                                       return ShaderDataType::Int;
			case GL_INT_VEC2:                                  return ShaderDataType::IVec2;
			case GL_INT_VEC3:                                  return ShaderDataType::IVec3;
			case GL_INT_VEC4:                                  return ShaderDataType::IVec4;
			case GL_UNSIGNED_INT:                              return ShaderDataType::UnsignedInt;
			case GL_UNSIGNED_INT_VEC2:                         return ShaderDataType::UVec2;
			case GL_UNSIGNED_INT_VEC3:                         return ShaderDataType::UVec3;
			case GL_UNSIGNED_INT_VEC4:                         return ShaderDataType::UVec4;
			case GL_BOOL:                                      return ShaderDataType::Bool;
			case GL_BOOL_VEC2:                                 return ShaderDataType::BVec2;
			case GL_BOOL_VEC3:                                 return ShaderDataType::BVec3;
			case GL_BOOL_VEC4:                                 return ShaderDataType::BVec4;
			case GL_FLOAT_MAT2:                                return ShaderDataType::Mat2;
			case GL_FLOAT_MAT3:                                return ShaderDataType::Mat3;
			case GL_FLOAT_MAT4:                                return ShaderDataType::Mat4;
			case GL_FLOAT_MAT2x3:                              return ShaderDataType::Mat2x3;
			case GL_FLOAT_MAT2x4:                              return ShaderDataType::Mat2x4;
			case GL_FLOAT_MAT3x2:                              return ShaderDataType::Mat3x2;
			case GL_FLOAT_MAT3x4:                              return ShaderDataType::Mat3x4;
			case GL_FLOAT_MAT4x2:                              return ShaderDataType::Mat4x2;
			case GL_FLOAT_MAT4x3:                              return ShaderDataType::Mat4x3;
			case GL_DOUBLE_MAT2:                               return ShaderDataType::Dmat2;
			case GL_DOUBLE_MAT3:                               return ShaderDataType::Dmat3;
			case GL_DOUBLE_MAT4:                               return ShaderDataType::Dmat4;
			case GL_DOUBLE_MAT2x3:                             return ShaderDataType::Dmat2x3;
			case GL_DOUBLE_MAT2x4:                             return ShaderDataType::Dmat2x4;
			case GL_DOUBLE_MAT3x2:                             return ShaderDataType::Dmat3x2;
			case GL_DOUBLE_MAT3x4:                             return ShaderDataType::Dmat3x4;
			case GL_DOUBLE_MAT4x2:                             return ShaderDataType::Dmat4x2;
			case GL_DOUBLE_MAT4x3:                             return ShaderDataType::Dmat4x3;
			case GL_SAMPLER_1D:                                return ShaderDataType::Sampler1D;
			case GL_SAMPLER_2D:                                return ShaderDataType::Sampler2D;
			case GL_SAMPLER_3D:                                return ShaderDataType::Sampler3D;
			case GL_SAMPLER_CUBE:                              return ShaderDataType::SamplerCube;
			case GL_SAMPLER_1D_SHADOW:                         return ShaderDataType::Sampler1DShadow;
			case GL_SAMPLER_2D_SHADOW:                         return ShaderDataType::Sampler2DShadow;
			case GL_SAMPLER_1D_ARRAY:                          return ShaderDataType::Sampler1DArray;
			case GL_SAMPLER_2D_ARRAY:                          return ShaderDataType::Sampler2DArray;
			case GL_SAMPLER_1D_ARRAY_SHADOW:                   return ShaderDataType::Sampler1DArrayShadow;
			case GL_SAMPLER_2D_ARRAY_SHADOW:                   return ShaderDataType::Sampler2DArrayShadow;
			case GL_SAMPLER_2D_MULTISAMPLE:                    return ShaderDataType::Sampler2DMS;
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:              return ShaderDataType::Sampler2DMSArray;
			case GL_SAMPLER_CUBE_SHADOW:                       return ShaderDataType::SamplerCubeShadow;
			case GL_SAMPLER_BUFFER:                            return ShaderDataType::SamplerBuffer;
			case GL_SAMPLER_2D_RECT:                           return ShaderDataType::Sampler2DRect;
			case GL_SAMPLER_2D_RECT_SHADOW:                    return ShaderDataType::Sampler2DRectShadow;
			case GL_INT_SAMPLER_1D:                            return ShaderDataType::Isampler1D;
			case GL_INT_SAMPLER_2D:                            return ShaderDataType::Isampler2D;
			case GL_INT_SAMPLER_3D:                            return ShaderDataType::Isampler3D;
			case GL_INT_SAMPLER_CUBE:                          return ShaderDataType::IsamplerCube;
			case GL_INT_SAMPLER_1D_ARRAY:                      return ShaderDataType::Isampler1DArray;
			case GL_INT_SAMPLER_2D_ARRAY:                      return ShaderDataType::Isampler2DArray;
			case GL_INT_SAMPLER_2D_MULTISAMPLE:                return ShaderDataType::Isampler2DMS;
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:          return ShaderDataType::Isampler2DMSArray;
			case GL_INT_SAMPLER_BUFFER:                        return ShaderDataType::IsamplerBuffer;
			case GL_INT_SAMPLER_2D_RECT:                       return ShaderDataType::Isampler2DRect;
			case GL_UNSIGNED_INT_SAMPLER_1D:                   return ShaderDataType::Usampler1D;
			case GL_UNSIGNED_INT_SAMPLER_2D:                   return ShaderDataType::Usampler2D;
			case GL_UNSIGNED_INT_SAMPLER_3D:                   return ShaderDataType::Usampler3D;
			case GL_UNSIGNED_INT_SAMPLER_CUBE:                 return ShaderDataType::UsamplerCube;
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:             return ShaderDataType::Usampler2DArray;
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:       return ShaderDataType::Usampler2DMS;
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return ShaderDataType::Usampler2DMSArray;
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:               return ShaderDataType::UsamplerBuffer;
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT:              return ShaderDataType::Usampler2DRect;
			default: ASSERT(false, "[OPENGL] Unknown ShaderDataType requested"); return ShaderDataType::Unknown;
		}
	}
	GLenum convert(ShaderDataType p_data_type)
	{
		switch (p_data_type)
		{
			case ShaderDataType::Float:                                          return GL_FLOAT;
			case ShaderDataType::Vec2:                                           return GL_FLOAT_VEC2;
			case ShaderDataType::Vec3:                                           return GL_FLOAT_VEC3;
			case ShaderDataType::Vec4:                                           return GL_FLOAT_VEC4;
			case ShaderDataType::Double:                                         return GL_DOUBLE;
			case ShaderDataType::DVec2:                                          return GL_DOUBLE_VEC2;
			case ShaderDataType::DVec3:                                          return GL_DOUBLE_VEC3;
			case ShaderDataType::DVec4:                                          return GL_DOUBLE_VEC4;
			case ShaderDataType::Int:                                            return GL_INT;
			case ShaderDataType::IVec2:                                          return GL_INT_VEC2;
			case ShaderDataType::IVec3:                                          return GL_INT_VEC3;
			case ShaderDataType::IVec4:                                          return GL_INT_VEC4;
			case ShaderDataType::UnsignedInt:                                    return GL_UNSIGNED_INT;
			case ShaderDataType::UVec2:                                          return GL_UNSIGNED_INT_VEC2;
			case ShaderDataType::UVec3:                                          return GL_UNSIGNED_INT_VEC3;
			case ShaderDataType::UVec4:                                          return GL_UNSIGNED_INT_VEC4;
			case ShaderDataType::Bool:                                           return GL_BOOL;
			case ShaderDataType::BVec2:                                          return GL_BOOL_VEC2;
			case ShaderDataType::BVec3:                                          return GL_BOOL_VEC3;
			case ShaderDataType::BVec4:                                          return GL_BOOL_VEC4;
			case ShaderDataType::Mat2:                                           return GL_FLOAT_MAT2;
			case ShaderDataType::Mat3:                                           return GL_FLOAT_MAT3;
			case ShaderDataType::Mat4:                                           return GL_FLOAT_MAT4;
			case ShaderDataType::Mat2x3:                                         return GL_FLOAT_MAT2x3;
			case ShaderDataType::Mat2x4:                                         return GL_FLOAT_MAT2x4;
			case ShaderDataType::Mat3x2:                                         return GL_FLOAT_MAT3x2;
			case ShaderDataType::Mat3x4:                                         return GL_FLOAT_MAT3x4;
			case ShaderDataType::Mat4x2:                                         return GL_FLOAT_MAT4x2;
			case ShaderDataType::Mat4x3:                                         return GL_FLOAT_MAT4x3;
			case ShaderDataType::Dmat2:                                          return GL_DOUBLE_MAT2;
			case ShaderDataType::Dmat3:                                          return GL_DOUBLE_MAT3;
			case ShaderDataType::Dmat4:                                          return GL_DOUBLE_MAT4;
			case ShaderDataType::Dmat2x3:                                        return GL_DOUBLE_MAT2x3;
			case ShaderDataType::Dmat2x4:                                        return GL_DOUBLE_MAT2x4;
			case ShaderDataType::Dmat3x2:                                        return GL_DOUBLE_MAT3x2;
			case ShaderDataType::Dmat3x4:                                        return GL_DOUBLE_MAT3x4;
			case ShaderDataType::Dmat4x2:                                        return GL_DOUBLE_MAT4x2;
			case ShaderDataType::Dmat4x3:                                        return GL_DOUBLE_MAT4x3;
			case ShaderDataType::Sampler1D:                                      return GL_SAMPLER_1D;
			case ShaderDataType::Sampler2D:                                      return GL_SAMPLER_2D;
			case ShaderDataType::Sampler3D:                                      return GL_SAMPLER_3D;
			case ShaderDataType::SamplerCube:                                    return GL_SAMPLER_CUBE;
			case ShaderDataType::Sampler1DShadow:                                return GL_SAMPLER_1D_SHADOW;
			case ShaderDataType::Sampler2DShadow:                                return GL_SAMPLER_2D_SHADOW;
			case ShaderDataType::Sampler1DArray:                                 return GL_SAMPLER_1D_ARRAY;
			case ShaderDataType::Sampler2DArray:                                 return GL_SAMPLER_2D_ARRAY;
			case ShaderDataType::Sampler1DArrayShadow:                           return GL_SAMPLER_1D_ARRAY_SHADOW;
			case ShaderDataType::Sampler2DArrayShadow:                           return GL_SAMPLER_2D_ARRAY_SHADOW;
			case ShaderDataType::Sampler2DMS:                                    return GL_SAMPLER_2D_MULTISAMPLE;
			case ShaderDataType::Sampler2DMSArray:                               return GL_SAMPLER_2D_MULTISAMPLE_ARRAY;
			case ShaderDataType::SamplerCubeShadow:                              return GL_SAMPLER_CUBE_SHADOW;
			case ShaderDataType::SamplerBuffer:                                  return GL_SAMPLER_BUFFER;
			case ShaderDataType::Sampler2DRect:                                  return GL_SAMPLER_2D_RECT;
			case ShaderDataType::Sampler2DRectShadow:                            return GL_SAMPLER_2D_RECT_SHADOW;
			case ShaderDataType::Isampler1D:                                     return GL_INT_SAMPLER_1D;
			case ShaderDataType::Isampler2D:                                     return GL_INT_SAMPLER_2D;
			case ShaderDataType::Isampler3D:                                     return GL_INT_SAMPLER_3D;
			case ShaderDataType::IsamplerCube:                                   return GL_INT_SAMPLER_CUBE;
			case ShaderDataType::Isampler1DArray:                                return GL_INT_SAMPLER_1D_ARRAY;
			case ShaderDataType::Isampler2DArray:                                return GL_INT_SAMPLER_2D_ARRAY;
			case ShaderDataType::Isampler2DMS:                                   return GL_INT_SAMPLER_2D_MULTISAMPLE;
			case ShaderDataType::Isampler2DMSArray:                              return GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
			case ShaderDataType::IsamplerBuffer:                                 return GL_INT_SAMPLER_BUFFER;
			case ShaderDataType::Isampler2DRect:                                 return GL_INT_SAMPLER_2D_RECT;
			case ShaderDataType::Usampler1D:                                     return GL_UNSIGNED_INT_SAMPLER_1D;
			case ShaderDataType::Usampler2D:                                     return GL_UNSIGNED_INT_SAMPLER_2D;
			case ShaderDataType::Usampler3D:                                     return GL_UNSIGNED_INT_SAMPLER_3D;
			case ShaderDataType::UsamplerCube:                                   return GL_UNSIGNED_INT_SAMPLER_CUBE;
			case ShaderDataType::Usampler2DArray:                                return GL_UNSIGNED_INT_SAMPLER_2D_ARRAY;
			case ShaderDataType::Usampler2DMS:                                   return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
			case ShaderDataType::Usampler2DMSArray:                              return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
			case ShaderDataType::UsamplerBuffer:                                 return GL_UNSIGNED_INT_SAMPLER_BUFFER;
			case ShaderDataType::Usampler2DRect:                                 return GL_UNSIGNED_INT_SAMPLER_2D_RECT;
			default: ASSERT(false, "[OPENGL] Unknown ShaderDataType requested"); return 0;
		}
	}
	GLenum convert(ShaderProgramType p_shader_program_type)
	{
		switch (p_shader_program_type)
		{
			case ShaderProgramType::Vertex:                                         return GL_VERTEX_SHADER;
			case ShaderProgramType::Geometry:                                       return GL_GEOMETRY_SHADER;
			case ShaderProgramType::Fragment:                                       return GL_FRAGMENT_SHADER;
			case ShaderProgramType::Compute:                                        return GL_COMPUTE_SHADER;
			default: ASSERT(false, "[OPENGL] Unknown ShaderProgramType requested"); return 0;
		}
	}


	GLenum convert(DepthTestType p_depth_test_type)
	{
		switch (p_depth_test_type)
		{
			case DepthTestType::Always:                                         return GL_ALWAYS;
			case DepthTestType::Never:                                          return GL_NEVER;
			case DepthTestType::Less:                                           return GL_LESS;
			case DepthTestType::Equal:                                          return GL_EQUAL;
			case DepthTestType::LessEqual:                                      return GL_LEQUAL;
			case DepthTestType::Greater:                                        return GL_GREATER;
			case DepthTestType::NotEqual:                                       return GL_NOTEQUAL;
			case DepthTestType::GreaterEqual:                                   return GL_GEQUAL;
			default: ASSERT(false, "[OPENGL] Unknown DepthTestType requested"); return 0;
		}
	}
	GLenum convert(BlendFactorType p_blend_factor_type)
	{
		switch (p_blend_factor_type)
		{
			case BlendFactorType::Zero:                                           return GL_ZERO;
			case BlendFactorType::One:                                            return GL_ONE;
			case BlendFactorType::SourceColour:                                   return GL_SRC_COLOR;
			case BlendFactorType::OneMinusSourceColour:                           return GL_ONE_MINUS_SRC_COLOR;
			case BlendFactorType::DestinationColour:                              return GL_DST_COLOR;
			case BlendFactorType::OneMinusDestinationColour:                      return GL_ONE_MINUS_DST_COLOR;
			case BlendFactorType::SourceAlpha:                                    return GL_SRC_ALPHA;
			case BlendFactorType::OneMinusSourceAlpha:                            return GL_ONE_MINUS_SRC_ALPHA;
			case BlendFactorType::DestinationAlpha:                               return GL_DST_ALPHA;
			case BlendFactorType::OneMinusDestinationAlpha:                       return GL_ONE_MINUS_DST_ALPHA;
			case BlendFactorType::ConstantColour:                                 return GL_CONSTANT_COLOR;
			case BlendFactorType::OneMinusConstantColour:                         return GL_ONE_MINUS_CONSTANT_COLOR;
			case BlendFactorType::ConstantAlpha:                                  return GL_CONSTANT_ALPHA;
			case BlendFactorType::OneMinusConstantAlpha:                          return GL_ONE_MINUS_CONSTANT_ALPHA;
			default: ASSERT(false, "[OPENGL] Unknown BlendFactorType requested"); return 0;
		}
	}
	GLenum convert(CullFaceType p_cull_faces_type)
	{
		switch (p_cull_faces_type)
		{
			case CullFaceType::Back:                                           return GL_BACK;
			case CullFaceType::Front:                                          return GL_FRONT;
			case CullFaceType::FrontAndBack:                                   return GL_FRONT_AND_BACK;
			default: ASSERT(false, "[OPENGL] Unknown CullFaceType requested"); return 0;
		}
	}
	GLenum convert(FrontFaceOrientation p_front_face_orientation)
	{
		switch (p_front_face_orientation)
		{
			case FrontFaceOrientation::Clockwise:                                      return GL_CW;
			case FrontFaceOrientation::CounterClockwise:                               return GL_CCW;
			default: ASSERT(false, "[OPENGL] Unknown FrontFaceOrientation requested"); return 0;
		}
	}
	GLenum convert(PolygonMode p_polygon_mode)
	{
		switch (p_polygon_mode)
		{
			case PolygonMode::Point:                                          return GL_POINT;
			case PolygonMode::Line:                                           return GL_LINE;
			case PolygonMode::Fill:                                           return GL_FILL;
			default: ASSERT(false, "[OPENGL] Unknown PolygonMode requested"); return 0;
		}
	}
	GLenum convert(PrimitiveMode p_primitive_mode)
	{
		switch (p_primitive_mode)
		{
			case PrimitiveMode::Points:                                         return GL_POINTS;
			case PrimitiveMode::LineStrip:                                      return GL_LINE_STRIP;
			case PrimitiveMode::LineLoop:                                       return GL_LINE_LOOP;
			case PrimitiveMode::Lines:                                          return GL_LINES;
			case PrimitiveMode::LineStripAdjacency:                             return GL_LINE_STRIP_ADJACENCY;
			case PrimitiveMode::LinesAdjacency:                                 return GL_LINES_ADJACENCY;
			case PrimitiveMode::TriangleStrip:                                  return GL_TRIANGLE_STRIP;
			case PrimitiveMode::TriangleFan:                                    return GL_TRIANGLE_FAN;
			case PrimitiveMode::Triangles:                                      return GL_TRIANGLES;
			case PrimitiveMode::TriangleStripAdjacency:                         return GL_TRIANGLE_STRIP_ADJACENCY;
			case PrimitiveMode::TrianglesAdjacency:                             return GL_TRIANGLES_ADJACENCY;
			case PrimitiveMode::Patches:                                        return GL_PATCHES;
			default: ASSERT(false, "[OPENGL] Unknown PrimitiveMode requested"); return 0;
		}
	}
}