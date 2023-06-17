#include "GLState.hpp"

#include "Logger.hpp"

#include "glm/vec4.hpp"

#include "glad/gl.h" // OpenGL functions

namespace OpenGL
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GENERAL STATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void set_depth_test(bool p_depth_test)
    {
        if (p_depth_test)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }
    void set_depth_test_type(DepthTestType p_type)
    {
        glDepthFunc(convert(p_type));
    }
    void toggle_blending(bool p_blend)
    {
        if (p_blend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }
    void set_blend_func(BlendFactorType p_source_factor, BlendFactorType p_destination_factor)
    {
        ASSERT(glIsEnabled(GL_BLEND), "Blending has to be enabled to set blend function.");
        glBlendFunc(convert(p_source_factor), convert(p_destination_factor)); // It is also possible to set individual RGBA factors using glBlendFuncSeparate().
    }
    void toggle_cull_face(bool p_cull)
    {
        if (p_cull)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }
    void set_cull_face_type(CullFacesType p_cull_face_type)
    {
        glCullFace(convert(p_cull_face_type));
    }
    void set_front_face_orientation(FrontFaceOrientation p_front_face_orientation)
    {
        glFrontFace(convert(p_front_face_orientation));
    }
    void set_clear_colour(const glm::vec4& p_colour)
    {
        glClearColor(p_colour[0], p_colour[1], p_colour[2], p_colour[3]);
    }
    void set_viewport(GLint p_x, GLint p_y, GLsizei p_width, GLsizei p_height)
    {
        glViewport(p_x, p_y, p_width, p_height);
    }
    void set_polygon_mode(PolygonMode p_polygon_mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, convert(p_polygon_mode));
    }
    void set_active_texture(int p_texture_unit_position)
    {
        glActiveTexture(GL_TEXTURE0 + p_texture_unit_position);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DRAW FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void draw_arrays(PrimitiveMode p_primitive_mode, GLint p_first, GLsizei p_count)
    {
        glDrawArrays(convert(p_primitive_mode), p_first, p_count);
    }
    void draw_arrays_instanced(PrimitiveMode p_primitive_mode, int p_array_size, int p_instance_count)
    {
        glDrawArraysInstanced(convert(p_primitive_mode), 0, p_array_size, p_instance_count);
    }
    void draw_elements(PrimitiveMode p_primitive_mode, int pElementsSize)
    {
        glDrawElements(convert(p_primitive_mode), pElementsSize, GL_UNSIGNED_INT, 0);
    }
    void draw_elements_instanced(PrimitiveMode p_primitive_mode, int pElementsSize, int p_instance_count)
    {
        glDrawElementsInstanced(convert(p_primitive_mode), pElementsSize, GL_UNSIGNED_INT, 0, p_instance_count);
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
    void vertex_attrib_pointer(GLuint p_index, GLint p_size, DataType p_type, GLboolean p_normalized, GLsizei p_stride, const void* p_pointer)
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

    DataType convert(int p_data_type)
    {
        switch (p_data_type)
        {
            case GL_FLOAT:                                                 return DataType::Float;
            case GL_FLOAT_VEC2:                                            return DataType::Vec2;
            case GL_FLOAT_VEC3:                                            return DataType::Vec3;
            case GL_FLOAT_VEC4:                                            return DataType::Vec4;
            case GL_DOUBLE:                                                return DataType::Double;
            case GL_DOUBLE_VEC2:                                           return DataType::DVec2;
            case GL_DOUBLE_VEC3:                                           return DataType::DVec3;
            case GL_DOUBLE_VEC4:                                           return DataType::DVec4;
            case GL_INT:                                                   return DataType::Int;
            case GL_INT_VEC2:                                              return DataType::IVec2;
            case GL_INT_VEC3:                                              return DataType::IVec3;
            case GL_INT_VEC4:                                              return DataType::IVec4;
            case GL_UNSIGNED_INT:                                          return DataType::UnsignedInt;
            case GL_UNSIGNED_INT_VEC2:                                     return DataType::UVec2;
            case GL_UNSIGNED_INT_VEC3:                                     return DataType::UVec3;
            case GL_UNSIGNED_INT_VEC4:                                     return DataType::UVec4;
            case GL_BOOL:                                                  return DataType::Bool;
            case GL_BOOL_VEC2:                                             return DataType::BVec2;
            case GL_BOOL_VEC3:                                             return DataType::BVec3;
            case GL_BOOL_VEC4:                                             return DataType::BVec4;
            case GL_FLOAT_MAT2:                                            return DataType::Mat2;
            case GL_FLOAT_MAT3:                                            return DataType::Mat3;
            case GL_FLOAT_MAT4:                                            return DataType::Mat4;
            case GL_FLOAT_MAT2x3:                                          return DataType::Mat2x3;
            case GL_FLOAT_MAT2x4:                                          return DataType::Mat2x4;
            case GL_FLOAT_MAT3x2:                                          return DataType::Mat3x2;
            case GL_FLOAT_MAT3x4:                                          return DataType::Mat3x4;
            case GL_FLOAT_MAT4x2:                                          return DataType::Mat4x2;
            case GL_FLOAT_MAT4x3:                                          return DataType::Mat4x3;
            case GL_DOUBLE_MAT2:                                           return DataType::Dmat2;
            case GL_DOUBLE_MAT3:                                           return DataType::Dmat3;
            case GL_DOUBLE_MAT4:                                           return DataType::Dmat4;
            case GL_DOUBLE_MAT2x3:                                         return DataType::Dmat2x3;
            case GL_DOUBLE_MAT2x4:                                         return DataType::Dmat2x4;
            case GL_DOUBLE_MAT3x2:                                         return DataType::Dmat3x2;
            case GL_DOUBLE_MAT3x4:                                         return DataType::Dmat3x4;
            case GL_DOUBLE_MAT4x2:                                         return DataType::Dmat4x2;
            case GL_DOUBLE_MAT4x3:                                         return DataType::Dmat4x3;
            case GL_SAMPLER_1D:                                            return DataType::Sampler1D;
            case GL_SAMPLER_2D:                                            return DataType::Sampler2D;
            case GL_SAMPLER_3D:                                            return DataType::Sampler3D;
            case GL_SAMPLER_CUBE:                                          return DataType::SamplerCube;
            case GL_SAMPLER_1D_SHADOW:                                     return DataType::Sampler1DShadow;
            case GL_SAMPLER_2D_SHADOW:                                     return DataType::Sampler2DShadow;
            case GL_SAMPLER_1D_ARRAY:                                      return DataType::Sampler1DArray;
            case GL_SAMPLER_2D_ARRAY:                                      return DataType::Sampler2DArray;
            case GL_SAMPLER_1D_ARRAY_SHADOW:                               return DataType::Sampler1DArrayShadow;
            case GL_SAMPLER_2D_ARRAY_SHADOW:                               return DataType::Sampler2DArrayShadow;
            case GL_SAMPLER_2D_MULTISAMPLE:                                return DataType::Sampler2DMS;
            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:                          return DataType::Sampler2DMSArray;
            case GL_SAMPLER_CUBE_SHADOW:                                   return DataType::SamplerCubeShadow;
            case GL_SAMPLER_BUFFER:                                        return DataType::SamplerBuffer;
            case GL_SAMPLER_2D_RECT:                                       return DataType::Sampler2DRect;
            case GL_SAMPLER_2D_RECT_SHADOW:                                return DataType::Sampler2DRectShadow;
            case GL_INT_SAMPLER_1D:                                        return DataType::Isampler1D;
            case GL_INT_SAMPLER_2D:                                        return DataType::Isampler2D;
            case GL_INT_SAMPLER_3D:                                        return DataType::Isampler3D;
            case GL_INT_SAMPLER_CUBE:                                      return DataType::IsamplerCube;
            case GL_INT_SAMPLER_1D_ARRAY:                                  return DataType::Isampler1DArray;
            case GL_INT_SAMPLER_2D_ARRAY:                                  return DataType::Isampler2DArray;
            case GL_INT_SAMPLER_2D_MULTISAMPLE:                            return DataType::Isampler2DMS;
            case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:                      return DataType::Isampler2DMSArray;
            case GL_INT_SAMPLER_BUFFER:                                    return DataType::IsamplerBuffer;
            case GL_INT_SAMPLER_2D_RECT:                                   return DataType::Isampler2DRect;
            case GL_UNSIGNED_INT_SAMPLER_1D:                               return DataType::Usampler1D;
            case GL_UNSIGNED_INT_SAMPLER_2D:                               return DataType::Usampler2D;
            case GL_UNSIGNED_INT_SAMPLER_3D:                               return DataType::Usampler3D;
            case GL_UNSIGNED_INT_SAMPLER_CUBE:                             return DataType::UsamplerCube;
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:                         return DataType::Usampler2DArray;
            case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:                   return DataType::Usampler2DMS;
            case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:             return DataType::Usampler2DMSArray;
            case GL_UNSIGNED_INT_SAMPLER_BUFFER:                           return DataType::UsamplerBuffer;
            case GL_UNSIGNED_INT_SAMPLER_2D_RECT:                          return DataType::Usampler2DRect;
            default: ASSERT(false, "[OPENGL] Unknown DataType requested"); return DataType::Unknown;
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
    const char* get_name(DataType p_data_type)
    {
        switch (p_data_type)
        {
            case DataType::Float:                                          return "Float";
            case DataType::Vec2:                                           return "vec2";
            case DataType::Vec3:                                           return "vec3";
            case DataType::Vec4:                                           return "vec4";
            case DataType::Double:                                         return "Double";
            case DataType::DVec2:                                          return "DVec2";
            case DataType::DVec3:                                          return "DVec3";
            case DataType::DVec4:                                          return "DVec4";
            case DataType::Int:                                            return "Int";
            case DataType::IVec2:                                          return "IVec2";
            case DataType::IVec3:                                          return "IVec3";
            case DataType::IVec4:                                          return "IVec4";
            case DataType::UnsignedInt:                                    return "UnsignedInt";
            case DataType::UVec2:                                          return "UVec2";
            case DataType::UVec3:                                          return "UVec3";
            case DataType::UVec4:                                          return "UVec4";
            case DataType::Bool:                                           return "Bool";
            case DataType::BVec2:                                          return "BVec2";
            case DataType::BVec3:                                          return "BVec3";
            case DataType::BVec4:                                          return "BVec4";
            case DataType::Mat2:                                           return "Mat2";
            case DataType::Mat3:                                           return "Mat3";
            case DataType::Mat4:                                           return "Mat4";
            case DataType::Mat2x3:                                         return "Mat2x3";
            case DataType::Mat2x4:                                         return "Mat2x4";
            case DataType::Mat3x2:                                         return "Mat3x2";
            case DataType::Mat3x4:                                         return "Mat3x4";
            case DataType::Mat4x2:                                         return "Mat4x2";
            case DataType::Mat4x3:                                         return "Mat4x3";
            case DataType::Dmat2:                                          return "Dmat2";
            case DataType::Dmat3:                                          return "Dmat3";
            case DataType::Dmat4:                                          return "Dmat4";
            case DataType::Dmat2x3:                                        return "Dmat2x3";
            case DataType::Dmat2x4:                                        return "Dmat2x4";
            case DataType::Dmat3x2:                                        return "Dmat3x2";
            case DataType::Dmat3x4:                                        return "Dmat3x4";
            case DataType::Dmat4x2:                                        return "Dmat4x2";
            case DataType::Dmat4x3:                                        return "Dmat4x3";
            case DataType::Sampler1D:                                      return "Sampler1D";
            case DataType::Sampler2D:                                      return "Sampler2D";
            case DataType::Sampler3D:                                      return "Sampler3D";
            case DataType::SamplerCube:                                    return "SamplerCube";
            case DataType::Sampler1DShadow:                                return "Sampler1DShadow";
            case DataType::Sampler2DShadow:                                return "Sampler2DShadow";
            case DataType::Sampler1DArray:                                 return "Sampler1DArray";
            case DataType::Sampler2DArray:                                 return "Sampler2DArray";
            case DataType::Sampler1DArrayShadow:                           return "Sampler1DArrayShadow";
            case DataType::Sampler2DArrayShadow:                           return "Sampler2DArrayShadow";
            case DataType::Sampler2DMS:                                    return "Sampler2DMS";
            case DataType::Sampler2DMSArray:                               return "Sampler2DMSArray";
            case DataType::SamplerCubeShadow:                              return "SamplerCubeShadow";
            case DataType::SamplerBuffer:                                  return "SamplerBuffer";
            case DataType::Sampler2DRect:                                  return "Sampler2DRect";
            case DataType::Sampler2DRectShadow:                            return "Sampler2DRectShadow";
            case DataType::Isampler1D:                                     return "Isampler1D";
            case DataType::Isampler2D:                                     return "Isampler2D";
            case DataType::Isampler3D:                                     return "Isampler3D";
            case DataType::IsamplerCube:                                   return "IsamplerCube";
            case DataType::Isampler1DArray:                                return "Isampler1DArray";
            case DataType::Isampler2DArray:                                return "Isampler2DArray";
            case DataType::Isampler2DMS:                                   return "Isampler2DMS";
            case DataType::Isampler2DMSArray:                              return "Isampler2DMSArray";
            case DataType::IsamplerBuffer:                                 return "IsamplerBuffer";
            case DataType::Isampler2DRect:                                 return "Isampler2DRect";
            case DataType::Usampler1D:                                     return "Usampler1D";
            case DataType::Usampler2D:                                     return "Usampler2D";
            case DataType::Usampler3D:                                     return "Usampler3D";
            case DataType::UsamplerCube:                                   return "UsamplerCube";
            case DataType::Usampler2DArray:                                return "Usampler2DArray";
            case DataType::Usampler2DMS:                                   return "Usampler2DMS";
            case DataType::Usampler2DMSArray:                              return "Usampler2DMSArray";
            case DataType::UsamplerBuffer:                                 return "UsamplerBuffer";
            case DataType::Usampler2DRect:                                 return "Usampler2DRect";
            case DataType::Unknown:                                        return "Unknown";
            default: ASSERT(false, "[OPENGL] Unknown DataType requested"); return "";
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
    const char* get_name(CullFacesType p_cull_faces_type)
    {
        switch (p_cull_faces_type)
        {
            case CullFacesType::Back:                                           return "Back";
            case CullFacesType::Front:                                          return "Front";
            case CullFacesType::FrontAndBack:                                   return "Front and Back";
            default: ASSERT(false, "[OPENGL] Unknown CullFacesType requested"); return "";
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

    int convert(DataType p_data_type)
    {
        switch (p_data_type)
        {
            case DataType::Float:                                          return GL_FLOAT;
            case DataType::Vec2:                                           return GL_FLOAT_VEC2;
            case DataType::Vec3:                                           return GL_FLOAT_VEC3;
            case DataType::Vec4:                                           return GL_FLOAT_VEC4;
            case DataType::Double:                                         return GL_DOUBLE;
            case DataType::DVec2:                                          return GL_DOUBLE_VEC2;
            case DataType::DVec3:                                          return GL_DOUBLE_VEC3;
            case DataType::DVec4:                                          return GL_DOUBLE_VEC4;
            case DataType::Int:                                            return GL_INT;
            case DataType::IVec2:                                          return GL_INT_VEC2;
            case DataType::IVec3:                                          return GL_INT_VEC3;
            case DataType::IVec4:                                          return GL_INT_VEC4;
            case DataType::UnsignedInt:                                    return GL_UNSIGNED_INT;
            case DataType::UVec2:                                          return GL_UNSIGNED_INT_VEC2;
            case DataType::UVec3:                                          return GL_UNSIGNED_INT_VEC3;
            case DataType::UVec4:                                          return GL_UNSIGNED_INT_VEC4;
            case DataType::Bool:                                           return GL_BOOL;
            case DataType::BVec2:                                          return GL_BOOL_VEC2;
            case DataType::BVec3:                                          return GL_BOOL_VEC3;
            case DataType::BVec4:                                          return GL_BOOL_VEC4;
            case DataType::Mat2:                                           return GL_FLOAT_MAT2;
            case DataType::Mat3:                                           return GL_FLOAT_MAT3;
            case DataType::Mat4:                                           return GL_FLOAT_MAT4;
            case DataType::Mat2x3:                                         return GL_FLOAT_MAT2x3;
            case DataType::Mat2x4:                                         return GL_FLOAT_MAT2x4;
            case DataType::Mat3x2:                                         return GL_FLOAT_MAT3x2;
            case DataType::Mat3x4:                                         return GL_FLOAT_MAT3x4;
            case DataType::Mat4x2:                                         return GL_FLOAT_MAT4x2;
            case DataType::Mat4x3:                                         return GL_FLOAT_MAT4x3;
            case DataType::Dmat2:                                          return GL_DOUBLE_MAT2;
            case DataType::Dmat3:                                          return GL_DOUBLE_MAT3;
            case DataType::Dmat4:                                          return GL_DOUBLE_MAT4;
            case DataType::Dmat2x3:                                        return GL_DOUBLE_MAT2x3;
            case DataType::Dmat2x4:                                        return GL_DOUBLE_MAT2x4;
            case DataType::Dmat3x2:                                        return GL_DOUBLE_MAT3x2;
            case DataType::Dmat3x4:                                        return GL_DOUBLE_MAT3x4;
            case DataType::Dmat4x2:                                        return GL_DOUBLE_MAT4x2;
            case DataType::Dmat4x3:                                        return GL_DOUBLE_MAT4x3;
            case DataType::Sampler1D:                                      return GL_SAMPLER_1D;
            case DataType::Sampler2D:                                      return GL_SAMPLER_2D;
            case DataType::Sampler3D:                                      return GL_SAMPLER_3D;
            case DataType::SamplerCube:                                    return GL_SAMPLER_CUBE;
            case DataType::Sampler1DShadow:                                return GL_SAMPLER_1D_SHADOW;
            case DataType::Sampler2DShadow:                                return GL_SAMPLER_2D_SHADOW;
            case DataType::Sampler1DArray:                                 return GL_SAMPLER_1D_ARRAY;
            case DataType::Sampler2DArray:                                 return GL_SAMPLER_2D_ARRAY;
            case DataType::Sampler1DArrayShadow:                           return GL_SAMPLER_1D_ARRAY_SHADOW;
            case DataType::Sampler2DArrayShadow:                           return GL_SAMPLER_2D_ARRAY_SHADOW;
            case DataType::Sampler2DMS:                                    return GL_SAMPLER_2D_MULTISAMPLE;
            case DataType::Sampler2DMSArray:                               return GL_SAMPLER_2D_MULTISAMPLE_ARRAY;
            case DataType::SamplerCubeShadow:                              return GL_SAMPLER_CUBE_SHADOW;
            case DataType::SamplerBuffer:                                  return GL_SAMPLER_BUFFER;
            case DataType::Sampler2DRect:                                  return GL_SAMPLER_2D_RECT;
            case DataType::Sampler2DRectShadow:                            return GL_SAMPLER_2D_RECT_SHADOW;
            case DataType::Isampler1D:                                     return GL_INT_SAMPLER_1D;
            case DataType::Isampler2D:                                     return GL_INT_SAMPLER_2D;
            case DataType::Isampler3D:                                     return GL_INT_SAMPLER_3D;
            case DataType::IsamplerCube:                                   return GL_INT_SAMPLER_CUBE;
            case DataType::Isampler1DArray:                                return GL_INT_SAMPLER_1D_ARRAY;
            case DataType::Isampler2DArray:                                return GL_INT_SAMPLER_2D_ARRAY;
            case DataType::Isampler2DMS:                                   return GL_INT_SAMPLER_2D_MULTISAMPLE;
            case DataType::Isampler2DMSArray:                              return GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
            case DataType::IsamplerBuffer:                                 return GL_INT_SAMPLER_BUFFER;
            case DataType::Isampler2DRect:                                 return GL_INT_SAMPLER_2D_RECT;
            case DataType::Usampler1D:                                     return GL_UNSIGNED_INT_SAMPLER_1D;
            case DataType::Usampler2D:                                     return GL_UNSIGNED_INT_SAMPLER_2D;
            case DataType::Usampler3D:                                     return GL_UNSIGNED_INT_SAMPLER_3D;
            case DataType::UsamplerCube:                                   return GL_UNSIGNED_INT_SAMPLER_CUBE;
            case DataType::Usampler2DArray:                                return GL_UNSIGNED_INT_SAMPLER_2D_ARRAY;
            case DataType::Usampler2DMS:                                   return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
            case DataType::Usampler2DMSArray:                              return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
            case DataType::UsamplerBuffer:                                 return GL_UNSIGNED_INT_SAMPLER_BUFFER;
            case DataType::Usampler2DRect:                                 return GL_UNSIGNED_INT_SAMPLER_2D_RECT;
            default: ASSERT(false, "[OPENGL] Unknown DataType requested"); return 0;
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
    int convert(CullFacesType p_cull_faces_type)
    {
        switch (p_cull_faces_type)
        {
            case CullFacesType::Back:                                           return GL_BACK;
            case CullFacesType::Front:                                          return GL_FRONT;
            case CullFacesType::FrontAndBack:                                   return GL_FRONT_AND_BACK;
            default: ASSERT(false, "[OPENGL] Unknown CullFacesType requested"); return 0;
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