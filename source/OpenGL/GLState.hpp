#pragma once

#include "glm/fwd.hpp"
#include <string>

using GLint      = int;
using GLuint     = unsigned int;
using GLboolean  = unsigned char;
using GLenum     = unsigned int;
using GLsizei    = int;
using GLsizeiptr = signed long long int;
using GLintptr   = signed long long int;
using GLHandle   = unsigned int; // A GLHandle is an ID used by OpenGL to point to memory owned by this OpenGL context on the GPU.
//using GLenum   = Corresponding enum type in OpenGL namespace
//using GLdouble = double; // Unused
//using GLfloat  = float; // Unused

// Define wrappers to strongly type GLenum types.
// All the wrappers provide helper functions to extract the values or string representations.
namespace OpenGL
{
    constexpr inline static bool LogGLTypeEvents = false;

    enum class DataType : uint8_t
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
    enum class GLSLVariableType : uint8_t
    {
        Uniform,      // 'loose' uniform variables not part of any interface blocks. These are owned by a shader program and can be directly set without buffer backing.
        UniformBlock, // UniformBlockVariable found inside a UniformBlock. Has to be backed by a UBO to be set. These can be global or exclusive to the shader depending on the UniformBlock layout definition.
        BufferBlock   // ShaderStorageBlockVariable found inside a ShaderStorageBlock. Has to be backed by a SSBO to be set. These can be global or exclusive to the shader depending on the ShaderStorageBlock layout definition.
    };
    // Usage can be broken down into two parts:
    // 1. The frequency of access (modification and usage). The frequency of access may be one of these:
    //  1.a.    STREAM:  Modified once and used at most a few times.
    //  1.b.    STATIC:  Modified once and used many times.
    //  1.c.    DYNAMIC: Modified repeatedly and used many times.
    // 2. The nature of that access. Can be one of these:
    //  2.a.    DRAW: Modified by the application, and used as the source for GL drawing and image specification commands.
    //  2.b.    READ: Modified by reading data from the GL, and used to return that data when queried by the application.
    //  2.c.    COPY: Modified by reading data from the GL, and used as the source for GL drawing and image specification commands.
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
    enum class BufferUsage : uint8_t
    {
        StreamDraw,
        StreamRead,
        StreamCopy,
        StaticDraw,
        StaticRead,
        StaticCopy,
        DynamicDraw,
        DynamicRead,
        DynamicCopy
    };
    enum class ShaderProgramType : uint8_t
    {
        Vertex,
        Geometry,
        Fragment
    };
    enum class ShaderResourceType : uint8_t
    {
        Uniform,
        UniformBlock,
        ShaderStorageBlock,
        BufferVariable,
        Buffer,
        ProgramInput,
        ProgramOutput,
        AtomicCounterBuffer,
        //AtomicCounterShader,
        VertexSubroutineUniform,
        FragmentSubroutineUniform,
        GeometrySubroutineUniform,
        ComputeSubroutineUniform,
        TessControlSubroutineUniform,
        TessEvaluationSubroutineUniform,
        TransformFeedbackBuffer,
        TransformFeedbackVarying
    };
    enum class ShaderResourceProperty : uint8_t
    {
        NameLength,
        Type,
        ArraySize,
        Offset,
        BlockIndex,
        ArrayStride,
        MatrixStride,
        IsRowMajor,
        AtomicCounterBufferIndex,
        TextureBuffer,
        BufferBinding,
        BufferDataSize,
        NumActiveVariables,
        ActiveVariables,
        ReferencedByVertexShader,
        ReferencedByTessControlShader,
        ReferencedByTessEvaluationShader,
        ReferencedByGeometryShader,
        ReferencedByFragmentShader,
        ReferencedByComputeShader,
        NumCompatibleSubroutines,
        CompatibleSubroutines,
        TopLevelArraySize,
        TopLevelArrayStride,
        Location,
        LocationIndex,
        IsPerPatch,
        LocationComponent,
        TransformFeedbackBufferIndex,
        TransformFeedbackBufferStride
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
    enum class CullFacesType : uint8_t
{
        Back,          // Culls only the back faces (Default OpenGL setting).
        Front,         // Culls only the front faces.
        FrontAndBack  // Culls both the front and back faces.
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
        Fill  // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon. (Default OpenGL setting).
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
    enum class FramebufferTarget : uint8_t
    {
        DrawFramebuffer,
        ReadFramebuffer,
        Framebuffer
    };
    enum class ErrorType : uint8_t
    {
        InvalidEnum,
        InvalidValue,
        InvalidOperation,
        InvalidFramebufferOperation,
        OutOfMemory,
        StackUnderflow,
        StackOverflow
    };
    enum class Function : uint8_t
    {
        Viewport,
        DrawElements,
        DrawArrays,
        DrawElementsInstanced,
        DrawArraysInstanced,
        BindFramebuffer,
        create_shader,
        shader_source,
        compile_shader,
        create_program,
        attach_shader,
        link_program,
        delete_shader,
        use_program,
        BindBuffer,
        DeleteBuffer,
        BufferData,
        buffer_sub_data,
        bind_buffer_range,
        uniform_block_binding,
        shader_storage_block_binding,
        copy_buffer_sub_data
    };
}

// OpenGL functions using the strongly typed enums defined by us.
namespace OpenGL
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GENERAL STATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void set_depth_test(bool p_depth_test);
	void set_depth_test_type(DepthTestType p_type);
	// Specifies if objects with alpha values <1 should be blended using function set with set_blend_func().
    void toggle_blending(bool p_blend);
    // Specifies how the RGBA factors of source and destination are blended to give the final pixel colour when encountering transparent objects.
    void set_blend_func(BlendFactorType p_source_factor, BlendFactorType p_destination_factor);
    // Specifies if facets specified by set_front_face_orientation are candidates for culling.
    void toggle_cull_face(bool p_cull);
    // Specifies which facets are candidates for culling.
    void set_cull_face_type(CullFacesType p_cull_face_type);
    // The orientation of front-facing polygons. Used to mark facets for culling.
    void set_front_face_orientation(FrontFaceOrientation p_front_face_orientation);
    // The red, green, blue, and alpha values to clear the color buffers. Values are clamped to the range 0-1.
    void set_clear_colour(const glm::vec4& p_colour);
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
    // Selects active texture unit subsequent texture state calls will affect. The number of texture units an implementation supports is implementation dependent, but must be at least 80.
    void set_active_texture(int pUnitPosition);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DRAW FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    void draw_arrays_instanced(PrimitiveMode p_primitive_mode, int p_array_size, int p_instance_count);
    // Render primitives from array data.
    // draw_elements specifies multiple geometric primitives with very few subroutine calls. Instead of calling a GL function to pass each individual vertex, normal, texture coordinate, edge flag, or color, you can prespecify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
    // When draw_elements is called, it uses pElementSize sequential elements from an enabled array, starting at indices to construct a sequence of geometric primitives. p_primitive_mode specifies what kind of primitives are constructed and how the array elements construct these primitives. If more than one array is enabled, each is used.
    // Vertex attributes that are modified by draw_elements have an unspecified value after draw_elements returns. Attributes that aren't modified maintain their previous values.
    void draw_elements(PrimitiveMode p_primitive_mode, int pElementsSize);
    // Draw multiple instances of a set of elements.
    // draw_elements_instanced behaves identically to draw_elements except that p_instance_count of the set of elements are executed and the value of the internal counter instanceID advances for each iteration.
    // instanceID is an internal 32-bit integer counter that may be read by a vertex shader as gl_InstanceID.
    void draw_elements_instanced(PrimitiveMode p_primitive_mode, int pElementsSize, int p_instance_count);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHADER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Creates an empty program object and returns a non-zero value by which it can be referenced.
    // A program object is an object to which shader objects can be attached. This provides a mechanism to specify the shader objects that will be linked to create a program.
    // It also provides a means for checking the compatibility of the shaders that will be used to create a program (for instance, checking the compatibility between a vertex shader and a fragment shader).
    // When no longer needed as part of a program object, shader objects can be detached.
    GLHandle create_program();
    // Deletes a program object and frees the memory.
    // Invalidates the p_shader_program associated with the program object.
    // This command effectively undoes the effects of a call to create_program.
    // If a program object is in use as part of current rendering state, it will be flagged for deletion, but it will not be deleted until it is no longer part of current state for any rendering context.
    // If a program object to be deleted has shader objects attached to it, those shader objects will be automatically detached but not deleted unless they have already been flagged for deletion by a previous call to delete_shader.
    // To determine whether a program object has been flagged for deletion, call glGetProgram with arguments program and GL_DELETE_STATUS.
    //@param p_shader_program Shader program object to be deleted. A value of 0 will be silently ignored.
    void delete_program(GLHandle p_shader_program);
    // Installs a program object as part of current rendering state
    // While a program object is in use, applications are free to modify attached shader objects, compile attached shader objects, attach additional shader objects, and detach or delete shader objects.
    // None of these operations will affect the executables that are part of the current state.
    // However, relinking the program object that is currently in use will install the program object as part of the current rendering state if the link operation was successful (glLinkProgram).
    // If the program object currently in use is relinked unsuccessfully, its link status will be set to GL_FALSE, but the executables and associated state will remain part of the current state until a subsequent call to glUseProgram removes it from use.
    // After it is removed from use, it cannot be made part of current state until it has been successfully relinked.
    void use_program(GLHandle p_shader_program);
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
    GLint get_max_uniform_binding_points();
    // Get the number of ShaderStorageBlock/SSBO binding points available.
    GLint get_max_shader_storage_binding_points();
    // Get the max size in bytes a UniformBlock can have.
    GLint get_max_uniform_block_size();
    // Get the max size in bytes a ShaderStorageBlock can have (only pertains to the fixed-size portion of the block, size can exceed this ignoring variable sized arrays).
    GLint get_max_shader_storage_block_size();
    // Returns the shader program object that is currently active, or 0 if no program object is active.
    GLHandle get_current_shader_program();

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DATA/BUFFER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Enables the generic vertex attribute array specified by p_index.
    // If enabled, the values in the generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands such as draw_arrays, draw_elements.
    // By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
    //@param p_index The generic vertex attribute to be enabled.
    void enable_vertex_attrib_array(GLuint p_index);
    // Disables the generic vertex attribute array specified by p_index.
    // If disabled, the values in the generic vertex attribute array won't be accessed and used for rendering when calls are made to vertex array commands such as draw_arrays, draw_elements.
    // By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
    //@param p_index Index of the generic vertex attribute to be disabled.
    void disable_vertex_attrib_array(GLuint p_index);
    // Specify the location and data format of the array of generic vertex attributes at index p_index to use when rendering.
    //@param p_index Index of the generic vertex attribute to be modified.
    //@param p_size Number of components per attribute and must be 1, 2, 3, 4, or GL_BGRA.
    //@param p_type Data type of each component,
    //@param p_normalized Indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed
    // and converted to floating point. Otherwise, values will be converted to floats directly without normalization.
    //@param p_stride Byte stride from one attribute to the next, allowing vertices and attributes to be packed into a single array or stored in separate arrays.
    //@param p_pointer Offset of the first component of the first generic vertex attribute in the array in the data store of the buffer currently bound to the GL_ARRAY_BUFFER target. The initial value is 0.
    void vertex_attrib_pointer(GLuint p_index, GLint p_size, DataType p_type, GLboolean p_normalized, GLsizei p_stride, const void* p_pointer);
    // Creates and initializes a buffer object's data store. The Buffer currently bound to target is used.
    // While creating the new storage, any pre-existing data store is deleted. The new data store is created with the specified size in bytes and usage.
    // When replacing the entire data store, consider using buffer_sub_data rather than completely recreating the data store with buffer_data. This avoids the cost of reallocating the data store.
    // If p_data is not NULL, the data store is initialized with data from this pointer. In its initial state, the new data store is not mapped, it has a NULL mapped pointer, and its mapped access is GL_READ_WRITE.
    // It does not, however, constrain the actual usage of the data store.
    // If p_data is NULL, a data store of the specified size is still created, but its contents remain uninitialized and thus undefined.
    // Clients must align data elements consistently with the requirements of the client platform, with an additional base-level requirement: an offset within a buffer to a datum comprising N bytes be a multiple of N.
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
    //@param p_target Target to which the buffer object being buffered to is bound.
    //@param p_size Size in bytes of the buffer object's new data store.
    //@param p_data Pointer to data that will be copied into the data store for initialization, or NULL if no data is to be copied.
    //@param p_usage Hint to the GL implementation on how a buffer will be accessed. This enables OpenGL to make more intelligent decisions that may impact buffer performance.
    void buffer_data(BufferType p_target, GLsizeiptr p_size, const void* p_data, BufferUsage p_usage);
    // Update a subset of a Buffer object's data store.
    // Redefines some or all of the data store for the buffer object currently bound to p_target.
    // Data starting at byte offset p_offset and extending for p_size bytes is copied to the data store from the memory pointed to by p_data.
    // An error is thrown if offset and size together define a range beyond the bounds of the buffer object's data store.
    //@param p_target Target to which the buffer object being buffered to is bound.
    //@param p_offset Offset into the buffer object's data store where data replacement will begin, measured in bytes.
    //@param p_size Size in bytes of the data store region being replaced.
    //@param p_data Pointer to the new data that will be copied into the data store.
    void buffer_sub_data(BufferType p_target, GLintptr p_offset, GLsizeiptr p_size, const void* p_data);
    // Copy all or part of the data store of a buffer object to the data store of another buffer object.
    // Copy part of the data store attached to p_source_target to the data store attached to p_destination_target.
    // The number of basic machine units indicated by p_size is copied from the p_source_target at p_source_offset to p_destination_target at p_destination_offset.
    //@param p_source_target Target to which the source buffer object is bound.
    //@param p_destination_target Target to which the destination buffer object is bound.
    //@param p_source_offset Offset, in basic machine units, within the data store of p_source_target at which data will be read.
    //@param p_destination_offset Offset, in basic machine units, within the data store of p_destination_target at which data will be written.
    //@param p_size Size, in basic machine units, of the data to be copied from the p_source_offset to p_destination_target.
    void copy_buffer_sub_data(BufferType p_source_target, BufferType p_destination_target, GLintptr p_source_offset, GLintptr p_destination_offset, GLsizeiptr p_size);
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

// Remainder are conversion functions for GLEnum wrappers to
namespace OpenGL
{
    const char* get_name(Function p_function);
    const char* get_name(ShaderProgramType p_shader_program_type);
    const char* get_name(DataType p_data_type);
    const char* get_name(GLSLVariableType p_variable_type);
    const char* get_name(BufferType p_buffer_type);
    const char* get_name(BufferUsage p_buffer_usage);
    const char* get_name(ShaderResourceType p_resource_type);
    const char* get_name(ShaderResourceProperty p_shader_resource_property);
    const char* get_name(DepthTestType p_depth_test_type);
    const char* get_name(BlendFactorType p_blend_factor_type);
    const char* get_name(CullFacesType p_cull_faces_type);
    const char* get_name(FrontFaceOrientation p_front_face_orientation);
    const char* get_name(PolygonMode p_polygon_mode);
    const char* get_name(PrimitiveMode p_primitive_mode);
    const char* get_name(FramebufferTarget p_framebuffer_target);
    const char* get_name(ErrorType p_error_type);

    // Convert a GLEnum value to the DataType wrapper.
    DataType convert(int p_data_type);
    // Assert the OpenGL p_type and type T match up. Used to runtime assert the DataType of variables matches the templated set functions.
    template <typename T>
    constexpr bool assert_type(DataType p_type)
    {
        switch (p_type)
        {
            case DataType::Float:                return std::is_same_v<T, float>;
            case DataType::Double:               return std::is_same_v<T, double>;
            case DataType::Int:                  return std::is_same_v<T, int>;
            case DataType::UnsignedInt:          return std::is_same_v<T, unsigned int>;
            case DataType::Bool:                 return std::is_same_v<T, bool>;
            case DataType::Sampler2D:            return false; // Setting texture sampler types uses int to set their bound texture unit. The actual texture being sampled is set by setting active an texture unit and using bindTexture.
            case DataType::SamplerCube:          return false;
            case DataType::Vec2:                 return std::is_same_v<T, glm::vec2>;
            case DataType::Vec3:                 return std::is_same_v<T, glm::vec3>;
            case DataType::Vec4:                 return std::is_same_v<T, glm::vec4>;
            case DataType::DVec2:                return std::is_same_v<T, glm::dvec2>;
            case DataType::DVec3:                return std::is_same_v<T, glm::dvec3>;
            case DataType::DVec4:                return std::is_same_v<T, glm::dvec4>;
            case DataType::IVec2:                return std::is_same_v<T, glm::ivec2>;
            case DataType::IVec3:                return std::is_same_v<T, glm::ivec3>;
            case DataType::IVec4:                return std::is_same_v<T, glm::ivec4>;
            case DataType::UVec2:                return std::is_same_v<T, glm::uvec2>;
            case DataType::UVec3:                return std::is_same_v<T, glm::uvec3>;
            case DataType::UVec4:                return std::is_same_v<T, glm::uvec4>;
            case DataType::BVec2:                return std::is_same_v<T, glm::bvec2>;
            case DataType::BVec3:                return std::is_same_v<T, glm::bvec3>;
            case DataType::BVec4:                return std::is_same_v<T, glm::bvec4>;
            case DataType::Mat2:                 return std::is_same_v<T, glm::mat2>;
            case DataType::Mat3:                 return std::is_same_v<T, glm::mat3>;
            case DataType::Mat4:                 return std::is_same_v<T, glm::mat4>;
            case DataType::Mat2x3:               return std::is_same_v<T, glm::mat2x3>;
            case DataType::Mat2x4:               return std::is_same_v<T, glm::mat2x4>;
            case DataType::Mat3x2:               return std::is_same_v<T, glm::mat3x2>;
            case DataType::Mat3x4:               return std::is_same_v<T, glm::mat3x4>;
            case DataType::Mat4x2:               return std::is_same_v<T, glm::mat4x2>;
            case DataType::Mat4x3:               return std::is_same_v<T, glm::mat4x3>;
            case DataType::Dmat2:                return std::is_same_v<T, glm::dmat2>;
            case DataType::Dmat3:                return std::is_same_v<T, glm::dmat3>;
            case DataType::Dmat4:                return std::is_same_v<T, glm::dmat4>;
            case DataType::Dmat2x3:              return std::is_same_v<T, glm::dmat2x3>;
            case DataType::Dmat2x4:              return std::is_same_v<T, glm::dmat2x4>;
            case DataType::Dmat3x2:              return std::is_same_v<T, glm::dmat3x2>;
            case DataType::Dmat3x4:              return std::is_same_v<T, glm::dmat3x4>;
            case DataType::Dmat4x2:              return std::is_same_v<T, glm::dmat4x2>;
            case DataType::Dmat4x3:              return std::is_same_v<T, glm::dmat4x3>;
            case DataType::Sampler1D:            return false; // Remaining types have not been implemented.
            case DataType::Sampler3D:            return false;
            case DataType::Sampler1DShadow:      return false;
            case DataType::Sampler2DShadow:      return false;
            case DataType::Sampler1DArray:       return false;
            case DataType::Sampler2DArray:       return false;
            case DataType::Sampler1DArrayShadow: return false;
            case DataType::Sampler2DArrayShadow: return false;
            case DataType::Sampler2DMS:          return false;
            case DataType::Sampler2DMSArray:     return false;
            case DataType::SamplerCubeShadow:    return false;
            case DataType::SamplerBuffer:        return false;
            case DataType::Sampler2DRect:        return false;
            case DataType::Sampler2DRectShadow:  return false;
            case DataType::Isampler1D:           return false;
            case DataType::Isampler2D:           return false;
            case DataType::Isampler3D:           return false;
            case DataType::IsamplerCube:         return false;
            case DataType::Isampler1DArray:      return false;
            case DataType::Isampler2DArray:      return false;
            case DataType::Isampler2DMS:         return false;
            case DataType::Isampler2DMSArray:    return false;
            case DataType::IsamplerBuffer:       return false;
            case DataType::Isampler2DRect:       return false;
            case DataType::Usampler1D:           return false;
            case DataType::Usampler2D:           return false;
            case DataType::Usampler3D:           return false;
            case DataType::UsamplerCube:         return false;
            case DataType::Usampler2DArray:      return false;
            case DataType::Usampler2DMS:         return false;
            case DataType::Usampler2DMSArray:    return false;
            case DataType::UsamplerBuffer:       return false;
            case DataType::Usampler2DRect:       return false;
            case DataType::Unknown:              return false;
            default:                             return false;
        }
    }


    int convert(ShaderProgramType p_shader_program_type);
    int convert(DataType p_data_type);
    int convert(GLSLVariableType p_variable_type);
    int convert(BufferType p_buffer_type);
    int convert(BufferUsage p_buffer_usage);
    int convert(ShaderResourceType p_resource_type);
    int convert(ShaderResourceProperty p_shader_resource_property);
    int convert(DepthTestType p_depth_test_type);
    int convert(BlendFactorType p_blend_factor_type);
    int convert(CullFacesType p_cull_faces_type);
    int convert(FrontFaceOrientation p_front_face_orientation);
    int convert(PolygonMode p_polygon_mode);
    int convert(PrimitiveMode p_primitive_mode);
    int convert(FramebufferTarget p_framebuffer_target);
}