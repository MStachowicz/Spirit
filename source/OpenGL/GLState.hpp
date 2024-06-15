#pragma once

#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

//using GLboolean  = unsigned char; // Our wrapper replaces GLboolean with bool.
using GLbyte     = signed char;
using GLubyte    = unsigned char;
using GLshort    = signed short;
using GLushort   = unsigned short;
using GLint      = int;
using GLuint     = unsigned int;
using GLHandle   = GLuint;       // A GLHandle is an ID used by OpenGL to point to memory owned by this OpenGL context on the GPU.
using GLenum     = unsigned int; // Our wrapper replaces GLenum with strongly typed enums. (except for texture setup)
using GLfixed    = int32_t;
using GLfloat    = float;
using GLhalf     = unsigned short;
using GLdouble   = double;
using GLsizei    = int;
using GLsizeiptr = std::ptrdiff_t;
using GLintptr   = std::ptrdiff_t;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// STRONGLY TYPED ENUM WRAPPERS
namespace OpenGL
{
	constexpr inline static bool LogGLTypeEvents = false;

	enum class BufferType : uint8_t
	{
		ArrayBuffer,             // Vertex attributes
		AtomicCounterBuffer,     // Atomic counter storage
		CopyReadBuffer,          // Buffer copy source
		CopyWriteBuffer,         // Buffer copy destination
		DispatchIndirectBuffer,  // Indirect compute dispatch commands
		DrawIndirectBuffer,      // Indirect command arguments
		ElementArrayBuffer,      // Vertex array indices
		PixelPackBuffer,         // Pixel read target
		PixelUnpackBuffer,       // Texture data source
		QueryBuffer,             // Query result buffer
		ShaderStorageBuffer,     // Read-write storage for shaders
		TextureBuffer,           // Texture data buffer
		TransformFeedbackBuffer, // Transform feedback buffer
		UniformBuffer            // Uniform block storage
	};
	enum class BufferDataType : uint8_t
	{
		Byte,          // GL_Byte aka GLbyte,
		UnsignedByte,  // GL_UNSIGNED_BYTE aka GLubyte,
		Short,         // GL_SHORT aka GLshort,
		UnsignedShort, // GL_UNSIGNED_SHORT aka GLushort,
		Int,           // GL_INT aka GLint,
		UnsignedInt,   // GL_UNSIGNED_INT aka GLuint,
		Fixed,         // GL_FIXED aka GLfixed,
		Float,         // GL_FLOAT aka GLfloat,
		HalfFloat,     // GL_HALF_FLOAT aka GLhalf,
		Double         // GL_DOUBLE aka GLdouble
	};
	// Specifies the intended usage of the buffer's data store.
	enum class BufferStorageFlag : uint8_t
	{
		// The contents of the data store may be updated after creation through calls to glBufferSubData.
		// If this bit is not set, the buffer content may not be directly updated by the client.
		// The p_data argument may be used to specify the initial content of the buffer's data store regardless of the presence of the DynamicStorageBit.
		// Regardless of the presence of this bit, buffers may always be updated with server-side calls such as glCopyBufferSubData and glClearBufferSubData.
		DynamicStorageBit,
		// The data store may be mapped by the client for read access and a pointer in the client's address space obtained that may be read from.
		MapReadBit,
		// The data store may be mapped by the client for write access and a pointer in the client's address space obtained that may be written through.
		MapWriteBit,
		// The client may request that the server read from or write to the buffer while it is mapped.
		// The client's pointer to the data store remains valid so long as the data store is mapped, even during execution of drawing or dispatch commands.
		MapPersistentBit,
		// Shared access to buffers that are simultaneously mapped for client access and are used by the server will be coherent, so long as that mapping is performed using glMapBufferRange.
		// That is, data written to the store by either the client or server will be immediately visible to the other with no further action taken by the application.
		// In particular,
		// If MapCoherentBit is not set and the client performs a write followed by a call to the glMemoryBarrier command with the GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT set, then in subsequent commands the server will see the writes.
		// If MapCoherentBit is set and the client performs a write, then in subsequent commands the server will see the writes.
		// If MapCoherentBit is not set and the server performs a write, the application must call glMemoryBarrier with the GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT set and then call glFenceSync with GL_SYNC_GPU_COMMANDS_COMPLETE (or glFinish). Then the CPU will see the writes after the sync is complete.
		// If MapCoherentBit is set and the server does a write, the app must call glFenceSync with GL_SYNC_GPU_COMMANDS_COMPLETE (or glFinish). Then the CPU will see the writes after the sync is complete.
		MapCoherentBit,
		// When all other criteria for the buffer storage allocation are met, this bit may be used by an implementation to determine whether to use storage that is local to the server or to the client to serve as the backing store for the buffer.
		ClientStorageBit
	};
	struct BufferStorageBitfield
	{
		BufferStorageBitfield(std::initializer_list<BufferStorageFlag> flags);
		GLuint bitfield = 0;
	};


	enum class TextureMagFunc : uint8_t
	{
		Nearest, // Nearest neighbour interpolation.
		Linear   // Linear interpolation.
	};
	enum class WrappingMode : uint8_t
	{
		Repeat,           // Default mode, coordinates wrap around the texture. For example, the texture coordinate (1.1, 1.2) is same as (0.1, 0.2).
		MirroredRepeat,   // Coordinates wrap around the texture, but the texture is flipped at every integer junction.
		ClampToEdge,      // Coordinates are clamped to the edge of the texture. Any texture coordinates outside the range [0, 1] will be clamped to the nearest value.
		ClampToBorder,    // Coordinates are clamped to the border colour.
		MirrorClampToEdge // Coordinates are mirrored and then clamped to the edge of the texture.
	};
	// Specifies the format of the pixel data.
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexSubImage2D.xhtml
	enum class TextureFormat : uint8_t
	{
		R,
		RG,
		RGB,
		BGR,
		RGBA,
		BGRA,
		DepthComponent,
		StencilIndex
	};
	// Specifies the sized internal format to be used to store texture image data on the GPU.
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
	enum class TextureInternalFormat : uint8_t
	{
		// TYPE             BASE-INTERNAL-FORMAT    RED,GREEN,BLUE           SHARED BITS
		R8,                 // GL_RED               8
		R8_SNORM,           // GL_RED               s8
		R16,                // GL_RED               16
		R16_SNORM,          // GL_RED               s16
		RG8,                // GL_RG                8, 8
		RG8_SNORM,          // GL_RG                s8, s8
		RG16,               // GL_RG                16, 16
		RG16_SNORM,         // GL_RG                s16, s16
		R3_G3_B2,           // GL_RGB               3, 3, 2
		RGB4,               // GL_RGB               4, 4, 4
		RGB5,               // GL_RGB               5, 5, 5
		RGB8,               // GL_RGB               8, 8, 8
		RGB8_SNORM,         // GL_RGB               s8, s8, s8
		RGB10,              // GL_RGB               10, 10, 10
		RGB12,              // GL_RGB               12, 12, 12
		RGB16_SNORM,        // GL_RGB               16, 16, 16
		RGBA2,              // GL_RGB               2, 2, 2, 2
		RGBA4,              // GL_RGB               4, 4, 4, 4
		RGB5_A1,            // GL_RGBA              5, 5, 5, 1
		RGBA8,              // GL_RGBA              8, 8, 8, 8
		RGBA8_SNORM,        // GL_RGBA              s8, s8, s8, s8
		RGB10_A2,           // GL_RGBA              10, 10, 10, 2
		RGB10_A2UI,         // GL_RGBA              ui10, ui10, ui10 ,ui2
		RGBA12,             // GL_RGBA              12, 12, 12, 12
		RGBA16,             // GL_RGBA              16, 16, 16, 16
		SRGB8,              // GL_RGB               8, 8, 8
		SRGB8_ALPHA8,       // GL_RGBA              8, 8, 8, 8
		R16F,               // GL_RED               f16
		RG16F,              // GL_RG                f16, f16
		RGB16F,             // GL_RGB               f16, f16, f16
		RGBA16F,            // GL_RGBA              f16, f16, f16, f16
		R32F,               // GL_RED               f32
		RG32F,              // GL_RG                f32, f32
		RGB32F,             // GL_RGB               f32, f32, f32
		RGBA32F,            // GL_RGBA              f32, f32, f32, f32
		R11F_G11F_B10F,     // GL_RGB               f11, f11, f10
		RGB9_E5,            // GL_RGB               9, 9, 9,                 5
		R8I,                // GL_RED               i8
		R8UI,               // GL_RED               ui8
		R16I,               // GL_RED               i16
		R16UI,              // GL_RED               ui16
		R32I,               // GL_RED               i32
		R32UI,              // GL_RED               ui32
		RG8I,               // GL_RG                i8, i8
		RG8UI,              // GL_RG                ui8, ui8
		RG16I,              // GL_RG                i16, i16
		RG16UI,             // GL_RG                ui16, ui16
		RG32I,              // GL_RG                i32, i32
		RG32UI,             // GL_RG                ui32, ui32
		RGB8I,              // GL_RGB               i8, i8, i8
		RGB8UI,             // GL_RGB               ui8, ui8, ui8
		RGB16I,             // GL_RGB               i16, i16, i16
		RGB16UI,            // GL_RGB               ui16, ui16, ui16
		RGB32I,             // GL_RGB               i32, i32, i32
		RGB32UI,            // GL_RGB               ui32, ui32, ui32
		RGBA8I,             // GL_RGBA              i8, i8, i8, i8
		RGBA8UI,            // GL_RGBA              ui8, ui8, ui8, ui8
		RGBA16I,            // GL_RGBA              i16, i16, i16, i16
		RGBA16UI,           // GL_RGBA              ui16, ui16, ui16, ui16
		RGBA32I,            // GL_RGBA              i32, i32, i32, i32
		RGBA32UI,           // GL_RGBA              ui32, ui32, ui32, ui32
		DEPTH_COMPONENT32F, // GL_DEPTH_COMPONENT32F
		DEPTH_COMPONENT24,  // GL_DEPTH_COMPONENT24
		DEPTH_COMPONENT16,  // GL_DEPTH_COMPONENT16
		DEPTH32F_STENCIL8,  // GL_DEPTH32F_STENCIL8
		DEPTH24_STENCIL8,   // GL_DEPTH24_STENCIL8
		STENCIL_INDEX8,     // GL_STENCIL_INDEX8
	};
	// The data type of the pixel data.
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexSubImage2D.xhtml
	enum class TextureDataType : uint8_t
	{
		UNSIGNED_BYTE,              // GL_UNSIGNED_BYTE
		BYTE,                       // GL_BYTE
		UNSIGNED_SHORT,             // GL_UNSIGNED_SHORT
		SHORT,                      // GL_SHORT
		UNSIGNED_INT,               // GL_UNSIGNED_INT
		INT,                        // GL_INT
		FLOAT,                      // GL_FLOAT
		UNSIGNED_BYTE_3_3_2,        // GL_UNSIGNED_BYTE_3_3_2
		UNSIGNED_BYTE_2_3_3_REV,    // GL_UNSIGNED_BYTE_2_3_3_REV
		UNSIGNED_SHORT_5_6_5,       // GL_UNSIGNED_SHORT_5_6_5
		UNSIGNED_SHORT_5_6_5_REV,   // GL_UNSIGNED_SHORT_5_6_5_REV
		UNSIGNED_SHORT_4_4_4_4,     // GL_UNSIGNED_SHORT_4_4_4_4
		UNSIGNED_SHORT_4_4_4_4_REV, // GL_UNSIGNED_SHORT_4_4_4_4_REV
		UNSIGNED_SHORT_5_5_5_1,     // GL_UNSIGNED_SHORT_5_5_5_1
		UNSIGNED_SHORT_1_5_5_5_REV, // GL_UNSIGNED_SHORT_1_5_5_5_REV
		UNSIGNED_INT_8_8_8_8,       // GL_UNSIGNED_INT_8_8_8_8
		UNSIGNED_INT_8_8_8_8_REV,   // GL_UNSIGNED_INT_8_8_8_8_REV
		UNSIGNED_INT_10_10_10_2,    // GL_UNSIGNED_INT_10_10_10_2
		UNSIGNED_INT_2_10_10_10_REV // GL_UNSIGNED_INT_2_10_10_10_REV
	};


	enum class ShaderProgramType : uint8_t
	{
		Vertex,
		Geometry,
		Fragment,
		Compute
	};
	enum class ShaderDataType : uint8_t
	{
		Float, Vec2, Vec3, Vec4,
		Double, DVec2, DVec3, DVec4,
		Int, IVec2, IVec3, IVec4,
		UnsignedInt, UVec2, UVec3, UVec4,
		Bool, BVec2, BVec3, BVec4,
		Mat2, Mat3, Mat4,
		Mat2x3, Mat2x4, Mat3x2, Mat3x4, Mat4x2, Mat4x3,
		Dmat2, Dmat3, Dmat4,
		Dmat2x3, Dmat2x4, Dmat3x2, Dmat3x4, Dmat4x2, Dmat4x3,
		Sampler1D, Sampler2D, Sampler3D,
		SamplerCube,
		Sampler1DShadow, Sampler2DShadow,
		Sampler1DArray, Sampler2DArray,
		Sampler1DArrayShadow, Sampler2DArrayShadow,
		Sampler2DMS, Sampler2DMSArray,
		SamplerCubeShadow,
		SamplerBuffer,
		Sampler2DRect,
		Sampler2DRectShadow,
		Isampler1D, Isampler2D, Isampler3D,
		IsamplerCube,
		Isampler1DArray, Isampler2DArray,
		Isampler2DMS,
		Isampler2DMSArray,
		IsamplerBuffer,
		Isampler2DRect,
		Usampler1D, Usampler2D, Usampler3D,
		UsamplerCube,
		Usampler2DArray,
		Usampler2DMS,
		Usampler2DMSArray,
		UsamplerBuffer,
		Usampler2DRect,
		Unknown
	};


	enum class DepthTestType : uint8_t
	{
		Always,
		Never,
		Less,
		Equal,
		NotEqual,
		Greater,
		LessEqual,
		GreaterEqual
	};
	enum class BlendFactorType : uint8_t
	{
		Zero,                      // Factor is equal to 0.
		One,                       // Factor is equal to 1.
		SourceColour,              // Factor is equal to the source colour vector.
		OneMinusSourceColour,      // Factor is equal to 1 minus the source colour vector.
		DestinationColour,         // Factor is equal to the destination colour vector
		OneMinusDestinationColour, // Factor is equal to 1 minus the destination colour vector
		SourceAlpha,               // Factor is equal to the alpha component of the source colour vector.
		OneMinusSourceAlpha,       // Factor is equal to 1 minus alpha of the source colour vector.
		DestinationAlpha,          // Factor is equal to the alpha component of the destination colour vector.
		OneMinusDestinationAlpha,  // Factor is equal to 1 minus alpha of the destination colour vector.
		ConstantColour,            // Factor is equal to the constant colour vector.
		OneMinusConstantColour,    // Factor is equal to 1 minus the constant colour vector.
		ConstantAlpha,             // Factor is equal to the alpha component of the constant colour vector.
		OneMinusConstantAlpha,     // Factor is equal to 1 minus alpha of the constant colour vector.
	};
	enum class CullFaceType : uint8_t
	{
		Back,          // Culls only the back faces (Default OpenGL setting).
		Front,         // Culls only the front faces.
		FrontAndBack   // Culls both the front and back faces.
	};
	enum class FrontFaceOrientation : uint8_t
	{
		Clockwise,          // Clockwise polygons are identified as front-facing.
		CounterClockwise,   // Counter-clockwise polygons are identified as front-facing (Default OpenGL setting).
	};
	// Polygon rasterization mode
	// Vertices are marked as boundary/non-boundary with an edge flag generated internally by OpenGL when it decomposes triangle stips and fans.
	enum class PolygonMode : uint8_t
	{
		Point, // Polygon vertices that are marked as the start of a boundary edge are drawn as points. Point attributes such as GL_POINT_SIZE and GL_POINT_SMOOTH control the rasterization of the points.
		Line,  // Boundary edges of the polygon are drawn as line segments. Line attributes such as GL_LINE_WIDTH and GL_LINE_SMOOTH control the rasterization of the lines.
		Fill   // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon. (Default OpenGL setting).
	};
	// Interpretation scheme used to determine what a stream of vertices represents when being rendered.
	enum class PrimitiveMode : uint8_t
	{
		Points,
		LineStrip,
		LineLoop,
		Lines,
		LineStripAdjacency,
		LinesAdjacency,
		TriangleStrip,
		TriangleFan,
		Triangles,
		TriangleStripAdjacency,
		TrianglesAdjacency,
		Patches,
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// STATE FUNCTIONS
namespace OpenGL
{
	// OpenGL::State encapsulates the state of the OpenGL context.
	// State prevents excess gl function calls by only allowing them to be called if there is a required state change.
	class State
	{
		bool write_to_depth_buffer;
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

		//glm::vec4 clear_colour;
		glm::ivec2 viewport_position;
		glm::ivec2 viewport_size;

		GLHandle current_bound_shader_program;
		GLHandle current_bound_VAO;
		GLHandle current_bound_FBO;

		// Buffers are bound to specific binding points per target.
		// We dont know the number of binding points at compile time hence these are vectors.
		// State will resize these on construction and they are never resized after that point.

		std::vector<std::optional<GLHandle>> current_bound_SSBO;    // Per binding point the current bound SSBO.
		std::vector<std::optional<GLHandle>> current_bound_UBO;     // Per binding point the current bound UBO.
		std::vector<std::optional<GLHandle>> current_bound_texture; // Per binding point the current bound texture unit.

	public:
		void bind_VAO(GLHandle p_VAO);
		void unbind_VAO();

		void bind_FBO(GLHandle p_FBO);
		void unbind_FBO();

		void bind_shader_storage_buffer(GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size);
		void bind_uniform_buffer(GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size);
		void unbind_buffer(GLHandle p_buffer);

		void bind_texture_unit(GLuint p_texture_unit, GLHandle p_texture);
		void unbind_texture_unit(GLHandle p_texture);


		// Installs a program object as part of current rendering state
		// While a program object is in use, applications are free to modify attached shader objects, compile attached shader objects, attach additional shader objects, and detach or delete shader objects.
		// None of these operations will affect the executables that are part of the current state.
		// However, relinking the program object that is currently in use will install the program object as part of the current rendering state if the link operation was successful (glLinkProgram).
		// If the program object currently in use is relinked unsuccessfully, its link status will be set to GL_FALSE, but the executables and associated state will remain part of the current state until a subsequent call to glUseProgram removes it from use.
		// After it is removed from use, it cannot be made part of current state until it has been successfully relinked.
		void use_program(GLHandle p_shader_program);
		// Deletes a program object and frees the memory.
		// Invalidates the p_shader_program associated with the program object.
		// This command effectively undoes the effects of a call to create_program.
		// If a program object is in use as part of current rendering state, it will be flagged for deletion, but it will not be deleted until it is no longer part of current state for any rendering context.
		// If a program object to be deleted has shader objects attached to it, those shader objects will be automatically detached but not deleted unless they have already been flagged for deletion by a previous call to delete_shader.
		// To determine whether a program object has been flagged for deletion, call glGetProgram with arguments program and GL_DELETE_STATUS.
		//@param p_shader_program Shader program object to be deleted. A value of 0 will be silently ignored.
		void delete_program(GLHandle p_shader_program);



		void set_depth_write(bool p_write_to_depth_buffer);
		void set_depth_test(bool p_depth_test);
		void set_depth_test_type(DepthTestType p_type);
		void set_polygon_offset(bool p_polygon_offset);
		// Set the scale and units used to calculate depth values.
		// polygon_offset is useful for rendering hidden-line images, for applying decals to surfaces, and for rendering solids with highlighted edges.
		//@param p_polygon_offset_factor Specifies a scale factor that is used to create a variable depth offset for each polygon.
		//@param p_polygon_offset_units Is multiplied by an implementation-specific value to create a constant depth offset.
		void set_polygon_offset_factor(GLfloat p_polygon_offset_factor, GLfloat p_polygon_offset_units);
		// Specifies if objects with alpha values <1 should be blended using function set with set_blend_func().
		void set_blending(bool p_blend);
		// Specifies how the RGBA factors of source and destination are blended to give the final pixel colour when encountering transparent objects.
		void set_blend_func(BlendFactorType p_source_factor, BlendFactorType p_destination_factor);
		// Specifies if facets specified by set_front_face_orientation are candidates for culling.
		void set_cull_face(bool p_cull);
		// Specifies which facets are candidates for culling.
		void set_cull_face_type(CullFaceType p_cull_face_type);
		// The orientation of front-facing polygons. Used to mark facets for culling.
		void set_front_face_orientation(FrontFaceOrientation p_front_face_orientation);
		// Set the viewport.
		// When a GL context is first attached to a window, width and height are set to the dimensions of that window.
		// The affine transformation of x and y from normalized device coordinates to window coordinates.
		//@param p_x The lower left x corner of the viewport rectangle, in pixels. The initial value is 0.
		//@param p_y The lower left y corner of the viewport rectangle, in pixels. The initial value is 0.
		//@param p_width The width of the viewport.
		//@param p_height The height of the viewport.
		void set_viewport(GLint p_x, GLint p_y, GLsizei p_width, GLsizei p_height);
		// Controls the interpretation of polygons for rasterization.
		// p_polygon_mode: Specifies how polygons will be rasterized.
		// Affects only the final rasterization of polygons - a polygon's vertices are lit and the polygon is clipped/culled before these modes are applied.
		void set_polygon_mode(PolygonMode p_polygon_mode);


		State(State const&)          = delete;
		void operator=(State const&) = delete;
		static State& Get()
		{
			// State accessor allows us to delay instantiation of the state until the
			// OpenGL context is initialised in Core::initialise_OpenGL.
			static State instance;
			return instance;
		}

	private:
		State(); // Private constructor to prevent instantiation outside Get().
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TEXTURE FUNCTIONS
namespace OpenGL
{
	// The max width and height of a 1D or 2D texture the GPU supports
	//@returns The single max for the width, height (1:1).
	GLint max_texture_size();
	// The max width, height, depth of a 3D texture that your GPU supports.
	//@returns The single max for the width, height and depth (1:1:1).
	GLint max_3D_texture_size();
	// The max width and height of a a cubemap texture that your GPU supports
	//@returns The single max for the width, height (1:1).
	GLint max_cube_map_texture_size();
	// The number of image-units/samplers the GPU supports in the fragment shader.
	GLint max_texture_image_units();
	// Max number of image-units/samplers the GPU supports in the vertex shader. This might return 0 for certain GPUs.
	GLint max_vertex_texture_image_units();
	// Max number of image-units/samplers the GPU supports in the geometry shader.
	GLint max_geometry_texture_image_units();
	// Max number of image-units/samplers the GPU supports in the vertex + geometry + fragment shaders combined.
	GLint max_combined_texture_image_units();
	// Max number of array levels for ArrayTexture objects.
	GLint max_array_texture_layers();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DRAW FUNCTIONS
namespace OpenGL
{
	// Render primitives from array data.
	// draw_arrays specifies multiple geometric primitives with very few subroutine calls. Instead of calling a GL procedure to pass each individual vertex, normal, texture coordinate, edge flag, or color, you can prespecify separate arrays of vertices, normals, and colors and use them to construct a sequence of primitives with a single call to glDrawArrays.
	// When draw_arrays is called, it uses p_array_size sequential elements from each enabled array to construct a sequence of geometric primitives, beginning with element first. p_primitive_mode specifies what kind of primitives are constructed and how the array elements construct those primitives.
	// Vertex attributes that are modified by draw_arrays have an unspecified value after draw_arrays returns. Attributes that aren't modified remain well defined.
	//@param p_primitive_mode What kind of primitives to render
	//@param p_first The starting index in the enabled arrays.
	//@param p_count The number of indices to be rendered.
	void draw_arrays(PrimitiveMode p_primitive_mode, GLint p_first, GLsizei p_count);
	// Draw multiple instances of a range of elements
	// draw_arrays_instanced behaves identically to draw_arrays except that p_instance_count instances of the range of elements are executed and the value of the internal counter instanceID advances for each iteration.
	// instanceID is an internal 32-bit integer counter that may be read by a vertex shader as gl_InstanceID.
	void draw_arrays_instanced(PrimitiveMode p_primitive_mode, GLint p_first, GLsizei p_array_size, GLsizei p_instance_count);
	// Render primitives from array data.
	// draw_elements specifies multiple geometric primitives with very few subroutine calls. Instead of calling a GL function to pass each individual vertex, normal, texture coordinate, edge flag, or color, you can prespecify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
	// When draw_elements is called, it uses p_elements_size sequential elements from an enabled array, starting at indices to construct a sequence of geometric primitives. p_primitive_mode specifies what kind of primitives are constructed and how the array elements construct these primitives. If more than one array is enabled, each is used.
	// Vertex attributes that are modified by draw_elements have an unspecified value after draw_elements returns. Attributes that aren't modified maintain their previous values.
	void draw_elements(PrimitiveMode p_primitive_mode, GLsizei p_elements_size);
	// Draw multiple instances of a set of elements.
	// draw_elements_instanced behaves identically to draw_elements except that p_instance_count of the set of elements are executed and the value of the internal counter instanceID advances for each iteration.
	// instanceID is an internal 32-bit integer counter that may be read by a vertex shader as gl_InstanceID.
	void draw_elements_instanced(PrimitiveMode p_primitive_mode, GLsizei p_elements_size, GLsizei p_instance_count);
	// Launch one or more compute work groups.
	// Each work group is processed by the active program object for the compute shader stage.
	// The individual shader invocations within a work group are executed as a unit, work groups are executed completely independently and in unspecified order.
	//@param p_num_groups_x The number of work groups to be launched in the X dimension.
	//@param p_num_groups_y The number of work groups to be launched in the Y dimension.
	//@param p_num_groups_z The number of work groups to be launched in the Z dimension.
	void dispatch_compute(GLuint p_num_groups_x, GLuint p_num_groups_y, GLuint p_num_groups_z);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHADER FUNCTIONS
namespace OpenGL
{
	// Creates an empty program object and returns a non-zero value by which it can be referenced.
	// A program object is an object to which shader objects can be attached. This provides a mechanism to specify the shader objects that will be linked to create a program.
	// It also provides a means for checking the compatibility of the shaders that will be used to create a program (for instance, checking the compatibility between a vertex shader and a fragment shader).
	// When no longer needed as part of a program object, shader objects can be detached.
	GLHandle create_program();
	// Attaches a shader object to a program object
	// Shaders that are to be linked together in a program object must first be attached to that program object.
	// All operations that can be performed on a shader object are valid whether or not the shader object is attached to a program object.
	// It is permissible to attach a shader object to a program object before source code has been loaded into the shader object or before the shader object has been compiled.
	// It is permissible to attach multiple shader objects of the same type because each may contain a portion of the complete shader.
	// It is also permissible to attach a shader object to more than one program object.
	// If a shader object is deleted while it is attached to a program object, it will be flagged for deletion, and deletion will not occur until DetachShader is called to detach it from all program objects to which it is attached.
	void attach_shader(GLHandle p_shader_program, GLHandle p_shader);
	// Links a program object
	// As a result of a successful link operation, all active user-defined uniform variables belonging to program will be initialized to 0
	// The active uniform variables will be assigned a location that can be queried using GL program introspection.
	// Any active user-defined attribute variables that have not been bound to a generic vertex attribute index will be bound to one at this time.
	// When a program object has been successfully linked, the program object can be made part of current state by calling glUseProgram
	// If the program object currently in use is relinked unsuccessfully the executables and associated state will remain part of the current state until a
	// subsequent call to glUseProgram removes it from use. After it is removed from use, it cannot be made part of current state until it has been successfully relinked.
	void link_program(GLHandle p_shader_program);
	// Creates an empty shader object and returns a non-zero value by which it can be referenced. A shader object is used to maintain the source code strings that define a shader.
	// Like buffer and texture objects, the name space for shader objects may be shared across a set of contexts, as long as the server sides of the contexts share the same address space.
	// If the name space is shared across contexts, any attached objects and the data associated with those attached objects are shared as well.
	GLHandle create_shader(ShaderProgramType p_program_type);
	// Deletes a shader object and frees the memory and invalidates the name associated with the shader object.
	// This command effectively undoes the effects of a call to create_shader.
	// If a shader object to be deleted is attached to a program object, it will be flagged for deletion, but it will not be deleted until it is no longer
	// attached to any program object, for any rendering context (i.e. it must be detached from wherever it was attached before it will be deleted).
	// To determine whether an object has been flagged for deletion, call glGetShader with arguments shader and GL_DELETE_STATUS.
	//@param p_shader Shader object to be deleted. A value of 0 for shader will be silently ignored.
	void delete_shader(GLHandle p_shader);
	// Replaces the source code in a shader object
	// Any source code previously stored in the shader object is completely replaced.
	// The source code strings are not scanned or parsed at this time; they are simply copied into the specified shader object.
	//@param p_shader Shader object to be compiled.
	//@param p_shader_source Souce code for the shader program.
	void shader_source(GLHandle p_shader, const std::string& p_shader_source);
	// Compiles the source code strings that have been stored in the shader object specified by p_shader.
	// The compilation status will be stored as part of the shader object's state.
	//@param p_shader Shader object to be compiled.
	void compile_shader(GLHandle p_shader);
	// Returns the location of a uniform variable.
	// p_name must be:
	//     - a null terminated string that contains no white space.
	//     - an active uniform variable in p_shader_program that is not a structure, an array of structures, or a subcomponent of a vector or a matrix.
	// This function returns -1 if p_name does not correspond to an active uniform variable in program, if p_name starts with the reserved prefix "gl_", or if p_name is associated with an atomic counter or a named uniform block.
	// The actual locations assigned to uniform variables are not known until the program object is linked successfully (link_program).
	// Uniform variables that are structures or arrays of structures may be queried by calling get_uniform_location for each field within the structure.
	// The array element operator "[]" and the structure field operator "." may be used in p_name in order to select elements within an array or fields within a structure.
	// The result of using these operators is not allowed to be another structure, an array of structures, or a subcomponent of a vector or a matrix.
	// Except if the last part of name indicates a uniform variable array, the location of the first element of an array can be retrieved by using the name of the array, or by using the name appended by "[0]".
	//@param p_shader_program Shader program object to be queried.
	//@param p_name Null terminated string containing the name of the uniform variable whose location is to be queried.
	//@return Integer that represents the location of a specific uniform variable within a program object or -1 if p_name is not an active uniform variable in p_shader_program.
	GLint get_uniform_location(GLHandle p_shader_program, const char* p_name);
	// Get the number of active uniforms variables within p_shader_program.
	// Any uniform variables optimised away will not be counted. p_shader_program must be linked before calling get_uniform_count.
	//@param p_shader_program Shader program to query.
	GLint get_uniform_count(GLHandle p_shader_program);
	// Get the number of active uniform buffer block objects (UBO) within p_shader_program.
	// Any uniform blocks optimised away will not be counted. p_shader_program must be linked before calling get_uniform_block_count.
	//@param p_shader_program Shader program to query.
	GLint get_uniform_block_count(GLHandle p_shader_program);
	// Get the number of active shader storage block objects (SSBO) within p_shader_program.
	// Any uniform blocks optimised away will not be counted. p_shader_program must be linked before calling get_shader_storage_block_count.
	//@param p_shader_program Shader program to query.
	GLint get_shader_storage_block_count(GLHandle p_shader_program);

	// Get the number of UniformBlock/UBO binding points available.
	GLint get_max_uniform_buffer_bindings();
	// Get the number of ShaderStorageBlock/SSBO binding points available.
	GLint get_max_shader_storage_buffer_bindings();
	// Get the max size in bytes a UniformBlock can have.
	GLint get_max_uniform_block_size();
	// Get the max size in bytes a ShaderStorageBlock can have (only pertains to the fixed-size portion of the block, size can exceed this ignoring variable sized arrays).
	GLint get_max_shader_storage_block_size();
	// Get the max number of texture units available for binding.
	//GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
	GLint get_max_combined_texture_image_units();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA/BUFFER FUNCTIONS
namespace OpenGL
{
	// Assign a binding point to an active uniform block.
	// Each of a program's active uniform blocks has a corresponding uniform buffer binding point.
	// If successful, specifies that p_shader_program will use the data store of the buffer object bound to p_uniform_block_binding to extract the values of the uniforms in the uniform block identified by p_uniform_block_index.
	//@param p_shader_program Shader program object containing the active UniformBlock whose binding to assign.
	//@param p_uniform_block_index Index of the active UniformBlock within program whose binding to assign.
	//@param p_uniform_block_binding Binding point to which to bind the UniformBlock with index p_uniform_block_index within program.
	void uniform_block_binding(GLHandle p_shader_program, GLuint p_uniform_block_index, GLuint p_uniform_block_binding);
	// Change an active shader storage block binding.
	// Changes the active shader storage block with an assigned index of p_storage_block_index in p_shader_program.
	// If successful, specifies that p_shader_program will use the data store of the buffer object bound to the binding point p_storage_block_binding to read and write the values of the buffer variables in the shader storage block identified by p_storage_block_index.
	//@param p_shader_program Shader program containing the block whose binding to change.
	//@param p_storage_block_index Index storage block within the program. Must be an active shader storage block index in program
	//@param p_storage_block_binding Index storage block binding to associate with the specified storage block. Must be less than the value of GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS.
	void shader_storage_block_binding(GLHandle p_shader_program, GLuint p_storage_block_index, GLuint p_storage_block_binding);
	// Bind the buffer named p_buffer to the vertex buffer binding point p_binding_index of the vertex array object p_VAO.
	// Does not modify the currently bound vertex array object, but does modify the state of the VAO by binding the buffer to the specified binding point.
	// If p_buffer is zero, then any buffer currently bound to the specified binding point is unbound.
	//@param p_VAO Specifies the name of the vertex array object to bind the buffer to.
	//@param p_binding_index Specifies the index of the vertex buffer binding point to which to bind the buffer. Must be less than GL_MAX_VERTEX_ATTRIB_BINDINGS.
	//@param p_buffer Specifies the name of a buffer to bind to the vertex buffer binding point.
	//@param p_offset Specifies the offset of the first element of the buffer in basic machine units.
	//@param p_stride Specifies the distance between elements within the buffer in basic machine units.
	void vertex_array_vertex_buffer(GLHandle p_VAO, GLuint p_binding_index, GLHandle p_buffer, GLintptr p_offset, GLsizei p_stride);
	// Binds a buffer object p_buffer to the element array buffer bind point of a vertex array object p_VAO.
	// If p_buffer is zero, any existing element array buffer binding to p_VAO is removed.
	//@param p_VAO Specifies the name of the vertex array object to bind the buffer to.
	//@param p_buffer Specifies the name of an index buffer to bind to the element array buffer bind point.
	void vertex_array_element_buffer(GLHandle p_VAO, GLHandle p_buffer);

	// Creates and initializes a buffer object's immutable data store
	//@param p_buffer Name of the buffer object to create and initialize.
	//@param p_size Size in bytes of the buffer object's new data store.
	//@param p_data Pointer to data that will be copied into the data store for initialization, or nullptr if no data is to be copied.
	//@param p_flags Bitwise combination of flags that specify the intended usage of the buffer's data store.
	void named_buffer_storage(GLHandle p_buffer, GLsizeiptr p_size, const void* p_data, BufferStorageBitfield p_flags);
	// Update a subset of a Buffer object's data store.
	// Redefines some or all of the data store for the buffer object p_buffer.
	// Data starting at byte p_offset and extending for p_size bytes is copied to the data store from the memory pointed to by p_data.
	// An error is thrown if offset and size together define a range beyond the bounds of the buffer object's data store.
	//@param p_buffer Name of the buffer object to update.
	//@param p_offset Offset into the buffer object's data store where data replacement will begin, measured in bytes.
	//@param p_size Size in bytes of the data store region being replaced.
	//@param p_data Pointer to the new data that will be copied into the data store.
	void named_buffer_sub_data(GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size, const void* p_data);
	// Copy all or part of the data store of a buffer object to the data store of another buffer object.
	// Copy part of the data store attached to p_source_target to the data store attached to p_destination_target.
	// The number of basic machine units indicated by p_size is copied from the p_source_target at p_source_offset to p_destination_target at p_destination_offset.
	//@param p_source_buffer Name of the buffer object from which data will be copied.
	//@param p_destination_buffer Name of the buffer object to which data will be copied.
	//@param p_source_offset Offset, in basic machine units, from the start of p_source_target at which data will be read.
	//@param p_destination_offset Offset, in basic machine units, from the start of p_destination_target at which data will be written.
	//@param p_size Size, in basic machine units, of the data to be copied from the p_source_offset to p_destination_target.
	void copy_named_buffer_sub_data(GLHandle p_source_buffer, GLHandle p_destination_buffer, GLintptr p_source_offset, GLintptr p_destination_offset, GLsizeiptr p_size);
	// Bind a range within a buffer object to an indexed buffer target.
	// Binds the range of the p_buffer represented by p_offset and p_size, to the binding point at p_index of the array of targets specified by p_target.
	// Each p_target represents an indexed array of buffer binding points, as well as a single general binding point that can be used by other buffer manipulation functions such as bind_buffer or map_buffer.
	// In addition to binding a range of buffer to the indexed buffer binding target, bind_buffer_range also binds the range to the generic buffer binding point specified by target.
	// Calling bind_buffer_range with p_offset 0 and p_size equal to the size of the p_target buffer is equivalent to bind_buffer_base.
	//@param p_target Specify Target of the bind operation. target must be one of AtomicCounterBuffer, TransformFeedbackBuffer, UniformBuffer, or ShaderStorageBuffer.
	//@param p_index Index of the binding point within the array specified by target.
	//@param p_buffer Name of a p_buffer object to bind to the specified binding point.
	//@param p_offset Starting offset in basic machine units into the p_buffer object buffer.
	//@param p_size Amount of data in machine units that can be read from the p_buffer object while used as an indexed target.
	void bind_buffer_range(BufferType p_target, GLuint p_index, GLHandle p_buffer, GLintptr p_offset, GLsizeiptr p_size);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CONVERSION FUNCS
namespace OpenGL
{
	GLenum convert(BufferType p_buffer_type);
	GLenum convert(BufferDataType p_data_type);
	GLenum convert(BufferStorageFlag p_flag);

	// Convert a GLEnum value to the ShaderDataType wrapper.
	ShaderDataType convert(GLenum p_data_type);
	GLenum convert(ShaderDataType p_data_type);
	GLenum convert(ShaderProgramType p_shader_program_type);

	GLenum convert(DepthTestType p_depth_test_type);
	GLenum convert(BlendFactorType p_blend_factor_type);
	GLenum convert(CullFaceType p_cull_faces_type);
	GLenum convert(FrontFaceOrientation p_front_face_orientation);
	GLenum convert(PolygonMode p_polygon_mode);
	GLenum convert(PrimitiveMode p_primitive_mode);
}