#include "GLState.hpp"

#include "Utility/Logger.hpp"

#include "glm/vec4.hpp"
#include "glm/vec2.hpp"

#include "glad/glad.h" // OpenGL functions

namespace OpenGL
{
	// OpenGL::State is a cpp only class so can only be accessed within the GLState.cpp. This allows us
	// to encapsulate the state of the OpenGL context and prevent other parts of the program from modifying it directly.
	// State prevents excess gl function calls by only allowing them to be called if there is a state change.
	class State
	{
	public:
		State()
			: depth_test_enabled(true)
			, depth_test_type(DepthTestType::Less)
			, polygon_offset_enabled(false)
			, polygon_offset_factor(0.0f)
			, polygon_offset_units(0.0f)
			, blending_enabled(false)
			, source_factor(BlendFactorType::SourceAlpha)
			, destination_factor(BlendFactorType::OneMinusSourceAlpha)
			, cull_face_enabled(true)
			, cull_face_type(CullFaceType::Back)
			, front_face_orientation(FrontFaceOrientation::CounterClockwise)
			, polygon_mode(PolygonMode::Fill)
			, clear_colour(0.0f, 0.0f, 0.0f, 0.0f)
			, viewport_position(0, 0)
			, viewport_size(0, 0)
		{
			if (depth_test_enabled)     glEnable(GL_DEPTH_TEST);
			else                        glDisable(GL_DEPTH_TEST);
			if (polygon_offset_enabled) glEnable(GL_POLYGON_OFFSET_FILL);
			else                        glDisable(GL_POLYGON_OFFSET_FILL);
			if (blending_enabled)       glEnable(GL_BLEND);
			else                        glDisable(GL_BLEND);
			if (cull_face_enabled)      glEnable(GL_CULL_FACE);
			else                        glDisable(GL_CULL_FACE);

			glDepthFunc(convert(depth_test_type));
			glPolygonOffset(polygon_offset_factor, polygon_offset_units);
			glBlendFunc(convert(source_factor), convert(destination_factor));
			glCullFace(convert(cull_face_type));
			glFrontFace(convert(front_face_orientation));
			glPolygonMode(GL_FRONT_AND_BACK, convert(polygon_mode));
			glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]);
			glViewport(viewport_position.x, viewport_position.y, viewport_size.x, viewport_size.y);
		}

		bool depth_test_enabled;
		DepthTestType depth_test_type;

		bool polygon_offset_enabled;
		GLfloat polygon_offset_factor;
		GLfloat polygon_offset_units;

		bool blending_enabled;
		BlendFactorType source_factor;
		BlendFactorType destination_factor;

		bool cull_face_enabled;
		CullFaceType cull_face_type;
		FrontFaceOrientation front_face_orientation;

		PolygonMode polygon_mode;

		glm::vec4 clear_colour;
		glm::ivec2 viewport_position;
		glm::ivec2 viewport_size;
	};
	static State& state()
	{
		// State accessor allows us to delay instantiation of the state until the
		// OpenGL context is initialised in Core::initialise_OpenGL.
		static State instance;
		return instance;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// GENERAL STATE FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void set_depth_test(bool p_depth_test)
	{
		if (p_depth_test != state().depth_test_enabled)
		{
			if (p_depth_test)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);
			state().depth_test_enabled = p_depth_test;
		}
	}
	void set_depth_test_type(DepthTestType p_type)
	{
		if (p_type != state().depth_test_type)
		{
			glDepthFunc(convert(p_type));
			state().depth_test_type = p_type;
		}
	}
	void set_polygon_offset(bool p_polygon_offset)
	{
		if (p_polygon_offset != state().polygon_offset_enabled)
		{
			if (p_polygon_offset)
				glEnable(GL_POLYGON_OFFSET_FILL);
			else
				glDisable(GL_POLYGON_OFFSET_FILL);
			state().polygon_offset_enabled = p_polygon_offset;
		}
	}
	void set_polygon_offset_factor(GLfloat p_polygon_offset_factor, GLfloat p_polygon_offset_units)
	{
		if (p_polygon_offset_factor != state().polygon_offset_factor || p_polygon_offset_units != state().polygon_offset_units)
		{
			glPolygonOffset(p_polygon_offset_factor, p_polygon_offset_units);
			state().polygon_offset_factor = p_polygon_offset_factor;
			state().polygon_offset_units  = p_polygon_offset_units;
		}
	}
	void set_blending(bool p_blend)
	{
		if (p_blend != state().blending_enabled)
		{
			if (p_blend)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);

			state().blending_enabled = p_blend;
		}
	}
	void set_blend_func(BlendFactorType p_source_factor, BlendFactorType p_destination_factor)
	{
		ASSERT(state().blending_enabled, "Blending has to be enabled to set blend function.");

		if (p_source_factor != state().source_factor || p_destination_factor != state().destination_factor)
		{
			glBlendFunc(convert(p_source_factor), convert(p_destination_factor)); // It is also possible to set individual RGBA factors using glBlendFuncSeparate().
			state().source_factor      = p_source_factor;
			state().destination_factor = p_destination_factor;
		}
	}

	void set_cull_face(bool p_cull)
	{
		if (p_cull != state().cull_face_enabled)
		{
			if (p_cull)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);

			state().cull_face_enabled = p_cull;
		}
	}
	void set_cull_face_type(CullFaceType p_cull_face_type)
	{
		if (p_cull_face_type != state().cull_face_type)
		{
			glCullFace(convert(p_cull_face_type));
			state().cull_face_type = p_cull_face_type;
		}
	}
	void set_front_face_orientation(FrontFaceOrientation p_front_face_orientation)
	{
		if (p_front_face_orientation != state().front_face_orientation)
		{
			glFrontFace(convert(p_front_face_orientation));
			state().front_face_orientation = p_front_face_orientation;
		}
	}
	void set_polygon_mode(PolygonMode p_polygon_mode)
	{
		if (p_polygon_mode != state().polygon_mode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, convert(p_polygon_mode));
			state().polygon_mode = p_polygon_mode;
		}
	}

	void set_clear_colour(const glm::vec4& p_colour)
	{
		if (p_colour != state().clear_colour)
		{
			glClearColor(p_colour[0], p_colour[1], p_colour[2], p_colour[3]);
			state().clear_colour = p_colour;
		}
	}
	void set_viewport(GLint p_x, GLint p_y, GLsizei p_width, GLsizei p_height)
	{
		if (p_x != state().viewport_position.x || p_y != state().viewport_position.y || p_width != state().viewport_size.x || p_height != state().viewport_size.y)
		{
			glViewport(p_x, p_y, p_width, p_height);
			state().viewport_position = { p_x, p_y };
			state().viewport_size     = { p_width, p_height };
		}
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TEXTURE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void active_texture(GLenum p_texture)
	{
		glActiveTexture(GL_TEXTURE0 + p_texture);
	}
	void tex_storage_3D(TextureType p_target, GLsizei p_levels, ImageFormat p_internal_format, GLsizei p_width, GLsizei p_height, GLsizei p_depth)
	{
		glTexStorage3D(convert(p_target), p_levels, convert(p_internal_format), p_width, p_height, p_depth);
	}
	void tex_sub_image_3D(TextureType p_target, GLint p_level, GLint p_xoffset, GLint p_yoffset, GLint p_zoffset, GLsizei p_width, GLsizei p_height, GLsizei p_depth, PixelDataFormat p_format, PixelDataType p_type, const void * p_pixels)
	{
		glTexSubImage3D(convert(p_target), p_level, p_xoffset, p_yoffset, p_zoffset, p_width, p_height, p_depth, convert(p_format), convert(p_type), p_pixels);
	}
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
	};
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DRAW FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHADER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GLHandle create_program()
	{
		GLHandle handle = glCreateProgram();
		ASSERT(handle != 0, "[OPENGL] Error occurred creating the shader program object");
		return handle;
	}
	void delete_program(GLHandle p_shader_program)
	{
		glDeleteProgram(p_shader_program);
	}
	void use_program(GLHandle p_shader_program)
	{
		glUseProgram(p_shader_program);
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
	GLint get_max_uniform_binding_points()
	{
		GLint max_uniform_buffer_bindings = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_uniform_buffer_bindings);
		return max_uniform_buffer_bindings;
	}
	GLint get_max_shader_storage_binding_points()
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
	GLHandle get_current_shader_program()
	{
		GLint handle;
		glGetIntegerv(GL_CURRENT_PROGRAM, &handle);
		return handle;
	}

	void uniform_block_binding(GLHandle p_shader_program, GLuint p_uniform_block_index, GLuint p_uniform_block_binding)
	{
		glUniformBlockBinding(p_shader_program, p_uniform_block_index, p_uniform_block_binding);
	}
	void shader_storage_block_binding(GLHandle p_shader_program, GLuint p_storage_block_index, GLuint p_storage_block_binding)
	{
		glShaderStorageBlockBinding(p_shader_program, p_storage_block_index, p_storage_block_binding);
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DATA/BUFFER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void enable_vertex_attrib_array(GLuint p_index)
	{
		glEnableVertexAttribArray(p_index);
	}
	void disable_vertex_attrib_array(GLuint p_index)
	{
		glDisableVertexAttribArray(p_index);
	}
	void vertex_attrib_pointer(GLuint p_index, GLint p_size, ShaderDataType p_type, GLboolean p_normalized, GLsizei p_stride, const void* p_pointer)
	{
		glVertexAttribPointer(p_index, p_size, convert(p_type), p_normalized, p_stride, (void*)p_pointer);
	}
	void buffer_data(BufferType p_target, GLsizeiptr p_size, const void* p_data, BufferUsage p_usage)
	{
		glBufferData(convert(p_target), p_size, p_data, convert(p_usage));
	}
	void buffer_sub_data(BufferType p_target, GLintptr p_offset, GLsizeiptr p_size, const void* p_data)
	{
		glBufferSubData(convert(p_target), p_offset, p_size, p_data);
	}
	void copy_buffer_sub_data(BufferType p_source_target, BufferType p_destination_target, GLintptr p_source_offset, GLintptr p_destination_offset, GLsizeiptr p_size)
	{
		glCopyBufferSubData(convert(p_source_target), convert(p_destination_target), p_source_offset, p_destination_offset, p_size);
	}
	void bind_buffer_range(BufferType p_target, GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size)
	{
		glBindBufferRange(convert(p_target), p_index, p_buffer, p_offset, p_size);
	}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CONVERSION FUNCS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ShaderDataType convert(int p_data_type)
	{
		switch (p_data_type)
		{
			case GL_FLOAT:                                                 return ShaderDataType::Float;
			case GL_FLOAT_VEC2:                                            return ShaderDataType::Vec2;
			case GL_FLOAT_VEC3:                                            return ShaderDataType::Vec3;
			case GL_FLOAT_VEC4:                                            return ShaderDataType::Vec4;
			case GL_DOUBLE:                                                return ShaderDataType::Double;
			case GL_DOUBLE_VEC2:                                           return ShaderDataType::DVec2;
			case GL_DOUBLE_VEC3:                                           return ShaderDataType::DVec3;
			case GL_DOUBLE_VEC4:                                           return ShaderDataType::DVec4;
			case GL_INT:                                                   return ShaderDataType::Int;
			case GL_INT_VEC2:                                              return ShaderDataType::IVec2;
			case GL_INT_VEC3:                                              return ShaderDataType::IVec3;
			case GL_INT_VEC4:                                              return ShaderDataType::IVec4;
			case GL_UNSIGNED_INT:                                          return ShaderDataType::UnsignedInt;
			case GL_UNSIGNED_INT_VEC2:                                     return ShaderDataType::UVec2;
			case GL_UNSIGNED_INT_VEC3:                                     return ShaderDataType::UVec3;
			case GL_UNSIGNED_INT_VEC4:                                     return ShaderDataType::UVec4;
			case GL_BOOL:                                                  return ShaderDataType::Bool;
			case GL_BOOL_VEC2:                                             return ShaderDataType::BVec2;
			case GL_BOOL_VEC3:                                             return ShaderDataType::BVec3;
			case GL_BOOL_VEC4:                                             return ShaderDataType::BVec4;
			case GL_FLOAT_MAT2:                                            return ShaderDataType::Mat2;
			case GL_FLOAT_MAT3:                                            return ShaderDataType::Mat3;
			case GL_FLOAT_MAT4:                                            return ShaderDataType::Mat4;
			case GL_FLOAT_MAT2x3:                                          return ShaderDataType::Mat2x3;
			case GL_FLOAT_MAT2x4:                                          return ShaderDataType::Mat2x4;
			case GL_FLOAT_MAT3x2:                                          return ShaderDataType::Mat3x2;
			case GL_FLOAT_MAT3x4:                                          return ShaderDataType::Mat3x4;
			case GL_FLOAT_MAT4x2:                                          return ShaderDataType::Mat4x2;
			case GL_FLOAT_MAT4x3:                                          return ShaderDataType::Mat4x3;
			case GL_DOUBLE_MAT2:                                           return ShaderDataType::Dmat2;
			case GL_DOUBLE_MAT3:                                           return ShaderDataType::Dmat3;
			case GL_DOUBLE_MAT4:                                           return ShaderDataType::Dmat4;
			case GL_DOUBLE_MAT2x3:                                         return ShaderDataType::Dmat2x3;
			case GL_DOUBLE_MAT2x4:                                         return ShaderDataType::Dmat2x4;
			case GL_DOUBLE_MAT3x2:                                         return ShaderDataType::Dmat3x2;
			case GL_DOUBLE_MAT3x4:                                         return ShaderDataType::Dmat3x4;
			case GL_DOUBLE_MAT4x2:                                         return ShaderDataType::Dmat4x2;
			case GL_DOUBLE_MAT4x3:                                         return ShaderDataType::Dmat4x3;
			case GL_SAMPLER_1D:                                            return ShaderDataType::Sampler1D;
			case GL_SAMPLER_2D:                                            return ShaderDataType::Sampler2D;
			case GL_SAMPLER_3D:                                            return ShaderDataType::Sampler3D;
			case GL_SAMPLER_CUBE:                                          return ShaderDataType::SamplerCube;
			case GL_SAMPLER_1D_SHADOW:                                     return ShaderDataType::Sampler1DShadow;
			case GL_SAMPLER_2D_SHADOW:                                     return ShaderDataType::Sampler2DShadow;
			case GL_SAMPLER_1D_ARRAY:                                      return ShaderDataType::Sampler1DArray;
			case GL_SAMPLER_2D_ARRAY:                                      return ShaderDataType::Sampler2DArray;
			case GL_SAMPLER_1D_ARRAY_SHADOW:                               return ShaderDataType::Sampler1DArrayShadow;
			case GL_SAMPLER_2D_ARRAY_SHADOW:                               return ShaderDataType::Sampler2DArrayShadow;
			case GL_SAMPLER_2D_MULTISAMPLE:                                return ShaderDataType::Sampler2DMS;
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:                          return ShaderDataType::Sampler2DMSArray;
			case GL_SAMPLER_CUBE_SHADOW:                                   return ShaderDataType::SamplerCubeShadow;
			case GL_SAMPLER_BUFFER:                                        return ShaderDataType::SamplerBuffer;
			case GL_SAMPLER_2D_RECT:                                       return ShaderDataType::Sampler2DRect;
			case GL_SAMPLER_2D_RECT_SHADOW:                                return ShaderDataType::Sampler2DRectShadow;
			case GL_INT_SAMPLER_1D:                                        return ShaderDataType::Isampler1D;
			case GL_INT_SAMPLER_2D:                                        return ShaderDataType::Isampler2D;
			case GL_INT_SAMPLER_3D:                                        return ShaderDataType::Isampler3D;
			case GL_INT_SAMPLER_CUBE:                                      return ShaderDataType::IsamplerCube;
			case GL_INT_SAMPLER_1D_ARRAY:                                  return ShaderDataType::Isampler1DArray;
			case GL_INT_SAMPLER_2D_ARRAY:                                  return ShaderDataType::Isampler2DArray;
			case GL_INT_SAMPLER_2D_MULTISAMPLE:                            return ShaderDataType::Isampler2DMS;
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:                      return ShaderDataType::Isampler2DMSArray;
			case GL_INT_SAMPLER_BUFFER:                                    return ShaderDataType::IsamplerBuffer;
			case GL_INT_SAMPLER_2D_RECT:                                   return ShaderDataType::Isampler2DRect;
			case GL_UNSIGNED_INT_SAMPLER_1D:                               return ShaderDataType::Usampler1D;
			case GL_UNSIGNED_INT_SAMPLER_2D:                               return ShaderDataType::Usampler2D;
			case GL_UNSIGNED_INT_SAMPLER_3D:                               return ShaderDataType::Usampler3D;
			case GL_UNSIGNED_INT_SAMPLER_CUBE:                             return ShaderDataType::UsamplerCube;
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:                         return ShaderDataType::Usampler2DArray;
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:                   return ShaderDataType::Usampler2DMS;
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:             return ShaderDataType::Usampler2DMSArray;
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:                           return ShaderDataType::UsamplerBuffer;
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT:                          return ShaderDataType::Usampler2DRect;
			default: ASSERT(false, "[OPENGL] Unknown ShaderDataType requested"); return ShaderDataType::Unknown;
		}
	}

	const char* get_name(const Function& p_function)
	{
		switch (p_function)
		{
			case Function::Viewport:                                       return "Viewport";
			case Function::DrawArrays:                                     return "DrawArrays";
			case Function::DrawArraysInstanced:                            return "DrawArraysInstanced";
			case Function::DrawElements:                                   return "DrawElements";
			case Function::DrawElementsInstanced:                          return "DrawElementsInstanced";
			case Function::BindFramebuffer:                                return "BindFramebuffer";
			case Function::create_shader:                                  return "create_shader";
			case Function::shader_source:                                  return "shader_source";
			case Function::compile_shader:                                 return "compile_shader";
			case Function::create_program:                                 return "create_program";
			case Function::attach_shader:                                  return "attach_shader";
			case Function::link_program:                                   return "link_program";
			case Function::delete_shader:                                  return "delete_shader";
			case Function::use_program:                                    return "use_program";
			case Function::BindBuffer:                                     return "BindBuffer";
			case Function::DeleteBuffer:                                   return "DeleteBuffer";
			case Function::BufferData:                                     return "BufferData";
			case Function::buffer_sub_data:                                return "buffer_sub_data";
			case Function::bind_buffer_range:                              return "bind_buffer_range";
			case Function::uniform_block_binding:                          return "uniform_block_binding";
			case Function::shader_storage_block_binding:                   return "shader_storage_block_binding";
			case Function::copy_buffer_sub_data:                           return "copy_buffer_sub_data";
			default: ASSERT(false, "[OPENGL] Unknown Function requested"); return "";
		}
	}
	const char* get_name(ShaderProgramType p_shader_program_type)
	{
		switch (p_shader_program_type)
		{
			case ShaderProgramType::Vertex:                                         return "VertexShader";
			case ShaderProgramType::Geometry:                                       return "GeometryShader";
			case ShaderProgramType::Fragment:                                       return "FragmentShader";
			default: ASSERT(false, "[OPENGL] Unknown ShaderProgramType requested"); return "";
		}
	}
	const char* get_name(ShaderDataType p_data_type)
	{
		switch (p_data_type)
		{
			case ShaderDataType::Float:                                          return "Float";
			case ShaderDataType::Vec2:                                           return "vec2";
			case ShaderDataType::Vec3:                                           return "vec3";
			case ShaderDataType::Vec4:                                           return "vec4";
			case ShaderDataType::Double:                                         return "Double";
			case ShaderDataType::DVec2:                                          return "DVec2";
			case ShaderDataType::DVec3:                                          return "DVec3";
			case ShaderDataType::DVec4:                                          return "DVec4";
			case ShaderDataType::Int:                                            return "Int";
			case ShaderDataType::IVec2:                                          return "IVec2";
			case ShaderDataType::IVec3:                                          return "IVec3";
			case ShaderDataType::IVec4:                                          return "IVec4";
			case ShaderDataType::UnsignedInt:                                    return "UnsignedInt";
			case ShaderDataType::UVec2:                                          return "UVec2";
			case ShaderDataType::UVec3:                                          return "UVec3";
			case ShaderDataType::UVec4:                                          return "UVec4";
			case ShaderDataType::Bool:                                           return "Bool";
			case ShaderDataType::BVec2:                                          return "BVec2";
			case ShaderDataType::BVec3:                                          return "BVec3";
			case ShaderDataType::BVec4:                                          return "BVec4";
			case ShaderDataType::Mat2:                                           return "Mat2";
			case ShaderDataType::Mat3:                                           return "Mat3";
			case ShaderDataType::Mat4:                                           return "Mat4";
			case ShaderDataType::Mat2x3:                                         return "Mat2x3";
			case ShaderDataType::Mat2x4:                                         return "Mat2x4";
			case ShaderDataType::Mat3x2:                                         return "Mat3x2";
			case ShaderDataType::Mat3x4:                                         return "Mat3x4";
			case ShaderDataType::Mat4x2:                                         return "Mat4x2";
			case ShaderDataType::Mat4x3:                                         return "Mat4x3";
			case ShaderDataType::Dmat2:                                          return "Dmat2";
			case ShaderDataType::Dmat3:                                          return "Dmat3";
			case ShaderDataType::Dmat4:                                          return "Dmat4";
			case ShaderDataType::Dmat2x3:                                        return "Dmat2x3";
			case ShaderDataType::Dmat2x4:                                        return "Dmat2x4";
			case ShaderDataType::Dmat3x2:                                        return "Dmat3x2";
			case ShaderDataType::Dmat3x4:                                        return "Dmat3x4";
			case ShaderDataType::Dmat4x2:                                        return "Dmat4x2";
			case ShaderDataType::Dmat4x3:                                        return "Dmat4x3";
			case ShaderDataType::Sampler1D:                                      return "Sampler1D";
			case ShaderDataType::Sampler2D:                                      return "Sampler2D";
			case ShaderDataType::Sampler3D:                                      return "Sampler3D";
			case ShaderDataType::SamplerCube:                                    return "SamplerCube";
			case ShaderDataType::Sampler1DShadow:                                return "Sampler1DShadow";
			case ShaderDataType::Sampler2DShadow:                                return "Sampler2DShadow";
			case ShaderDataType::Sampler1DArray:                                 return "Sampler1DArray";
			case ShaderDataType::Sampler2DArray:                                 return "Sampler2DArray";
			case ShaderDataType::Sampler1DArrayShadow:                           return "Sampler1DArrayShadow";
			case ShaderDataType::Sampler2DArrayShadow:                           return "Sampler2DArrayShadow";
			case ShaderDataType::Sampler2DMS:                                    return "Sampler2DMS";
			case ShaderDataType::Sampler2DMSArray:                               return "Sampler2DMSArray";
			case ShaderDataType::SamplerCubeShadow:                              return "SamplerCubeShadow";
			case ShaderDataType::SamplerBuffer:                                  return "SamplerBuffer";
			case ShaderDataType::Sampler2DRect:                                  return "Sampler2DRect";
			case ShaderDataType::Sampler2DRectShadow:                            return "Sampler2DRectShadow";
			case ShaderDataType::Isampler1D:                                     return "Isampler1D";
			case ShaderDataType::Isampler2D:                                     return "Isampler2D";
			case ShaderDataType::Isampler3D:                                     return "Isampler3D";
			case ShaderDataType::IsamplerCube:                                   return "IsamplerCube";
			case ShaderDataType::Isampler1DArray:                                return "Isampler1DArray";
			case ShaderDataType::Isampler2DArray:                                return "Isampler2DArray";
			case ShaderDataType::Isampler2DMS:                                   return "Isampler2DMS";
			case ShaderDataType::Isampler2DMSArray:                              return "Isampler2DMSArray";
			case ShaderDataType::IsamplerBuffer:                                 return "IsamplerBuffer";
			case ShaderDataType::Isampler2DRect:                                 return "Isampler2DRect";
			case ShaderDataType::Usampler1D:                                     return "Usampler1D";
			case ShaderDataType::Usampler2D:                                     return "Usampler2D";
			case ShaderDataType::Usampler3D:                                     return "Usampler3D";
			case ShaderDataType::UsamplerCube:                                   return "UsamplerCube";
			case ShaderDataType::Usampler2DArray:                                return "Usampler2DArray";
			case ShaderDataType::Usampler2DMS:                                   return "Usampler2DMS";
			case ShaderDataType::Usampler2DMSArray:                              return "Usampler2DMSArray";
			case ShaderDataType::UsamplerBuffer:                                 return "UsamplerBuffer";
			case ShaderDataType::Usampler2DRect:                                 return "Usampler2DRect";
			case ShaderDataType::Unknown:                                        return "Unknown";
			default: ASSERT(false, "[OPENGL] Unknown ShaderDataType requested"); return "";
		}
	}
	const char* get_name(GLSLVariableType p_variable_type)
	{
		switch (p_variable_type)
		{
			case GLSLVariableType::Uniform:                                        return "'loose' Uniform variable";
			case GLSLVariableType::UniformBlock:                                   return "Uniform block variable";
			case GLSLVariableType::BufferBlock:                                    return "Buffer block variable";
			default: ASSERT(false, "[OPENGL] Unknown GLSLVariableType requested"); return "";
		}
	}
	const char* get_name(BufferType p_buffer_type)
	{
		switch (p_buffer_type)
		{
			case BufferType::ArrayBuffer:                                    return "Array Buffer";
			case BufferType::AtomicCounterBuffer:                            return "Atomic Counter Buffer";
			case BufferType::CopyReadBuffer:                                 return "Copy Read Buffer";
			case BufferType::CopyWriteBuffer:                                return "Copy Write Buffer";
			case BufferType::DispatchIndirectBuffer:                         return "Dispatch Indirect Buffer";
			case BufferType::DrawIndirectBuffer:                             return "Draw Indirect Buffer";
			case BufferType::ElementArrayBuffer:                             return "Element Array Buffer";
			case BufferType::PixelPackBuffer:                                return "Pixel Pack Buffer";
			case BufferType::PixelUnpackBuffer:                              return "Pixel Unpack Buffer";
			case BufferType::QueryBuffer:                                    return "Query Buffer";
			case BufferType::ShaderStorageBuffer:                            return "Shader Storage Buffer";
			case BufferType::TextureBuffer:                                  return "Texture Buffer";
			case BufferType::TransformFeedbackBuffer:                        return "Transform Feedback Buffer";
			case BufferType::UniformBuffer:                                  return "Uniform Buffer";
			default: ASSERT(false, "[OPENGL] Unknown BufferType requested"); return "";
		}
	}
	const char* get_name(BufferUsage p_buffer_usage)
	{
		switch (p_buffer_usage)
		{
			case BufferUsage::StreamDraw:                                        return "Stream Draw";
			case BufferUsage::StreamRead:                                        return "Stream Read";
			case BufferUsage::StreamCopy:                                        return "Stream Copy";
			case BufferUsage::StaticDraw:                                        return "Static Draw";
			case BufferUsage::StaticRead:                                        return "Static Read";
			case BufferUsage::StaticCopy:                                        return "Static Copy";
			case BufferUsage::DynamicDraw:                                       return "Dynamic Draw";
			case BufferUsage::DynamicRead:                                       return "Dynamic Read";
			case BufferUsage::DynamicCopy:                                       return "Dynamic Copy";
			default: ASSERT(false, "[OPENGL] Unknown p_buffer_usage requested"); return "";
		}
	}
	const char* get_name(ShaderResourceType p_resource_type)
	{
		switch (p_resource_type)
		{
			case ShaderResourceType::Uniform:                                        return "Uniform";
			case ShaderResourceType::UniformBlock:                                   return "UniformBlock";
			case ShaderResourceType::ShaderStorageBlock:                             return "ShaderStorageBlock";
			case ShaderResourceType::BufferVariable:                                 return "BufferVariable";
			case ShaderResourceType::Buffer:                                         return "Buffer";
			case ShaderResourceType::ProgramInput:                                   return "ProgramInput";
			case ShaderResourceType::ProgramOutput:                                  return "ProgramOutput";
			case ShaderResourceType::AtomicCounterBuffer:                            return "AtomicCounterBuffer";
			// case ShaderResourceType::AtomicCounterShader, :                       return //"AtomicCounterShader";
			case ShaderResourceType::VertexSubroutineUniform:                        return "VertexSubroutineUniform";
			case ShaderResourceType::FragmentSubroutineUniform:                      return "FragmentSubroutineUniform";
			case ShaderResourceType::GeometrySubroutineUniform:                      return "GeometrySubroutineUniform";
			case ShaderResourceType::ComputeSubroutineUniform:                       return "ComputeSubroutineUniform";
			case ShaderResourceType::TessControlSubroutineUniform:                   return "TessControlSubroutineUniform";
			case ShaderResourceType::TessEvaluationSubroutineUniform:                return "TessEvaluationSubroutineUniform";
			case ShaderResourceType::TransformFeedbackBuffer:                        return "TransformFeedbackBuffer";
			case ShaderResourceType::TransformFeedbackVarying:                       return "TransformFeedbackVarying";
			default: ASSERT(false, "[OPENGL] Unknown ShaderResourceType requested"); return "";
		}
	}
	const char* get_name(ShaderResourceProperty p_shader_resource_property)
	{
		switch (p_shader_resource_property)
		{
			case ShaderResourceProperty::NameLength:                                     return "NameLength";
			case ShaderResourceProperty::Type:                                           return "Type";
			case ShaderResourceProperty::ArraySize:                                      return "ArraySize";
			case ShaderResourceProperty::Offset:                                         return "Offset";
			case ShaderResourceProperty::BlockIndex:                                     return "BlockIndex";
			case ShaderResourceProperty::ArrayStride:                                    return "ArrayStride";
			case ShaderResourceProperty::MatrixStride:                                   return "MatrixStride";
			case ShaderResourceProperty::IsRowMajor:                                     return "IsRowMajor";
			case ShaderResourceProperty::AtomicCounterBufferIndex:                       return "AtomicCounterBufferIndex";
			case ShaderResourceProperty::TextureBuffer:                                  return "TextureBuffer";
			case ShaderResourceProperty::BufferBinding:                                  return "BufferBinding";
			case ShaderResourceProperty::BufferDataSize:                                 return "BufferDataSize";
			case ShaderResourceProperty::NumActiveVariables:                             return "NumActiveVariables";
			case ShaderResourceProperty::ActiveVariables:                                return "ActiveVariables";
			case ShaderResourceProperty::ReferencedByVertexShader:                       return "ReferencedByVertexShader";
			case ShaderResourceProperty::ReferencedByTessControlShader:                  return "ReferencedByTessControlShader";
			case ShaderResourceProperty::ReferencedByTessEvaluationShader:               return "ReferencedByTessEvaluationShader";
			case ShaderResourceProperty::ReferencedByGeometryShader:                     return "ReferencedByGeometryShader";
			case ShaderResourceProperty::ReferencedByFragmentShader:                     return "ReferencedByFragmentShader";
			case ShaderResourceProperty::ReferencedByComputeShader:                      return "ReferencedByComputeShader";
			case ShaderResourceProperty::NumCompatibleSubroutines:                       return "NumCompatibleSubroutines";
			case ShaderResourceProperty::CompatibleSubroutines:                          return "CompatibleSubroutines";
			case ShaderResourceProperty::TopLevelArraySize:                              return "TopLevelArraySize";
			case ShaderResourceProperty::TopLevelArrayStride:                            return "TopLevelArrayStride";
			case ShaderResourceProperty::Location:                                       return "Location";
			case ShaderResourceProperty::LocationIndex:                                  return "LocationIndex";
			case ShaderResourceProperty::IsPerPatch:                                     return "IsPerPatch";
			case ShaderResourceProperty::LocationComponent:                              return "LocationComponent";
			case ShaderResourceProperty::TransformFeedbackBufferIndex:                   return "TransformFeedbackBufferIndex";
			case ShaderResourceProperty::TransformFeedbackBufferStride:                  return "TransformFeedbackBufferStride";
			default: ASSERT(false, "[OPENGL] Unknown ShaderResourceProperty requested"); return "";
		}
	}
	const char* get_name(DepthTestType p_depth_test_type)
	{
		switch (p_depth_test_type)
		{
			case DepthTestType::Always:                                         return "Always";
			case DepthTestType::Never:                                          return "Never";
			case DepthTestType::Less:                                           return "Less";
			case DepthTestType::Equal:                                          return "Equal";
			case DepthTestType::NotEqual:                                       return "Not equal";
			case DepthTestType::Greater:                                        return "Greater than";
			case DepthTestType::LessEqual:                                      return "Less than or equal";
			case DepthTestType::GreaterEqual:                                   return "Greater than or equal";
			default: ASSERT(false, "[OPENGL] Unknown DepthTestType requested"); return "";
		}
	}
	const char* get_name(BlendFactorType p_blend_factor_type)
	{
		switch (p_blend_factor_type)
		{
			case BlendFactorType::Zero:                                           return "Zero";
			case BlendFactorType::One:                                            return "One";
			case BlendFactorType::SourceColour:                                   return "Source Colour";
			case BlendFactorType::OneMinusSourceColour:                           return "One Minus Source Colour";
			case BlendFactorType::DestinationColour:                              return "Destination Colour";
			case BlendFactorType::OneMinusDestinationColour:                      return "One Minus Destination Colour";
			case BlendFactorType::SourceAlpha:                                    return "Source Alpha";
			case BlendFactorType::OneMinusSourceAlpha:                            return "One Minus Source Alpha";
			case BlendFactorType::DestinationAlpha:                               return "Destination Alpha";
			case BlendFactorType::OneMinusDestinationAlpha:                       return "One Minus Destination Alpha";
			case BlendFactorType::ConstantColour:                                 return "Constant Colour";
			case BlendFactorType::OneMinusConstantColour:                         return "One Minus Constant Colour";
			case BlendFactorType::ConstantAlpha:                                  return "Constant Alpha";
			case BlendFactorType::OneMinusConstantAlpha:                          return "One Minus Constant Alpha";
			default: ASSERT(false, "[OPENGL] Unknown BlendFactorType requested"); return "";
		}
	}
	const char* get_name(CullFaceType p_cull_faces_type)
	{
		switch (p_cull_faces_type)
		{
			case CullFaceType::Back:                                           return "Back";
			case CullFaceType::Front:                                          return "Front";
			case CullFaceType::FrontAndBack:                                   return "Front and Back";
			default: ASSERT(false, "[OPENGL] Unknown CullFaceType requested"); return "";
		}
	}
	const char* get_name(FrontFaceOrientation p_front_face_orientation)
	{
		switch (p_front_face_orientation)
		{
			case FrontFaceOrientation::Clockwise:                                      return "Clockwise";
			case FrontFaceOrientation::CounterClockwise:                               return "CounterClockwise";
			default: ASSERT(false, "[OPENGL] Unknown FrontFaceOrientation requested"); return "";
		}
	}
	const char* get_name(PolygonMode p_polygon_mode)
	{
		switch (p_polygon_mode)
		{
			case PolygonMode::Point:                                          return "Point";
			case PolygonMode::Line:                                           return "Line";
			case PolygonMode::Fill:                                           return "Fill";
			default: ASSERT(false, "[OPENGL] Unknown PolygonMode requested"); return "";
		}
	}
	const char* get_name(PrimitiveMode p_primitive_mode)
	{
		switch (p_primitive_mode)
		{
			case PrimitiveMode::Points:                                         return "Points";
			case PrimitiveMode::LineStrip:                                      return "LineStrip";
			case PrimitiveMode::LineLoop:                                       return "LineLoop";
			case PrimitiveMode::Lines:                                          return "Lines";
			case PrimitiveMode::LineStripAdjacency:                             return "LineStripAdjacency";
			case PrimitiveMode::LinesAdjacency:                                 return "LinesAdjacency";
			case PrimitiveMode::TriangleStrip:                                  return "TriangleStrip";
			case PrimitiveMode::TriangleFan:                                    return "TriangleFan";
			case PrimitiveMode::Triangles:                                      return "Triangles";
			case PrimitiveMode::TriangleStripAdjacency:                         return "TriangleStripAdjacency";
			case PrimitiveMode::TrianglesAdjacency:                             return "TrianglesAdjacency";
			case PrimitiveMode::Patches:                                        return "Patches";
			default: ASSERT(false, "[OPENGL] Unknown PrimitiveMode requested"); return "";
		}
	}
	const char* get_name(FramebufferTarget p_framebuffer_target)
	{
		switch (p_framebuffer_target)
		{
			case FramebufferTarget::DrawFramebuffer:                                return "DrawFramebuffer";
			case FramebufferTarget::ReadFramebuffer:                                return "ReadFramebuffer";
			case FramebufferTarget::Framebuffer:                                    return "Framebuffer";
			default: ASSERT(false, "[OPENGL] Unknown FramebufferTarget requested"); return "";
		}
	}

	int convert(ShaderDataType p_data_type)
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
	int convert(GLSLVariableType p_variable_type)
	{
		switch (p_variable_type)
		{
			// Both loose uniform variables and UniformBlockVariables are treated as GL_UNIFORM in OpenGL
			case GLSLVariableType::Uniform:                                        return GL_UNIFORM;
			case GLSLVariableType::UniformBlock:                                   return GL_UNIFORM;
			case GLSLVariableType::BufferBlock:                                    return GL_BUFFER_VARIABLE;
			default: ASSERT(false, "[OPENGL] Unknown GLSLVariableType requested"); return 0;
		}
	}
	int convert(BufferType p_buffer_type)
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
	int convert(BufferUsage p_buffer_usage)
	{
		switch (p_buffer_usage)
		{
			case BufferUsage::StreamDraw:                                        return GL_STREAM_DRAW;
			case BufferUsage::StreamRead:                                        return GL_STREAM_READ;
			case BufferUsage::StreamCopy:                                        return GL_STREAM_COPY;
			case BufferUsage::StaticDraw:                                        return GL_STATIC_DRAW;
			case BufferUsage::StaticRead:                                        return GL_STATIC_READ;
			case BufferUsage::StaticCopy:                                        return GL_STATIC_COPY;
			case BufferUsage::DynamicDraw:                                       return GL_DYNAMIC_DRAW;
			case BufferUsage::DynamicRead:                                       return GL_DYNAMIC_READ;
			case BufferUsage::DynamicCopy:                                       return GL_DYNAMIC_COPY;
			default: ASSERT(false, "[OPENGL] Unknown p_buffer_usage requested"); return 0;
		}
	}
	int convert(TextureType p_texture_type)
	{
		switch (p_texture_type)
		{
			case TextureType::Texture_1D:                                        return GL_TEXTURE_1D;
			case TextureType::Texture_2D:                                        return GL_TEXTURE_2D;
			case TextureType::Texture_3D:                                        return GL_TEXTURE_3D;
			case TextureType::Texture_rectangle:                                 return GL_TEXTURE_RECTANGLE;
			case TextureType::Texture_buffer:                                    return GL_TEXTURE_BUFFER;
			case TextureType::Texture_cube_map:                                  return GL_TEXTURE_CUBE_MAP;
			case TextureType::Texture_1D_array:                                  return GL_TEXTURE_1D_ARRAY;
			case TextureType::Texture_2D_array:                                  return GL_TEXTURE_2D_ARRAY;
			case TextureType::Texture_cube_map_array:                            return GL_TEXTURE_CUBE_MAP_ARRAY;
			case TextureType::Texture_2D_multisample:                            return GL_TEXTURE_2D_MULTISAMPLE;
			case TextureType::Texture_2D_multisample_array:                      return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
			default: ASSERT(false, "[OPENGL] Unknown p_texture_type requested"); return 0;
		}
	}
	int convert(ImageFormat p_image_format)
	{
		switch (p_image_format)
		{
			case ImageFormat::R8:                   return GL_R8;
			case ImageFormat::R8_SNORM:             return GL_R8_SNORM;
			case ImageFormat::R16:                  return GL_R16;
			case ImageFormat::R16_SNORM:            return GL_R16_SNORM;
			case ImageFormat::RG8:                  return GL_RG8;
			case ImageFormat::RG8_SNORM:            return GL_RG8_SNORM;
			case ImageFormat::RG16:                 return GL_RG16;
			case ImageFormat::RG16_SNORM:           return GL_RG16_SNORM;
			case ImageFormat::R3_G3_B2:             return GL_R3_G3_B2;
			case ImageFormat::RGB4:                 return GL_RGB4;
			case ImageFormat::RGB5:                 return GL_RGB5;
			case ImageFormat::RGB8:                 return GL_RGB8;
			case ImageFormat::RGB8_SNORM:           return GL_RGB8_SNORM;
			case ImageFormat::RGB10:                return GL_RGB10;
			case ImageFormat::RGB12:                return GL_RGB12;
			case ImageFormat::RGB16_SNORM:          return GL_RGB16_SNORM;
			case ImageFormat::RGBA2:                return GL_RGBA2;
			case ImageFormat::RGBA4:                return GL_RGBA4;
			case ImageFormat::RGB5_A1:              return GL_RGB5_A1;
			case ImageFormat::RGBA8:                return GL_RGBA8;
			case ImageFormat::RGBA8_SNORM:          return GL_RGBA8_SNORM;
			case ImageFormat::RGB10_A2:             return GL_RGB10_A2;
			case ImageFormat::RGB10_A2UI:           return GL_RGB10_A2UI;
			case ImageFormat::RGBA12:               return GL_RGBA12;
			case ImageFormat::RGBA16:               return GL_RGBA16;
			case ImageFormat::SRGB8:                return GL_SRGB8;
			case ImageFormat::SRGB8_ALPHA8:         return GL_SRGB8_ALPHA8;
			case ImageFormat::R16F:                 return GL_R16F;
			case ImageFormat::RG16F:                return GL_RG16F;
			case ImageFormat::RGB16F:               return GL_RGB16F;
			case ImageFormat::RGBA16F:              return GL_RGBA16F;
			case ImageFormat::R32F:                 return GL_R32F;
			case ImageFormat::RG32F:                return GL_RG32F;
			case ImageFormat::RGB32F:               return GL_RGB32F;
			case ImageFormat::RGBA32F:              return GL_RGBA32F;
			case ImageFormat::R11F_G11F_B10F:       return GL_R11F_G11F_B10F;
			case ImageFormat::RGB9_E5:              return GL_RGB9_E5;
			case ImageFormat::R8I:                  return GL_R8I;
			case ImageFormat::R8UI:                 return GL_R8UI;
			case ImageFormat::R16I:                 return GL_R16I;
			case ImageFormat::R16UI:                return GL_R16UI;
			case ImageFormat::R32I:                 return GL_R32I;
			case ImageFormat::R32UI:                return GL_R32UI;
			case ImageFormat::RG8I:                 return GL_RG8I;
			case ImageFormat::RG8UI:                return GL_RG8UI;
			case ImageFormat::RG16I:                return GL_RG16I;
			case ImageFormat::RG16UI:               return GL_RG16UI;
			case ImageFormat::RG32I:                return GL_RG32I;
			case ImageFormat::RG32UI:               return GL_RG32UI;
			case ImageFormat::RGB8I:                return GL_RGB8I;
			case ImageFormat::RGB8UI:               return GL_RGB8UI;
			case ImageFormat::RGB16I:               return GL_RGB16I;
			case ImageFormat::RGB16UI:              return GL_RGB16UI;
			case ImageFormat::RGB32I:               return GL_RGB32I;
			case ImageFormat::RGB32UI:              return GL_RGB32UI;
			case ImageFormat::RGBA8I:               return GL_RGBA8I;
			case ImageFormat::RGBA8UI:              return GL_RGBA8UI;
			case ImageFormat::RGBA16I:              return GL_RGBA16I;
			case ImageFormat::RGBA16UI:             return GL_RGBA16UI;
			case ImageFormat::RGBA32I:              return GL_RGBA32I;
			case ImageFormat::RGBA32UI:             return GL_RGBA32UI;
			case ImageFormat::depth_component_32F:  return GL_DEPTH_COMPONENT32F;
			case ImageFormat::depth_component_24:   return GL_DEPTH_COMPONENT24;
			case ImageFormat::depth_component_16:   return GL_DEPTH_COMPONENT16;
			case ImageFormat::depth_32F_stencil_8:  return GL_DEPTH32F_STENCIL8;
			case ImageFormat::depth_24_stencil_8:   return GL_DEPTH24_STENCIL8;
			case ImageFormat::stencil_index_8:      return GL_STENCIL_INDEX8;
			default: ASSERT(false, "[OPENGL] Unknown p_image_format requested"); return 0;
		}
	}
	int convert(PixelDataFormat p_pixel_format)
	{
		switch (p_pixel_format)
		{
			case PixelDataFormat::RED:             return GL_RED;
			case PixelDataFormat::RG:              return GL_RG;
			case PixelDataFormat::RGB:             return GL_RGB;
			case PixelDataFormat::BGR:             return GL_BGR;
			case PixelDataFormat::RGBA:            return GL_RGBA;
			case PixelDataFormat::DEPTH_COMPONENT: return GL_DEPTH_COMPONENT;
			case PixelDataFormat::STENCIL_INDEX:   return GL_STENCIL_INDEX;
			default: ASSERT(false, "[OPENGL] Unknown p_pixel_format requested"); return 0;
		}
	}
	int convert(PixelDataType p_pixel_data_type)
	{
		switch (p_pixel_data_type)
		{
			case PixelDataType::UNSIGNED_BYTE:               return GL_UNSIGNED_BYTE;
			case PixelDataType::BYTE:                        return GL_BYTE;
			case PixelDataType::UNSIGNED_SHORT:              return GL_UNSIGNED_SHORT;
			case PixelDataType::SHORT:                       return GL_SHORT;
			case PixelDataType::UNSIGNED_INT:                return GL_UNSIGNED_INT;
			case PixelDataType::INT:                         return GL_INT;
			case PixelDataType::FLOAT:                       return GL_FLOAT;
			case PixelDataType::UNSIGNED_BYTE_3_3_2:         return GL_UNSIGNED_BYTE_3_3_2;
			case PixelDataType::UNSIGNED_BYTE_2_3_3_REV:     return GL_UNSIGNED_BYTE_2_3_3_REV;
			case PixelDataType::UNSIGNED_SHORT_5_6_5:        return GL_UNSIGNED_SHORT_5_6_5;
			case PixelDataType::UNSIGNED_SHORT_5_6_5_REV:    return GL_UNSIGNED_SHORT_5_6_5_REV;
			case PixelDataType::UNSIGNED_SHORT_4_4_4_4:      return GL_UNSIGNED_SHORT_4_4_4_4;
			case PixelDataType::UNSIGNED_SHORT_4_4_4_4_REV:  return GL_UNSIGNED_SHORT_4_4_4_4_REV;
			case PixelDataType::UNSIGNED_SHORT_5_5_5_1:      return GL_UNSIGNED_SHORT_5_5_5_1;
			case PixelDataType::UNSIGNED_SHORT_1_5_5_5_REV:  return GL_UNSIGNED_SHORT_1_5_5_5_REV;
			case PixelDataType::UNSIGNED_INT_8_8_8_8:        return GL_UNSIGNED_INT_8_8_8_8;
			case PixelDataType::UNSIGNED_INT_8_8_8_8_REV:    return GL_UNSIGNED_INT_8_8_8_8_REV;
			case PixelDataType::UNSIGNED_INT_10_10_10_2:     return GL_UNSIGNED_INT_10_10_10_2;
			case PixelDataType::UNSIGNED_INT_2_10_10_10_REV: return GL_UNSIGNED_INT_2_10_10_10_REV;
			default: ASSERT(false, "[OPENGL] Unknown p_pixel_data_type requested"); return 0;
		}
	}
	int convert(ShaderProgramType p_shader_program_type)
	{
		switch (p_shader_program_type)
		{
			case ShaderProgramType::Vertex:                                         return GL_VERTEX_SHADER;
			case ShaderProgramType::Geometry:                                       return GL_GEOMETRY_SHADER;
			case ShaderProgramType::Fragment:                                       return GL_FRAGMENT_SHADER;
			default: ASSERT(false, "[OPENGL] Unknown ShaderProgramType requested"); return 0;
		}
	}
	int convert(ShaderResourceType p_resource_type)
	{
		switch (p_resource_type)
		{
			case ShaderResourceType::Uniform:                                        return GL_UNIFORM;
			case ShaderResourceType::UniformBlock:                                   return GL_UNIFORM_BLOCK;
			case ShaderResourceType::ShaderStorageBlock:                             return GL_SHADER_STORAGE_BLOCK;
			case ShaderResourceType::BufferVariable:                                 return GL_BUFFER_VARIABLE;
			case ShaderResourceType::Buffer:                                         return GL_BUFFER;
			case ShaderResourceType::ProgramInput:                                   return GL_PROGRAM_INPUT;
			case ShaderResourceType::ProgramOutput:                                  return GL_PROGRAM_OUTPUT;
			case ShaderResourceType::AtomicCounterBuffer:                            return GL_ATOMIC_COUNTER_BUFFER;
			// case ShaderResourceType::AtomicCounterShader:                         return GL_ATOMIC_COUNTER_SHADER;
			case ShaderResourceType::VertexSubroutineUniform:                        return GL_VERTEX_SUBROUTINE_UNIFORM;
			case ShaderResourceType::FragmentSubroutineUniform:                      return GL_FRAGMENT_SUBROUTINE_UNIFORM;
			case ShaderResourceType::GeometrySubroutineUniform:                      return GL_GEOMETRY_SUBROUTINE_UNIFORM;
			case ShaderResourceType::ComputeSubroutineUniform:                       return GL_COMPUTE_SUBROUTINE_UNIFORM;
			case ShaderResourceType::TessControlSubroutineUniform:                   return GL_TESS_CONTROL_SUBROUTINE_UNIFORM;
			case ShaderResourceType::TessEvaluationSubroutineUniform:                return GL_TESS_EVALUATION_SUBROUTINE_UNIFORM;
			case ShaderResourceType::TransformFeedbackBuffer:                        return GL_TRANSFORM_FEEDBACK_BUFFER;
			case ShaderResourceType::TransformFeedbackVarying:                       return GL_TRANSFORM_FEEDBACK_VARYING;
			default: ASSERT(false, "[OPENGL] Unknown ShaderResourceType requested"); return 0;
		}
	}
	int convert(ShaderResourceProperty p_shader_resource_property)
	{
		switch (p_shader_resource_property)
		{
			case ShaderResourceProperty::NameLength:                                     return GL_NAME_LENGTH;
			case ShaderResourceProperty::Type:                                           return GL_TYPE;
			case ShaderResourceProperty::ArraySize:                                      return GL_ARRAY_SIZE;
			case ShaderResourceProperty::Offset:                                         return GL_OFFSET;
			case ShaderResourceProperty::BlockIndex:                                     return GL_BLOCK_INDEX;
			case ShaderResourceProperty::ArrayStride:                                    return GL_ARRAY_STRIDE;
			case ShaderResourceProperty::MatrixStride:                                   return GL_MATRIX_STRIDE;
			case ShaderResourceProperty::IsRowMajor:                                     return GL_IS_ROW_MAJOR;
			case ShaderResourceProperty::AtomicCounterBufferIndex:                       return GL_ATOMIC_COUNTER_BUFFER_INDEX;
			case ShaderResourceProperty::TextureBuffer:                                  return GL_TEXTURE_BUFFER;
			case ShaderResourceProperty::BufferBinding:                                  return GL_BUFFER_BINDING;
			case ShaderResourceProperty::BufferDataSize:                                 return GL_BUFFER_DATA_SIZE;
			case ShaderResourceProperty::NumActiveVariables:                             return GL_NUM_ACTIVE_VARIABLES;
			case ShaderResourceProperty::ActiveVariables:                                return GL_ACTIVE_VARIABLES;
			case ShaderResourceProperty::ReferencedByVertexShader:                       return GL_REFERENCED_BY_VERTEX_SHADER;
			case ShaderResourceProperty::ReferencedByTessControlShader:                  return GL_REFERENCED_BY_TESS_CONTROL_SHADER;
			case ShaderResourceProperty::ReferencedByTessEvaluationShader:               return GL_REFERENCED_BY_TESS_EVALUATION_SHADER;
			case ShaderResourceProperty::ReferencedByGeometryShader:                     return GL_REFERENCED_BY_GEOMETRY_SHADER;
			case ShaderResourceProperty::ReferencedByFragmentShader:                     return GL_REFERENCED_BY_FRAGMENT_SHADER;
			case ShaderResourceProperty::ReferencedByComputeShader:                      return GL_REFERENCED_BY_COMPUTE_SHADER;
			case ShaderResourceProperty::NumCompatibleSubroutines:                       return GL_NUM_COMPATIBLE_SUBROUTINES;
			case ShaderResourceProperty::CompatibleSubroutines:                          return GL_COMPATIBLE_SUBROUTINES;
			case ShaderResourceProperty::TopLevelArraySize:                              return GL_TOP_LEVEL_ARRAY_SIZE;
			case ShaderResourceProperty::TopLevelArrayStride:                            return GL_TOP_LEVEL_ARRAY_STRIDE;
			case ShaderResourceProperty::Location:                                       return GL_LOCATION;
			case ShaderResourceProperty::LocationIndex:                                  return GL_LOCATION_INDEX;
			case ShaderResourceProperty::IsPerPatch:                                     return GL_IS_PER_PATCH;
			case ShaderResourceProperty::LocationComponent:                              return GL_LOCATION_COMPONENT;
			case ShaderResourceProperty::TransformFeedbackBufferIndex:                   return GL_TRANSFORM_FEEDBACK_BUFFER_INDEX;
			case ShaderResourceProperty::TransformFeedbackBufferStride:                  return GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE;
			default: ASSERT(false, "[OPENGL] Unknown ShaderResourceProperty requested"); return 0;
		}
	}
	int convert(DepthTestType p_depth_test_type)
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
	int convert(BlendFactorType p_blend_factor_type)
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
	int convert(CullFaceType p_cull_faces_type)
	{
		switch (p_cull_faces_type)
		{
			case CullFaceType::Back:                                           return GL_BACK;
			case CullFaceType::Front:                                          return GL_FRONT;
			case CullFaceType::FrontAndBack:                                   return GL_FRONT_AND_BACK;
			default: ASSERT(false, "[OPENGL] Unknown CullFaceType requested"); return 0;
		}
	}
	int convert(FrontFaceOrientation p_front_face_orientation)
	{
		switch (p_front_face_orientation)
		{
			case FrontFaceOrientation::Clockwise:                                      return GL_CW;
			case FrontFaceOrientation::CounterClockwise:                               return GL_CCW;
			default: ASSERT(false, "[OPENGL] Unknown FrontFaceOrientation requested"); return 0;
		}
	}
	int convert(PolygonMode p_polygon_mode)
	{
		switch (p_polygon_mode)
		{
			case PolygonMode::Point:                                          return GL_POINT;
			case PolygonMode::Line:                                           return GL_LINE;
			case PolygonMode::Fill:                                           return GL_FILL;
			default: ASSERT(false, "[OPENGL] Unknown PolygonMode requested"); return 0;
		}
	}
	int convert(PrimitiveMode p_primitive_mode)
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
	int convert(FramebufferTarget p_framebuffer_target)
	{
		switch (p_framebuffer_target)
		{
			case FramebufferTarget::DrawFramebuffer:                                return GL_DRAW_FRAMEBUFFER;
			case FramebufferTarget::ReadFramebuffer:                                return GL_READ_FRAMEBUFFER;
			case FramebufferTarget::Framebuffer:                                    return GL_FRAMEBUFFER;
			default: ASSERT(false, "[OPENGL] Unknown FramebufferTarget requested"); return 0;
		}
	}
} // namespace OpenGL