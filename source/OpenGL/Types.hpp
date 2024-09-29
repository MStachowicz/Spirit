// Types uses GLState to define helper functions that encapsulate commone usage of GL functions.
// Types uses raw OpenGL functions in the cpp where we can trust the usage is correct.
// They are RAII wrappers handling the memory owned and stored by OpenGL.
#pragma once

#include "GLState.hpp"

#include "Utility/Logger.hpp"

#include <glm/glm.hpp>

#include <array>
#include <limits>
#include <optional>
#include <vector>

namespace OpenGL
{
	// Represents a block of memory on the GPU.
	// The buffer can be used to store any data. It is up to the user to interpret the data correctly using a VAO.
	class Buffer
	{
		friend class VAO;
		friend class DrawCall;

		GLHandle m_handle;
		GLsizeiptr m_size; // Size in bytes of the buffer. i.e. the amount of GPU memory the whole buffer holds.
		GLsizei m_stride;  // Stride in bytes between consecutive elements in the buffer. i.e. the amount of GPU memory each element takes up.
		BufferStorageBitfield m_flags;

	public:
		Buffer(BufferStorageBitfield p_flags);
		~Buffer();

		Buffer(const Buffer& p_other);
		Buffer& operator=(const Buffer& p_other);
		Buffer(Buffer&& p_other);
		Buffer& operator=(Buffer&& p_other);

		// Creates and initializes a buffer object's immutable data store.
		//@param p_data Vector of data to copy to the buffer.
		template<typename T>
		void upload_data(const std::vector<T>& p_data)
		{
			ASSERT_THROW(p_data.size() <= std::numeric_limits<GLsizei>::max(), "Too many elements in vector to fit in a GLsizei.");

			m_size   = p_data.size() * sizeof(T);
			m_stride = sizeof(T);
			named_buffer_storage(m_handle, m_size, p_data.data(), m_flags);
		}
		// Creates and initializes a buffer object's immutable data store.
		//@param p_data Array of data to copy to the buffer.
		template<typename T, size_t N>
		void upload_data(const std::array<T, N>& p_data)
		{
			static_assert(N <= std::numeric_limits<GLsizei>::max(), "Too many elements in array to fit in a GLsizei.");

			m_size   = N * sizeof(T);
			m_stride = sizeof(T);
			named_buffer_storage(m_handle, m_size, p_data.data(), m_flags);
		}
		template<typename T>
		std::vector<T> download_data(size_t p_count) const
		{
			std::vector<T> data(p_count);
			get_named_buffer_sub_data(m_handle, 0, m_size, data.data());
			return data;
		}
		// Update a subset of a Buffer object's data store
		template<typename T>
		void buffer_sub_data(GLintptr p_offset, const T& p_data)
		{
			// assert T is not a pointer or reference type.
			static_assert(!std::is_pointer_v<T>, "T must not be a pointer type.");
			static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
			static_assert(std::is_standard_layout_v<T>, "T must be a standard layout type.");
			ASSERT(m_size >= p_offset + (GLsizeiptr)(sizeof(T)), "Buffer sub data out of bounds. Have you set the buffer size using upload_data or resize?");

			named_buffer_sub_data(m_handle, p_offset, sizeof(T), &p_data);
		}
		// Update a subset of a Buffer object's data store
		template<typename T>
		void buffer_sub_data(GLintptr p_offset, const std::vector<T>& p_data)
		{
			// assert T is not a pointer or reference type.
			static_assert(!std::is_pointer_v<T>, "T must not be a pointer type.");
			static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
			static_assert(std::is_standard_layout_v<T>, "T must be a standard layout type.");
			ASSERT(m_size >= p_offset + (GLsizeiptr)(sizeof(T) * p_data.size()), "Buffer sub data out of bounds. Have you set the buffer size using upload_data or resize?");

			named_buffer_sub_data(m_handle, p_offset, (GLsizeiptr)(sizeof(T) * p_data.size()), p_data.data());
		}
		// Mat4 specialisation of buffer_sub_data
		void buffer_sub_data(GLintptr p_offset, const glm::mat4& p_data)
		{
			ASSERT(m_size >= p_offset + (GLsizeiptr)(sizeof(glm::mat4)), "Buffer sub data out of bounds. Have you set the buffer size using upload_data or resize?");
			named_buffer_sub_data(m_handle, p_offset, sizeof(glm::mat4), &p_data[0][0]);
		}
		// Bool specialisation of buffer_sub_data
		void buffer_sub_data(GLintptr p_offset, bool p_data)
		{
			// GLSL bools are 4 bytes in size.
			GLint gl_bool = p_data;
			buffer_sub_data(p_offset, gl_bool);
		}
		// Copy a portion of p_source_buffer from p_source_offset into this buffer at p_destination_offset of p_size.
		void copy_sub_data(const Buffer& p_source_buffer, GLint p_source_offset, GLint p_destination_offset, GLsizeiptr p_size)
		{
			if (p_size > 0)
				copy_named_buffer_sub_data(p_source_buffer.m_handle, m_handle, p_source_offset, p_destination_offset, p_size);
		}

		// Resizes the buffer object's data store. All existing data is lost.
		void resize(GLsizeiptr p_size);
		// Clears the buffer object's data store. All existing data is lost.
		void clear();
		GLsizeiptr size() const { return m_size; }
		GLsizei stride()  const { return m_stride; }
		bool is_immutable() const;
	};
	// Meta struct to hold information about a vertex attribute.
	// Every attribute in a VAO must have a corresponding VertexAttributeMeta which describes how and where to interpret the data.
	struct VertexAttributeMeta
	{
		GLuint index;                       // Index of the attribute. Can be queried from a Shader program using glGetAttribLocation. Must be less than GL_MAX_VERTEX_ATTRIBS.
		GLint size;                         // Number of attribute per attribute (e.g., vec2 = 2, vec3 = 3) must be 1, 2, 3, 4 or GL_BGRA.
		BufferDataType type;                // Data type of the attribute. (e.g. double = double, vec3 = float, ivec4 = int, etc.)
		GLuint relative_offset;             // Offset from the beginning of the buffer to the start of this attribute.
		GLuint vertex_buffer_binding_point; // The vertex buffer binding point of the vertex buffer that contains the attribute data.
		bool normalized;                    // Whether to normalize the data to -1.0 to 1.0 (signed) or 0.0 to 1.0 (unsigned).
	};

	// Vertex Array Object (VAO)
	// Stores all of the state needed to make a draw call.
	// It stores non-owning references to the buffers that contain the vertex data and vertex_index_data.
	class VAO
	{
		friend class DrawCall;

		GLHandle m_handle;
		GLsizei m_draw_count; // Number of vertices to draw. If an index buffer is attached, this is the number of indices to draw.
		PrimitiveMode m_draw_primitive_mode;
		bool m_is_indexed; // Whether an index buffer was attached to the VAO.
	public:
		VAO();
		~VAO();
		VAO(const VAO& p_other)            = delete;
		VAO& operator=(const VAO& p_other) = delete;
		VAO(VAO&& p_other);
		VAO& operator=(VAO&& p_other);

		// Set how the VAO will interpret the vertex data in the attached p_vertex_buffer.
		// Must be set before using the VAO for draw call.
		void set_vertex_attrib_pointers(PrimitiveMode p_primitive_mode, const std::vector<VertexAttributeMeta>& attributes);
		// Bind p_vertex_buffer to the p_vertex_buffer_binding_point of the VAO. Does not modify the global GL state.
		//@param p_vertex_buffer The vertex buffer object (VBO) to attach to the VAO for reading vertex attribute data.
		//@param p_vertex_buffer_offset The offset in bytes from the start of the p_vertex_buffer to the start of the vertex data.
		//@param p_vertex_buffer_binding_point The vertex buffer binding point of the VAO to bind the buffer to.
		//@param p_stride The stride in bytes between consecutive vertices in the buffer.
		void attach_buffer(Buffer& p_vertex_buffer, GLintptr p_vertex_buffer_offset, GLuint p_vertex_buffer_binding_point, GLsizei p_stride);
		// Binds p_element_buffer to the VAO. Does not modify the global GL state.
		//@param p_element_buffer The element buffer object (EBO) to attach to the VAO for reading index data of the attached vertex_buffer.
		//@param p_element_count The number of indices in p_element_buffer.
		void attach_element_buffer(Buffer& p_element_buffer, GLsizei p_element_count);
		// Whether the VAO has an index buffer.
		bool is_indexed() const { return m_is_indexed; }
		// Number of vertices to draw. If an index buffer is attached, this is the number of indices to draw.
		GLsizei draw_count() const { return m_draw_count; }
		// The primitive mode to draw the vertices in.
		PrimitiveMode draw_primitive_mode() const { return m_draw_primitive_mode; }
	};

	class Texture
	{
		friend class DrawCall;
		friend class FBO;

		GLHandle m_handle;
	public:
		// Create a texture object with the specified resolution with no pixel data.
		//@param p_resolution The resolution of the texture.
		//@param p_number_of_channels The number of channels in the texture. Must be 1, 2, 3, or 4.
		//@param p_magnification_function The function to use when magnifying the texture.
		//@param p_wrapping_mode The wrapping mode to use for the texture.
		//@param p_internal_format Specifies the sized internal format to be used to store texture image data.
		Texture(const glm::uvec2& p_resolution,
	                 TextureMagFunc p_magnification_function,
	                 WrappingMode p_wrapping_mode,
	                 TextureInternalFormat p_internal_format);

		// Create a texture object with the specified resolution. p_pixel_data is copied to the texture.
		//@param p_resolution The resolution of the texture.
		//@param p_number_of_channels The number of channels in the texture. Must be 1, 2, 3, or 4.
		//@param p_magnification_function The function to use when magnifying the texture.
		//@param p_wrapping_mode The wrapping mode to use for the texture.
		//@param p_internal_format Specifies the sized internal format to be used to store texture image data.
		//@param p_format Specifies the format of p_pixel_data.
		//@param p_data_type Specifies the data type of p_pixel_data.
		//@param generate_mip_map Whether to generate mipmaps for the texture.
		//@param p_pixel_data The pixel data to copy to the texture.
		Texture(const glm::uvec2& p_resolution,
		        TextureMagFunc p_magnification_function,
		        WrappingMode p_wrapping_mode,
		        TextureInternalFormat p_internal_format,
		        TextureFormat p_format,
		        TextureDataType p_data_type,
		        bool p_generate_mip_map,
		        const void* p_pixel_data);

		~Texture();

		Texture(const Texture& p_other) = delete;
		Texture& operator=(const Texture& p_other) = delete;
		Texture(Texture&& p_other);
		Texture& operator=(Texture&& p_other);

		GLHandle handle() const { return m_handle; }
	};


	// Framebuffer object.
	// Allows creation of user-defined framebuffers that can be rendered to without disturbing the main screen.
	class FBO
	{
		friend class DrawCall;

		GLHandle m_handle;
		glm::uvec2 m_resolution;
		glm::vec4 m_clear_colour;
		std::optional<Texture> m_colour_attachment;
		std::optional<Texture> m_depth_attachment;
		std::optional<Texture> m_stencil_attachment;
		std::optional<Texture> m_depth_stencil_attachment;
	public:
		static void clear_default_framebuffer(const glm::vec4& p_clear_colour);

		FBO(const glm::uvec2& p_resolution, bool p_colour_attachment = true, bool p_depth_attachment = true, bool p_stencil_attachment = true);
		~FBO();

		FBO(const FBO& p_other)            = delete;
		FBO& operator=(const FBO& p_other) = delete;
		FBO(FBO&& p_other);
		FBO& operator=(FBO&& p_other);

		const Texture& color_attachment()   const { return m_colour_attachment.value(); }
		const Texture& depth_attachment()   const { return m_depth_stencil_attachment ? m_depth_stencil_attachment.value() : m_depth_attachment.value(); }
		const Texture& stencil_attachment() const { return m_depth_stencil_attachment ? m_depth_stencil_attachment.value() : m_stencil_attachment.value(); }

		void clear() const;
		void resize(const glm::uvec2& p_resolution);
		void set_clear_colour(const glm::vec4& p_clear_colour) { m_clear_colour = p_clear_colour; }
		bool is_complete() const;
		const glm::uvec2& resolution() const { return m_resolution; }
	};
} // namespace OpenGL