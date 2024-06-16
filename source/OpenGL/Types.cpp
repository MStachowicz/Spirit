#include "Types.hpp"

#include "Utility/Logger.hpp"
#include "Utility/File.hpp"

#include "glad/glad.h"

#include <utility>

namespace OpenGL
{
	Buffer::Buffer(BufferStorageBitfield p_flags)
		: m_handle{0}
		, m_size{0}
		, m_stride{0}
		, m_flags{p_flags}
	{
		glCreateBuffers(1, &m_handle);
	}
	Buffer::~Buffer()
	{
		if (m_handle != 0)
			glDeleteBuffers(1, &m_handle);

		State::Get().unbind_buffer(m_handle);
	}
	Buffer::Buffer(Buffer&& p_other)
		: m_handle{std::exchange(p_other.m_handle, 0)}
		, m_size{std::exchange(p_other.m_size, 0)}
		, m_stride{std::exchange(p_other.m_stride, 0)}
		, m_flags{p_other.m_flags}
	{}
	Buffer& Buffer::operator=(Buffer&& p_other)
	{
		if (this != &p_other)
		{
			// Free the existing resource.
			if (m_handle != 0)
				glDeleteBuffers(1, &m_handle);

			// Copy the data pointer from the source object.
			m_handle = std::exchange(p_other.m_handle, 0);
			m_size   = std::exchange(p_other.m_size, 0);
			m_stride = std::exchange(p_other.m_stride, 0);
			m_flags  = p_other.m_flags;
		}
		return *this;
	}
	void Buffer::resize(GLsizeiptr p_size)
	{
		ASSERT(is_immutable() == false, "Cannot resize an immutable buffer");

		named_buffer_storage(m_handle, p_size, nullptr, m_flags);
		m_size = p_size;
	}
	void Buffer::clear()
	{
		m_size   = 0;
		m_stride = 0;
		named_buffer_storage(m_handle, 0, nullptr, m_flags);
	}
	bool Buffer::is_immutable() const
	{
		GLint is_immutable = 0;
		glGetNamedBufferParameteriv(m_handle, GL_BUFFER_IMMUTABLE_STORAGE, &is_immutable);
		return is_immutable == GL_TRUE;
	}

	VAO::VAO()
		: m_handle{0}
		, m_draw_count{0}
		, m_draw_primitive_mode{PrimitiveMode::Triangles}
		, m_is_indexed{false}
	{
		glCreateVertexArrays(1, &m_handle);
		if constexpr (LogGLTypeEvents) LOG("VAO constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	VAO::~VAO()
	{
		// We initialise m_handle to 0 and reset to 0 on move and depend on this to check if there is a vertex array to delete here.
		// We still want to call glDeleteVertexArrays if the array was generated but not used with bind and/or VBOs hence
		// not using glIsVertexArray here.
		if (m_handle != 0)
			glDeleteVertexArrays(1, &m_handle);

		if constexpr (LogGLTypeEvents) LOG("VAO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	VAO::VAO(VAO&& p_other)
		: m_handle{std::move(p_other.m_handle)}
		, m_draw_count{std::move(p_other.m_draw_count)}
		, m_draw_primitive_mode{std::move(p_other.m_draw_primitive_mode)}
		, m_is_indexed{std::move(p_other.m_is_indexed)}
	{ // Steal the handle of the other VAO and set it to 0 to prevent the p_other destructor calling glDeleteVertexArrays
		p_other.m_handle = 0;

		if constexpr (LogGLTypeEvents) LOG("VAO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	VAO& VAO::operator=(VAO&& p_other)
	{
		if (this != &p_other)
		{
			// Free the existing resource.
			if (m_handle != 0)
				glDeleteVertexArrays(1, &m_handle);

			// Copy the data pointer from the source object.
			m_handle              = p_other.m_handle;
			m_draw_count          = p_other.m_draw_count;
			m_draw_primitive_mode = p_other.m_draw_primitive_mode;
			m_is_indexed          = p_other.m_is_indexed;
			// Release the handle so ~VAO doesnt call glDeleteVertexArrays on m_handle.
			p_other.m_handle = 0;
		}

		if constexpr (LogGLTypeEvents) LOG("VAO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
		return *this;
	}
	void VAO::attach_buffer(Buffer& p_vertex_buffer, GLintptr p_vertex_buffer_offset, GLuint p_vertex_buffer_binding_point, GLsizei p_stride)
	{
		vertex_array_vertex_buffer(m_handle, p_vertex_buffer_binding_point, p_vertex_buffer.m_handle, p_vertex_buffer_offset, p_stride);
		if (!m_is_indexed)
			m_draw_count = p_vertex_buffer.count();
	}
	void VAO::attach_element_buffer(Buffer& p_element_buffer)
	{
		vertex_array_element_buffer(m_handle, p_element_buffer.m_handle);
		m_is_indexed = true;
		m_draw_count = p_element_buffer.count();
	}
	void VAO::set_vertex_attrib_pointers(PrimitiveMode p_primitive_mode, const std::vector<VertexAttributeMeta>& attributes)
	{
		m_draw_primitive_mode = p_primitive_mode;

		// Loop through each attribute and set up vertex attribute format
		for (const auto& attribute : attributes)
		{
			// Enable the vertex attribute array.
			glEnableVertexArrayAttrib(m_handle, attribute.index);
			// Specify the binding index of the vertex buffer for the attribute.
			glVertexArrayAttribBinding(m_handle, attribute.index, attribute.vertex_buffer_binding_point);
			// Set the format of the attribute.
			if (attribute.type == BufferDataType::Int || attribute.type == BufferDataType::UnsignedInt)
				glVertexArrayAttribIFormat(m_handle, attribute.index, attribute.size, convert(attribute.type), attribute.relative_offset);
			else if (attribute.type == BufferDataType::Double)
				glVertexArrayAttribLFormat(m_handle, attribute.index, attribute.size, convert(attribute.type), attribute.relative_offset);
			else
				glVertexArrayAttribFormat(m_handle, attribute.index, attribute.size, convert(attribute.type), attribute.normalized ? GL_TRUE : GL_FALSE, attribute.relative_offset);
		}
	}

	GLenum convert(TextureMagFunc p_magnification_function)
	{
		switch (p_magnification_function)
		{
			case TextureMagFunc::Nearest: return GL_NEAREST;
			case TextureMagFunc::Linear:  return GL_LINEAR;
			default: ASSERT(false, "Unknown TextureMagFunc function"); return 0;
		}
	}
	GLenum convert(WrappingMode p_wrapping_mode)
	{
		switch (p_wrapping_mode)
		{
			case WrappingMode::Repeat:            return GL_REPEAT;
			case WrappingMode::MirroredRepeat:    return GL_MIRRORED_REPEAT;
			case WrappingMode::ClampToEdge:       return GL_CLAMP_TO_EDGE;
			case WrappingMode::ClampToBorder:     return GL_CLAMP_TO_BORDER;
			case WrappingMode::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
			default: ASSERT(false, "Unknown WrappingMode function"); return 0;
		}
	}
	GLenum convert(TextureFormat p_format)
	{
		switch (p_format)
		{
			case TextureFormat::R:              return GL_RED;
			case TextureFormat::RG:             return GL_RG;
			case TextureFormat::RGB:            return GL_RGB;
			case TextureFormat::BGR:            return GL_BGR;
			case TextureFormat::RGBA:           return GL_RGBA;
			case TextureFormat::BGRA:           return GL_BGRA;
			case TextureFormat::DepthComponent: return GL_DEPTH_COMPONENT;
			case TextureFormat::StencilIndex:   return GL_STENCIL_INDEX;
			default: ASSERT(false, "Unknown TextureInternalFormat function"); return 0;
		}
	}
	GLenum convert(TextureInternalFormat p_internal_format)
	{
		switch (p_internal_format)
		{
			case TextureInternalFormat::R8:                 return GL_R8;
			case TextureInternalFormat::R8_SNORM:           return GL_R8_SNORM;
			case TextureInternalFormat::R16:                return GL_R16;
			case TextureInternalFormat::R16_SNORM:          return GL_R16_SNORM;
			case TextureInternalFormat::RG8:                return GL_RG8;
			case TextureInternalFormat::RG8_SNORM:          return GL_RG8_SNORM;
			case TextureInternalFormat::RG16:               return GL_RG16;
			case TextureInternalFormat::RG16_SNORM:         return GL_RG16_SNORM;
			case TextureInternalFormat::R3_G3_B2:           return GL_R3_G3_B2;
			case TextureInternalFormat::RGB4:               return GL_RGB4;
			case TextureInternalFormat::RGB5:               return GL_RGB5;
			case TextureInternalFormat::RGB8:               return GL_RGB8;
			case TextureInternalFormat::RGB8_SNORM:         return GL_RGB8_SNORM;
			case TextureInternalFormat::RGB10:              return GL_RGB10;
			case TextureInternalFormat::RGB12:              return GL_RGB12;
			case TextureInternalFormat::RGB16_SNORM:        return GL_RGB16_SNORM;
			case TextureInternalFormat::RGBA2:              return GL_RGBA2;
			case TextureInternalFormat::RGBA4:              return GL_RGBA4;
			case TextureInternalFormat::RGB5_A1:            return GL_RGB5_A1;
			case TextureInternalFormat::RGBA8:              return GL_RGBA8;
			case TextureInternalFormat::RGBA8_SNORM:        return GL_RGBA8_SNORM;
			case TextureInternalFormat::RGB10_A2:           return GL_RGB10_A2;
			case TextureInternalFormat::RGB10_A2UI:         return GL_RGB10_A2UI;
			case TextureInternalFormat::RGBA12:             return GL_RGBA12;
			case TextureInternalFormat::RGBA16:             return GL_RGBA16;
			case TextureInternalFormat::SRGB8:              return GL_SRGB8;
			case TextureInternalFormat::SRGB8_ALPHA8:       return GL_SRGB8_ALPHA8;
			case TextureInternalFormat::R16F:               return GL_R16F;
			case TextureInternalFormat::RG16F:              return GL_RG16F;
			case TextureInternalFormat::RGB16F:             return GL_RGB16F;
			case TextureInternalFormat::RGBA16F:            return GL_RGBA16F;
			case TextureInternalFormat::R32F:               return GL_R32F;
			case TextureInternalFormat::RG32F:              return GL_RG32F;
			case TextureInternalFormat::RGB32F:             return GL_RGB32F;
			case TextureInternalFormat::RGBA32F:            return GL_RGBA32F;
			case TextureInternalFormat::R11F_G11F_B10F:     return GL_R11F_G11F_B10F;
			case TextureInternalFormat::RGB9_E5:            return GL_RGB9_E5;
			case TextureInternalFormat::R8I:                return GL_R8I;
			case TextureInternalFormat::R8UI:               return GL_R8UI;
			case TextureInternalFormat::R16I:               return GL_R16I;
			case TextureInternalFormat::R16UI:              return GL_R16UI;
			case TextureInternalFormat::R32I:               return GL_R32I;
			case TextureInternalFormat::R32UI:              return GL_R32UI;
			case TextureInternalFormat::RG8I:               return GL_RG8I;
			case TextureInternalFormat::RG8UI:              return GL_RG8UI;
			case TextureInternalFormat::RG16I:              return GL_RG16I;
			case TextureInternalFormat::RG16UI:             return GL_RG16UI;
			case TextureInternalFormat::RG32I:              return GL_RG32I;
			case TextureInternalFormat::RG32UI:             return GL_RG32UI;
			case TextureInternalFormat::RGB8I:              return GL_RGB8I;
			case TextureInternalFormat::RGB8UI:             return GL_RGB8UI;
			case TextureInternalFormat::RGB16I:             return GL_RGB16I;
			case TextureInternalFormat::RGB16UI:            return GL_RGB16UI;
			case TextureInternalFormat::RGB32I:             return GL_RGB32I;
			case TextureInternalFormat::RGB32UI:            return GL_RGB32UI;
			case TextureInternalFormat::RGBA8I:             return GL_RGBA8I;
			case TextureInternalFormat::RGBA8UI:            return GL_RGBA8UI;
			case TextureInternalFormat::RGBA16I:            return GL_RGBA16I;
			case TextureInternalFormat::RGBA16UI:           return GL_RGBA16UI;
			case TextureInternalFormat::RGBA32I:            return GL_RGBA32I;
			case TextureInternalFormat::RGBA32UI:           return GL_RGBA32UI;
			case TextureInternalFormat::DEPTH_COMPONENT32F: return GL_DEPTH_COMPONENT32F;
			case TextureInternalFormat::DEPTH_COMPONENT24:  return GL_DEPTH_COMPONENT24;
			case TextureInternalFormat::DEPTH_COMPONENT16:  return GL_DEPTH_COMPONENT16;
			case TextureInternalFormat::DEPTH32F_STENCIL8:  return GL_DEPTH32F_STENCIL8;
			case TextureInternalFormat::DEPTH24_STENCIL8:   return GL_DEPTH24_STENCIL8;
			case TextureInternalFormat::STENCIL_INDEX8:     return GL_STENCIL_INDEX8;
			default: ASSERT(false, "Unknown TextureInternalFormat function"); return 0;
		}
	};
	GLenum convert(TextureDataType p_texture_data_type)
	{
		switch (p_texture_data_type)
		{
			case TextureDataType::UNSIGNED_BYTE:               return GL_UNSIGNED_BYTE;
			case TextureDataType::BYTE:                        return GL_BYTE;
			case TextureDataType::UNSIGNED_SHORT:              return GL_UNSIGNED_SHORT;
			case TextureDataType::SHORT:                       return GL_SHORT;
			case TextureDataType::UNSIGNED_INT:                return GL_UNSIGNED_INT;
			case TextureDataType::INT:                         return GL_INT;
			case TextureDataType::FLOAT:                       return GL_FLOAT;
			case TextureDataType::UNSIGNED_BYTE_3_3_2:         return GL_UNSIGNED_BYTE_3_3_2;
			case TextureDataType::UNSIGNED_BYTE_2_3_3_REV:     return GL_UNSIGNED_BYTE_2_3_3_REV;
			case TextureDataType::UNSIGNED_SHORT_5_6_5:        return GL_UNSIGNED_SHORT_5_6_5;
			case TextureDataType::UNSIGNED_SHORT_5_6_5_REV:    return GL_UNSIGNED_SHORT_5_6_5_REV;
			case TextureDataType::UNSIGNED_SHORT_4_4_4_4:      return GL_UNSIGNED_SHORT_4_4_4_4;
			case TextureDataType::UNSIGNED_SHORT_4_4_4_4_REV:  return GL_UNSIGNED_SHORT_4_4_4_4_REV;
			case TextureDataType::UNSIGNED_SHORT_5_5_5_1:      return GL_UNSIGNED_SHORT_5_5_5_1;
			case TextureDataType::UNSIGNED_SHORT_1_5_5_5_REV:  return GL_UNSIGNED_SHORT_1_5_5_5_REV;
			case TextureDataType::UNSIGNED_INT_8_8_8_8:        return GL_UNSIGNED_INT_8_8_8_8;
			case TextureDataType::UNSIGNED_INT_8_8_8_8_REV:    return GL_UNSIGNED_INT_8_8_8_8_REV;
			case TextureDataType::UNSIGNED_INT_10_10_10_2:     return GL_UNSIGNED_INT_10_10_10_2;
			case TextureDataType::UNSIGNED_INT_2_10_10_10_REV: return GL_UNSIGNED_INT_2_10_10_10_REV;
			default: ASSERT(false, "Unknown TextureDataType function"); return 0;
		}
	};

	Texture::Texture(const glm::uvec2& p_resolution,
	                 TextureMagFunc p_magnification_function,
	                 WrappingMode p_wrapping_mode,
	                 TextureInternalFormat p_internal_format)
	    : m_handle{0}
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);

		glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, convert(p_magnification_function));
		glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, convert(p_magnification_function));
		glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, convert(p_wrapping_mode));
		glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, convert(p_wrapping_mode));

		constexpr GLint levels = 1;
		glTextureStorage2D(m_handle, levels, convert(p_internal_format), p_resolution.x, p_resolution.y);

		if constexpr (LogGLTypeEvents) LOG("Texture constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	Texture::Texture(const glm::uvec2& p_resolution,
	                 TextureMagFunc p_magnification_function,
	                 WrappingMode p_wrapping_mode,
	                 TextureInternalFormat p_internal_format,
	                 TextureFormat p_format,
	                 TextureDataType p_data_type,
	                 bool generate_mip_map,
	                 const void* p_pixel_data)
	    : m_handle{0}
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);

		glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, convert(p_magnification_function));
		glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, convert(p_magnification_function));
		glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, convert(p_wrapping_mode));
		glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, convert(p_wrapping_mode));

		// For valid format combinations see https://www.khronos.org/opengl/wiki/Image_Format#Required_formats
		constexpr GLint levels = 1;
		glTextureStorage2D(m_handle, levels, convert(p_internal_format), p_resolution.x, p_resolution.y);

		constexpr GLint level = 0;
		constexpr glm::ivec2 offset = {0, 0};
		glTextureSubImage2D(m_handle, level, offset.x, offset.y, p_resolution.x, p_resolution.y, convert(p_format), convert(p_data_type), p_pixel_data);

		if (generate_mip_map)
			glGenerateTextureMipmap(m_handle);

		if constexpr (LogGLTypeEvents) LOG("Texture constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	Texture::~Texture()
	{
		if (m_handle != 0)
			glDeleteTextures(1, &m_handle);

		if constexpr (LogGLTypeEvents) LOG("Texture destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	Texture::Texture(Texture&& p_other)
		: m_handle{std::move(p_other.m_handle)}
	{
		p_other.m_handle = 0;

		if constexpr (LogGLTypeEvents) LOG("Texture move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	Texture& Texture::operator=(Texture&& p_other)
	{
		if (this != &p_other)
		{
			// Free the existing resource.
			if (m_handle != 0)
				glDeleteTextures(1, &m_handle);

			// Copy the data pointer from the source object.
			m_handle = p_other.m_handle;
			// Release the handle so ~Texture doesnt call glDeleteBuffers on m_handle.
			p_other.m_handle = 0;
		}

		if constexpr (LogGLTypeEvents) LOG("Texture move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
		return *this;
	}




	FBO::FBO(const glm::uvec2& p_resolution, bool p_colour_attachment, bool p_depth_attachment, bool p_stencil_attachment)
		: m_handle{0}
		, m_resolution{p_resolution}
		, m_clear_colour{0.f, 0.f, 0.f, 1.f}
		, m_colour_attachment{}
		, m_depth_attachment{}
		, m_stencil_attachment{}
		, m_depth_stencil_attachment{}
	{
		glCreateFramebuffers(1, &m_handle);

		constexpr GLint level = 0;
		if (p_colour_attachment)
		{
			m_colour_attachment = Texture(p_resolution, TextureMagFunc::Linear, WrappingMode::ClampToBorder, TextureInternalFormat::RGBA8);
			glNamedFramebufferTexture(m_handle, GL_COLOR_ATTACHMENT0, m_colour_attachment->m_handle, level);
		}

		if (p_depth_attachment && p_stencil_attachment)
		{
			// If we want both depth and stencil, we use a combined depth/stencil attachment.
			// Seems support for attaching both depth and stencil separately is not guaranteed.
			m_depth_stencil_attachment = Texture(p_resolution, TextureMagFunc::Nearest, WrappingMode::ClampToEdge, TextureInternalFormat::DEPTH32F_STENCIL8);
			glNamedFramebufferTexture(m_handle, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil_attachment->m_handle, level);
		}
		else
		{
			if (p_depth_attachment)
			{
				m_depth_attachment = Texture(p_resolution, TextureMagFunc::Nearest, WrappingMode::ClampToEdge, TextureInternalFormat::DEPTH_COMPONENT32F);
				glNamedFramebufferTexture(m_handle, GL_DEPTH_ATTACHMENT, m_depth_attachment->m_handle, level);
			}
			if (p_stencil_attachment)
			{
				m_stencil_attachment = Texture(p_resolution, TextureMagFunc::Nearest, WrappingMode::ClampToEdge, TextureInternalFormat::STENCIL_INDEX8);
				glNamedFramebufferTexture(m_handle, GL_STENCIL_ATTACHMENT, m_stencil_attachment->m_handle, level);
			}
		}
		ASSERT_THROW(is_complete(), "Framebuffer is not complete!");

		if constexpr (LogGLTypeEvents) LOG("FBO constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	FBO::~FBO()
	{
		if (m_handle != 0)
			glDeleteFramebuffers(1, &m_handle);

		if constexpr (LogGLTypeEvents) LOG("FBO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	FBO::FBO(FBO&& p_other)
		: m_handle{std::move(p_other.m_handle)}
		, m_resolution{std::move(p_other.m_resolution)}
		, m_clear_colour{std::move(p_other.m_clear_colour)}
		, m_colour_attachment{std::move(p_other.m_colour_attachment)}
		, m_depth_attachment{std::move(p_other.m_depth_attachment)}
		, m_stencil_attachment{std::move(p_other.m_stencil_attachment)}
		, m_depth_stencil_attachment{std::move(p_other.m_depth_stencil_attachment)}
	{
		p_other.m_handle = 0;

		if constexpr (LogGLTypeEvents) LOG("FBO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	FBO& FBO::operator=(FBO&& p_other)
	{
		if (this != &p_other)
		{
			// Free the existing resources.
			if (m_handle != 0)
				glDeleteFramebuffers(1, &m_handle);

			m_handle = p_other.m_handle;
			p_other.m_handle = 0;

			m_resolution         = std::move(p_other.m_resolution);
			m_clear_colour       = std::move(p_other.m_clear_colour);
			m_colour_attachment  = std::move(p_other.m_colour_attachment);
			m_depth_attachment   = std::move(p_other.m_depth_attachment);
			m_stencil_attachment = std::move(p_other.m_stencil_attachment);
		}

		if constexpr (LogGLTypeEvents) LOG("FBO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
		return *this;
	}
	void FBO::clear() const
	{
		if (m_colour_attachment)
		{
			glClearNamedFramebufferfv(m_handle, GL_COLOR, 0, &m_clear_colour[0]);
		}

		if (m_depth_stencil_attachment)
		{
			constexpr GLfloat depth   = 1.0f; // Farthest depth value, range [0, 1]
			constexpr GLint stencil   = 0;
			State::Get().set_depth_write(true); // GL requires depth write to be enabled for clearing the depth buffer. oof
			glClearNamedFramebufferfi(m_handle, GL_DEPTH_STENCIL, 0, depth, stencil);
		}
		else
		{
			if (m_depth_attachment)
			{
				constexpr GLfloat depth   = 1.0f; // Farthest depth value, range [0, 1]
				State::Get().set_depth_write(true); // GL requires depth write to be enabled for clearing the depth buffer. oof
				glClearNamedFramebufferfv(m_handle, GL_DEPTH, 0, &depth);
			}
			if (m_stencil_attachment)
			{
				constexpr GLint stencil   = 0;
				glClearNamedFramebufferiv(m_handle, GL_STENCIL, 0, &stencil);
			}
		}
	}
	void FBO::clear_default_framebuffer(const glm::vec4& p_clear_colour)
	{
		State::Get().set_depth_write(true); // GL requires depth write to be enabled for clearing the depth buffer. oof
		constexpr GLint drawbuffer    = 0;
		constexpr GLfloat clear_depth = 1.0f; // Clear depth to farthest (1.0)
		constexpr GLint clear_stencil = 0;    // Clear stencil to 0
		glClearNamedFramebufferfv(0, GL_COLOR, drawbuffer, &p_clear_colour[0]);
		glClearNamedFramebufferfi(0, GL_DEPTH_STENCIL, drawbuffer, clear_depth, clear_stencil);
	}
	void FBO::resize(const glm::uvec2& p_resolution)
	{
		if (p_resolution == m_resolution)
			return;

		m_resolution = p_resolution;

		constexpr GLint level = 0;
		if (m_colour_attachment)
		{
			m_colour_attachment = Texture(p_resolution, TextureMagFunc::Linear, WrappingMode::ClampToBorder, TextureInternalFormat::RGBA8);
			glNamedFramebufferTexture(m_handle, GL_COLOR_ATTACHMENT0, m_colour_attachment->m_handle, level);
		}

		if (m_depth_stencil_attachment)
		{
			m_depth_stencil_attachment = Texture(p_resolution, TextureMagFunc::Nearest, WrappingMode::ClampToEdge, TextureInternalFormat::DEPTH32F_STENCIL8);
			glNamedFramebufferTexture(m_handle, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil_attachment->m_handle, level);
		}
		else
		{
			if (m_depth_attachment)
			{
				m_depth_attachment = Texture(p_resolution, TextureMagFunc::Nearest, WrappingMode::ClampToEdge, TextureInternalFormat::DEPTH_COMPONENT32F);
				glNamedFramebufferTexture(m_handle, GL_DEPTH_ATTACHMENT, m_depth_attachment->m_handle, level);
			}
			if (m_stencil_attachment)
			{
				m_stencil_attachment = Texture(p_resolution, TextureMagFunc::Nearest, WrappingMode::ClampToEdge, TextureInternalFormat::STENCIL_INDEX8);
				glNamedFramebufferTexture(m_handle, GL_STENCIL_ATTACHMENT, m_stencil_attachment->m_handle, level);
			}
		}
		ASSERT_THROW(is_complete(), "Framebuffer is not complete!");

		if constexpr (LogGLTypeEvents) LOG("FBO resized with GLHandle {} at address {}", m_handle, (void*)(this));
	}
	bool FBO::is_complete() const
	{
		// https://www.khronos.org/opengl/wiki/Framebuffer_Object#Framebuffer_Completeness
		return glCheckNamedFramebufferStatus(m_handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}
} // namespace OpenGL