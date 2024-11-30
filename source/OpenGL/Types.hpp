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
	// If the buffer data wont be changing, it is recommended construct the buffer with the data directly.
	// Otherwise allocate the buffer with the desired capacity and use set_data to update the data as you go.
	class Buffer
	{
		friend class VAO;
		friend class DrawCall;

		GLHandle m_handle;
		size_t m_capacity;      // Number of Bytes allocated for the buffer.
		size_t m_used_capacity; // Number of Bytes used by the buffer. Not the number of elements like in std::vector.
		BufferStorageBitfield m_flags;

	public:
		// Create a buffer object with the specified flags and capacity.
		//@param p_flags The flags to use when creating the buffer.
		//@param p_capacity The capacity of the buffer in bytes. If 0, the buffer is created with no storage.
		Buffer(BufferStorageBitfield p_flags, size_t p_capacity = 0);

		// Construct a buffer to match the vector p_data exactly.
		//@param p_flags The flags to use when creating the buffer.
		//@param p_data The data to copy into the buffer.
		//@param p_capacity The capacity of the buffer in bytes. Must be greater than or equal to the size of the vector p_data.
		template<typename T>
		Buffer(BufferStorageBitfield p_flags, const std::vector<T>& p_data, size_t p_capacity)
			: m_handle{State::Get().create_buffer()}
			, m_capacity{p_capacity}
			, m_used_capacity{p_data.size() * sizeof(T)}
			, m_flags{p_flags}
		{
			ASSERT(p_data.size() <= std::numeric_limits<GLsizei>::max(), "Too many elements in vector to fit in a GLsizei.");
			ASSERT(p_data.size() > 0, "Vector is empty.");
			ASSERT(p_capacity >= m_used_capacity, "Buffer capacity must be greater than or equal to the size of the data.");
			if constexpr (LogGLTypeEvents || LogGLBufferEvents) LOG("[OPENGL][BUFFER] Creating buffer {} with capacity {}B and copying {}B of data", m_handle, m_capacity, m_used_capacity);

			if (m_used_capacity == m_capacity) // If size is equal to capacity, we can directly allocate and copy the data.
				named_buffer_storage(m_handle, m_capacity, p_data.data(), m_flags);
			else
			{
				named_buffer_storage(m_handle, m_capacity, nullptr, m_flags);
				named_buffer_sub_data(m_handle, 0, m_used_capacity, p_data.data());
			}
		}
		template<typename T>
		Buffer(BufferStorageBitfield p_flags, const std::vector<T>& p_data)
			: Buffer(p_flags, p_data, p_data.size() * sizeof(T)) {}

		// Construct the buffer to match the array p_data exactly.
		template<typename T, size_t N>
		Buffer(BufferStorageBitfield p_flags, const std::array<T, N>& p_data, size_t p_capacity = N * sizeof(T))
			: m_handle{State::Get().create_buffer()}
			, m_capacity{p_capacity}
			, m_used_capacity{N * sizeof(T)}
			, m_flags{p_flags}
		{
			static_assert(N <= std::numeric_limits<GLsizei>::max(), "Too many elements in array to fit in a GLsizei.");
			static_assert(N > 0, "Array is empty.");
			ASSERT(p_capacity >= m_used_capacity, "Buffer capacity must be greater than or equal to the size of the data.");
			if constexpr (LogGLTypeEvents || LogGLBufferEvents) LOG("[OPENGL][BUFFER] Creating buffer {} with capacity {}B and copying {}B of data", m_handle, m_capacity, m_used_capacity);

			if (m_used_capacity == m_capacity) // If size is equal to capacity, we can directly allocate and copy the data.
				named_buffer_storage(m_handle, m_capacity, p_data.data(), m_flags);
			else
			{
				named_buffer_storage(m_handle, m_capacity, nullptr, m_flags);
				named_buffer_sub_data(m_handle, 0, m_used_capacity, p_data.data());
			}
		}

		Buffer(const Buffer& p_other);
		Buffer& operator=(const Buffer& p_other);
		Buffer(Buffer&& p_other);
		Buffer& operator=(Buffer&& p_other);
		~Buffer();

		template<typename T>
		std::vector<T> download_data(size_t p_count) const
		{
			std::vector<T> data(p_count);
			get_named_buffer_sub_data(m_handle, 0, m_used_capacity, data.data());
			return data;
		}
		template<typename T, size_t N>
		std::array<T, N> download_data_array(size_t offset = 0) const
		{
			ASSERT(N * sizeof(T) <= m_used_capacity - offset, "Downloading data out of bounds.");

			std::array<T, N> data;
			get_named_buffer_sub_data(m_handle, offset, m_used_capacity, data.data());
			return data;
		}

		// Set the section of the buffer starting at p_offset to p_data.
		template<typename T>
		void set_data(const T& p_data, GLintptr p_offset)
		{
			static_assert(!std::is_pointer_v<T>, "T must not be a pointer type.");
			static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
			static_assert(std::is_standard_layout_v<T>, "T must be a standard layout type.");
			ASSERT(m_capacity >= p_offset + sizeof(T), "Buffer sub data out of bounds. Use reserve to increase the capacity of the buffer before.");

			named_buffer_sub_data(m_handle, p_offset, sizeof(T), &p_data);
			m_used_capacity = std::max(m_used_capacity, p_offset + sizeof(T));
		}
		template<typename T>
		void set_data(const T& p_data) { set_data(p_data, m_used_capacity); }

		// Set the section of the buffer starting at p_offset to the vector p_data.
		template<typename T>
		void set_data(const std::vector<T>& p_data, GLintptr p_offset)
		{
			static_assert(!std::is_pointer_v<T>, "T must not be a pointer type.");
			static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
			static_assert(std::is_standard_layout_v<T>, "T must be a standard layout type.");
			ASSERT(m_capacity >= p_offset + (sizeof(T) * p_data.size()), "Buffer sub data out of bounds. Use reserve to increase the capacity of the buffer before.");

			named_buffer_sub_data(m_handle, p_offset, sizeof(T) * p_data.size(), p_data.data());
			m_used_capacity = std::max(m_used_capacity, p_offset + (sizeof(T) * p_data.size()));
		}
		template<typename T>
		void set_data(const std::vector<T>& p_data) { set_data(p_data, m_used_capacity); }

		// Set a section of the buffer to 4x4 matrix p_data.
		//@param p_data The data to set in the buffer.
		//@param p_offset The offset in bytes from the start of the buffer to the start of the data. By default, the data is appended to the end of the buffer.
		void set_data(const glm::mat4& p_data, GLintptr p_offset)
		{
			ASSERT(m_capacity >= p_offset + sizeof(glm::mat4), "Buffer sub data out of bounds. Use reserve to increase the capacity of the buffer before.");

			named_buffer_sub_data(m_handle, p_offset, sizeof(glm::mat4), &p_data[0][0]);
			m_used_capacity = std::max(m_used_capacity, p_offset + sizeof(glm::mat4));
		}
		void set_data(const glm::mat4& p_data) { set_data(p_data, m_used_capacity); }

		// Set a section of the buffer using p_data.
		//@param p_data The data to set in the buffer.
		//@param p_offset The offset in bytes from the start of the buffer to the start of the data. By default, the data is appended to the end of the buffer.
		void set_data(bool p_data, GLintptr p_offset)
		{
			// GLSL bools are 4 bytes in size.
			GLint gl_bool = p_data;
			set_data(p_offset, gl_bool);
		}
		void set_data(bool p_data) { set_data(p_data, m_used_capacity); }


		// Copy a p_size portion of p_source_buffer from p_source_offset into this buffer at p_destination_offset.
		void copy_from_buffer(const Buffer& p_source_buffer, size_t p_source_offset, size_t p_destination_offset, size_t p_size)
		{
			ASSERT(p_source_buffer.m_handle != m_handle, "Source and destination buffers must be different.");
			ASSERT(m_capacity >= p_destination_offset + p_size, "Buffer sub data out of bounds. Use reserve to increase the capacity of the buffer before.");
			ASSERT(p_source_buffer.m_capacity >= p_source_offset + p_size, "Source buffer sub data out of bounds. Use reserve to increase the capacity of the buffer before.");
			ASSERT(p_size > 0, "Size must be greater than 0.");
			if constexpr (LogGLBufferEvents) LOG("[OPENGL][BUFFER] Copying {}B of data from buffer {} to buffer {}", p_size, p_source_buffer.m_handle, m_handle);

			copy_named_buffer_sub_data(p_source_buffer.m_handle, m_handle, p_source_offset, p_destination_offset, p_size);
			m_used_capacity = std::max(m_used_capacity, p_destination_offset + p_size);
		}

		// Increase the capacity of the Buffer to a number of bytes that's greater or equal to p_capacity.
		//@param p_capacity The new capacity of the buffer in bytes.
		void reserve(size_t p_capacity);
		// Truncate the buffer to the specified size. Any data beyond p_size is lost.
		//@param p_size The new size of the buffer in bytes.
		void shrink_to_size(size_t p_size);
		// Shrinks the buffer to fit the data. Does nothing if size() == capacity().
		void shrink_to_fit();
		// Clears the buffer object's data store. All existing data is lost.
		void clear();
		// Clears a section of the buffer starting at p_start_offset and p_size bytes long.
		//@param p_start_offset The offset in bytes from the start of the buffer to the start of the data to clear.
		//@param p_size The number of bytes to clear.
		void clear(size_t p_start_offset, size_t p_size);

		size_t capacity()           const { return m_capacity; }
		size_t used_capacity()      const { return m_used_capacity; }
		float used_capacity_ratio() const { return (float)m_used_capacity / (float)m_capacity; }
		bool empty()                const { return m_used_capacity == 0; }
		bool is_immutable()         const;
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
		void attach_buffer(Buffer& p_vertex_buffer, GLintptr p_vertex_buffer_offset, GLuint p_vertex_buffer_binding_point, GLsizei p_stride, GLsizei p_vertex_count);
		// Binds p_element_buffer to the VAO. Does not modify the global GL state.
		//@param p_element_buffer The element buffer object (EBO) to attach to the VAO for reading index data of the attached vertex_buffer.
		//@param p_element_count The number of indices in p_element_buffer. Used by VAO to determine the number of vertices to draw.
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