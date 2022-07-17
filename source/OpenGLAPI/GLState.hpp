#pragma once

#include "glm/fwd.hpp"

#include "Logger.hpp"

#include <string>
#include <array>
#include <vector>
#include <optional>

// Wraps all the GL types into enums and provides helper functions to extract the values or string representations.
// All the enums come with a matching array to allow iterating over the enums in ImGui and converting to string by O(1) indexing.
namespace GLType
{
    enum class DataType
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
        Count
    };
    enum class BufferType
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
    enum class ShaderProgramType
    {
        Vertex,
        Geometry,
        Fragment,
        Count
    };
    enum class ShaderResourceType
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
        TransformFeedbackVarying,

        Count
    };
    enum class ShaderResourceProperty
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
        TransformFeedbackBufferStride,

        Count
    };
    enum class DepthTestType
    {
        Always,
        Never,
        Less,
        Equal,
        NotEqual,
        Greater,
        LessEqual,
        GreaterEqual,
        Count
    };
    enum class BlendFactorType
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
        Count
    };
    enum class CullFacesType
    {
        Back,          // Culls only the back faces (Default OpenGL setting).
        Front,         // Culls only the front faces.
        FrontAndBack,  // Culls both the front and back faces.
        Count
    };
    enum class FrontFaceOrientation
    {
        Clockwise,          // Clockwise polygons are identified as front-facing.
        CounterClockwise,   // Counter-clockwise polygons are identified as front-facing (Default OpenGL setting).
        Count
    };
    // Polygon rasterization mode
    // Vertices are marked as boundary/non-boundary with an edge flag generated internally by OpenGL when it decomposes triangle stips and fans.
    enum class PolygonMode
    {
        Point, // Polygon vertices that are marked as the start of a boundary edge are drawn as points. Point attributes such as GL_POINT_SIZE and GL_POINT_SMOOTH control the rasterization of the points.
        Line,  // Boundary edges of the polygon are drawn as line segments. Line attributes such as GL_LINE_WIDTH and GL_LINE_SMOOTH control the rasterization of the lines.
        Fill,  // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon. (Default OpenGL setting).
        Count
    };
    // Interpretation scheme used to determine what a stream of vertices represents when being rendered.
    enum class PrimitiveMode
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
        Count
    };
    enum class FramebufferTarget
    {
        DrawFramebuffer,
        ReadFramebuffer,
        Framebuffer,
        Count
    };
    enum class ErrorType
    {
        InvalidEnum,
        InvalidValue,
        InvalidOperation,
        InvalidFramebufferOperation,
        OutOfMemory,
        StackUnderflow,
        StackOverflow,
        Count
    };
    enum class Function
    {
        UniformBlockBinding,
        Viewport,
        DrawElements,
        DrawArrays,
        BindFramebuffer,
        CreateShader,
        ShaderSource,
        CompileShader,
        CreateProgram,
        AttachShader,
        LinkProgram,
        DeleteShader,
        UseProgram,
        BindBuffer,
        DeleteBuffer,
        Count
    };

    std::string toString(const Function& pFunction);
    std::string toString(const ShaderProgramType& pShaderProgramType);
    std::string toString(const DataType &pDataType);
    std::string toString(const BufferType& pBufferType);
    std::string toString(const ShaderResourceType &pResourceType);
    std::string toString(const ShaderResourceProperty &pShaderResourceProperty);
    std::string toString(const DepthTestType &pDepthTestType);
    std::string toString(const BlendFactorType &pBlendFactorType);
    std::string toString(const CullFacesType &pCullFacesType);
    std::string toString(const FrontFaceOrientation &pFrontFaceOrientation);
    std::string toString(const PolygonMode &pPolygonMode);
    std::string toString(const PrimitiveMode &pPrimitiveMode);
    std::string toString(const FramebufferTarget &pFramebufferTarget);
    std::string toString(const ErrorType &pErrorType);

    DataType convert(const int& pDataType);

    int convert(const ShaderProgramType& pShaderProgramType);
    int convert(const DataType& pDataType);
    int convert(const BufferType& pBufferType);
    int convert(const ShaderResourceType& pResourceType);
    int convert(const ShaderResourceProperty& pShaderResourceProperty);
    int convert(const DepthTestType& pDepthTestType);
    int convert(const BlendFactorType& pBlendFactorType);
    int convert(const CullFacesType& pCullFacesType);
    int convert(const FrontFaceOrientation& pFrontFaceOrientation);
    int convert(const PolygonMode& pPolygonMode);
    int convert(const PrimitiveMode& pPrimitiveMode);
    int convert(const FramebufferTarget& pFramebufferTarget);
}

class GLState;
// Wraps OpenGL data types that hold GPU data. Each type follows the same class structure:
// generate() - Creates a handle that data can be bound to.
// bind()     - Makes the data 'current'
// release()  - Deletes the data on the GPU freeing the handle (has to be called before destruction).
namespace GLData
{
    // Vertex Array Object (VAO)
    // Stores all of the state needed to supply vertex data. VAO::bind() needs to be called before setting the state using VBO's and EBO's.
    // It stores the format of the vertex data as well as the Buffer Objects (see below) providing the vertex data arrays.
    // Note: If you change any of the data in the buffers referenced by an existing VAO (VBO/EBO), those changes will be seen by users of the VAO.
	struct VAO
	{
		void generate();
		void bind() const;
		void release();
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};

    // Stores an array of memory allocated using OpenGL context on the GPU.
    struct Buffer
    {
        void Bind(const GLState& pGLState) const;
        void Free(const GLState& pGLState);

        const GLType::BufferType mBufferType;
        const unsigned int mHandle;

    protected:
        Buffer(const GLType::BufferType& pBufferType, const GLState& pGLState);
    private:
        size_t mSize; // The size of the buffer in Bytes
    };

    // Vertex Buffer Object
    // Buffer storing per-vertex array data.
    struct VBO : public Buffer
	{
        VBO(const GLState& pGLState);
        void PushData(const GLState& pGLState, const std::vector<float>& pData, const int& pAttributeIndex, const int& pAttributeSize);
	};
    // Element Buffer Object
    // Buffer storing vertex index data defining which order to draw vertex data stored in a VBO.
    // EBO's are used only if a mesh uses Indexed drawing.
	struct EBO : public Buffer
	{
        EBO(const GLState& pGLState);
        void PushData(const GLState& pGLState, const std::vector<int>& pData);
	};
    // Uniform Buffer Object
    // A Buffer Object that is used to store uniform data for a shader program. They can be used to share
    // uniforms between different programs, as well as quickly change between sets of uniforms for the same program object.
    // Used to provide buffer-backed storage for uniforms.
    // https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object
    struct UBO : public Buffer
    {
        UBO(const GLState& pGLState);
        void PushData(const GLState& pGLState, const int& pBufferSizeBytes, const unsigned int& pBindingPoint);
    };
    // A Shader Storage block Object (SSBO)
    // SSBOs are a lot like Uniform Buffer Objects (UBO's). SSBOs are bound to ShaderStorageBlockBindingPoint's, just as UBO's are bound to UniformBlockBindingPoint's.
    // Compared to UBO's SSBOs:
    // Can be much larger. The spec guarantees that SSBOs can be up to 128MB. Most implementations will let you allocate a size up to the limit of GPU memory.
    // Are writable, even atomically. SSBOs reads and writes use incoherent memory accesses, so they need the appropriate barriers, just as Image Load Store operations.
    // Can have variable storage, up to whatever buffer range was bound for that particular buffer. This means that you can have an array of arbitrary length in an SSBO (at the end, rather).
    // The actual size of the array, based on the range of the buffer bound, can be queried at runtime in the shader using the length function on the unbounded array variable.
    // SSBO access will likely be slower than UBO access. At the very least, UBOs will be no slower than SSBOs.
    // https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
    struct SSBO : public Buffer
    {
        SSBO(const GLState& pGLState);
        void PushData(const GLState& pGLState, const int& pBufferSizeBytes, const unsigned int& pBindingPoint);
    };

    // OpenGL Texture object.
    // Represents a texture pushed to the GPU.
    struct Texture
	{
        friend struct FBO;

        enum class Type
        {
            Texture2D, // Texture target: GL_TEXTURE_2D
            CubeMap, // Texture target: GL_TEXTURE_CUBE_MAP
            None
        };
        Texture(const Type& pType) : mType(pType) {}
        Texture() = default;

		void generate();
		void bind() const;
        // Pushes the texture data using glTexImage2D.
        // If pCubeMapIndexOffset is supplied the data is pushed to GL_TEXTURE_CUBE_MAP at index GL_TEXTURE_CUBE_MAP_POSITIVE_X (0) + offset to GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (5).
        void pushData(const int& pWidth, const int& pHeight, const int& pNumberOfChannels, const unsigned char* pData, const int& pCubeMapIndexOffset = -1);
		void release();
		unsigned int getHandle() const { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
        Type mType              = Type::None;
	};

    // A shader-program global variable.
    // Declared with the "uniform" storage qualifier in GLSL or inside a Uniform Buffer Object (UBO).
    // Uniform variables don't change from one shader invocation to the next within a particular rendering call thus their value is uniform among all invocations. This makes them unlike shader stage inputs and outputs, which are often different for each invocation of a shader stage.
    // These act as parameters that the user of a shader program can pass to that program. Their values are stored in a program object.
    // https://www.khronos.org/opengl/wiki/Uniform_(GLSL)
    struct UniformVariable
    {
        std::string mName      = "";
        GLType::DataType mType = GLType::DataType::Count;
        int mOffset            = -1;  // Number of bytes (basic machine units) from the beginning of the buffer to the memory location for this variable
        int mLocation          = -1;
        int mBlockIndex        = -1;
        int mArraySize         = -1;  // For elements that are aggregated into arrays, this is the number of elements in the array
        int mArrayStride       = -1;  // For elements that are aggregated into arrays, this is the number of bytes from the start of one element to the start of the next one
        int mMatrixStride      = -1;  // Number of bytes from the start of one column/row vector to the next column/row (depending on whether the matrix is laid out as column-major or row-major)
        int mIsRowMajor        = -1;  // For matrix elements, the column/row-major ordering.
    };
    // UniformBlocks are GLSL interface blocks which group UniformVariable's.
    // UniformBlock's can be buffer-backed using UniformBufferObjects allowing data to be shared across shader-programs.
    // buffer-backed blocks declared shared can be used with any program that defines a block with the same elements in the same order.
    // Matching blocks in different shader stages will, when linked into the same program, be presented as a single interface block.
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Uniform_blocks
    struct UniformBlock
    {
        std::string mName           = "";
        int mBlockIndex             = -1; // Index of the UniformBlock in its parentShader.
        int mParentShaderHandle     = -1;
        int mBufferDataSize         = -1; // Size of the block in bytes.
        int mActiveVariablesCount   = -1; // The number of UniformVariables this block contains.

        std::optional<unsigned int> mBindingPoint; // The binding of the block to a corrresponding UniformBlockBindingPoint
        std::vector<UniformVariable> mVariables;
        std::vector<int> mVariableIndices;
    };
    // A GLSL variable that belongs to a ShaderStorageBlock
    struct ShaderStorageBlockVariable
    {
        std::string mName        = "";
        GLType::DataType mType   = GLType::DataType::Count;
        int mOffset              = -1;  // Number of bytes (basic machine units) from the beginning of the buffer to the memory location for this variable
        int mBlockIndex          = -1;
        int mArraySize           = -1;  // For elements that are aggregated into arrays, this is the number of elements in the array
        int mArrayStride         = -1;  // For elements that are aggregated into arrays, this is the number of bytes from the start of one element to the start of the next one
        int mMatrixStride        = -1;  // Number of bytes from the start of one column/row vector to the next column/row (depending on whether the matrix is laid out as column-major or row-major)
        int mIsRowMajor          = -1;  // For matrix elements, the column/row-major ordering.
        // The number of active array elements of the top-level shader storage block member.
        // If the top-level block member is not declared as an array, the value is '1'. If the top-level block member is an array with no declared size, the value zero is written to params.
        int mTopLevelArraySize   = -1;
        // The stride between array elements of the top-level shader storage block member.
        // For top-level block members declared as arrays, the value written is the difference, in basic machine units, between the offsets of the active variable for consecutive elements in the top-level array.
        // For top-level block members not declared as an array, the value is '0'.
        int mTopLevelArrayStride = -1;
    };

    // ShaderStorageBlock's are GLSL interface blocks which group ShaderStorageBlockVariable's.
    // ShaderStorageBlock's can be buffer-backed using SSBO's allowing data to be shared across shader-programs.
    // buffer-backed blocks declared shared can be used with any program that defines a block with the same elements in the same order.
    // Matching blocks in different shader stages will, when linked into the same program, be presented as a single interface block.
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Shader_storage_blocks
    // Compared to UniformBlocks ShaderStorageBlocks can store more data, are writable atomically, can have variable storage (array of arbitrary length).
    // ShaderStorageBlock access will likely be slower than UBO access. At the very least, UBOs will be no slower than SSBOs.
    struct ShaderStorageBlock
    {
        std::string mName           = "";
        int mBlockIndex             = -1; // Index of the ShaderStorageBlock in its parentShader.
        int mParentShaderHandle     = -1;
        int mBufferDataSize         = -1; // Size of the block in bytes.
        int mActiveVariablesCount   = -1; // The number of ShaderStorageBlockVariables this block contains.

        std::optional<unsigned int> mBindingPoint; // The binding of the block to a corrresponding ShaderStorageBlockBindingPoint
        std::vector<ShaderStorageBlockVariable> mVariables;
        std::vector<int> mVariableIndices;
    };
    // Render Buffer Object
    // RBO's contain images optimized for use as render targets, and are the logical choice when you do not need to sample (i.e. in a post-pass shader) from the produced image.
    // If you need to resample (such as when reading depth back in a second shader pass), use Texture instead.
    // RBO's are created and used specifically with Framebuffer Objects (FBO's).
    struct RBO
    {
        void generate();
		void bind() const;
        void release();
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
    };
    // Framebuffer object.
    // Allows creation of user-defined framebuffers that can be rendered to without disturbing the main screen.
    struct FBO
    {
        void generate();
		void bind(GLState& pGLState) const;
        void release();
		unsigned int getHandle() { return mHandle; };
        Texture& getColourTexture();
        void clearBuffers();

        void resize(const int& pWidth, const int& pHeight, GLState& pGLState);
        void attachColourBuffer(const int& pWidth, const int& pHeight, GLState& pGLState);
        void detachColourBuffer();
        void attachDepthBuffer(const int& pWidth, const int& pHeight, GLState& pGLState);
        void detachDepthBuffer();
	private:
        std::optional<Texture> mColourAttachment    = std::nullopt;
        std::optional<RBO> mDepthAttachment         = std::nullopt;

		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
        int mBufferClearBitField = 0; // Bit field sent to OpenGL clear buffers before next draw.
    };
}

// Tracks the current GLState and provides helpers to set global GL state using GlTypes.
class GLState
{
public:
    GLState();

    // Copies the state of pOther and sets all the discrepant GL states.
    GLState& operator=(const GLState& pOther);
    // Checks if GLState matches what OpenGL state machine is set to.
    bool validateState();

	void toggleDepthTest(const bool& pDepthTest);
	void setDepthTestType(const GLType::DepthTestType& pType);

	// Specifies if objects with alpha values <1 should be blended using function set with setBlendFunction().
    void toggleBlending(const bool& pBlend);
    // Specifies how the RGBA factors of source and destination are blended to give the final pixel colour when encountering transparent objects.
    void setBlendFunction(const GLType::BlendFactorType& pSourceFactor, const GLType::BlendFactorType& pDestinationFactor);

    // Specifies if facets specified by setFrontFaceOrientation are candidates for culling.
    void toggleCullFaces(const bool& pCull);
    // Specifies which facets are candidates for culling.
    void setCullFacesType(const GLType::CullFacesType& pCullFaceType);
    // Specifies the orientation of front-facing polygons. Used to mark facets for culling.
    void setFrontFaceOrientation(const GLType::FrontFaceOrientation& pFrontFaceOrientation);

    // Specifies the red, green, blue, and alpha values to clear the color buffers. Values are clamped to the range 0-1.
    void setClearColour(const std::array<float, 4>& pColour);
    // Controls the interpretation of polygons for rasterization.
    // pPolygonMode: Specifies how polygons will be rasterized.
    // Affects only the final rasterization of polygons - a polygon's vertices are lit and the polygon is clipped/culled before these modes are applied.
    void setPolygonMode(const GLType::PolygonMode& pPolygonMode);

    // Selects active texture unit subsequent texture state calls will affect. The number of texture units an implementation supports is implementation dependent, but must be at least 80.
    void setActiveTextureUnit(const int& pUnitPosition);

    // Render primitives from array data
    // pPrimitiveMode: What kind of primitives to render.
    // pCount: Number of elements to be rendered.
    // Specifies multiple geometric primitives with very few subroutine calls.
    // Instead of passing each individual vertex, normal, texture coordinate... you can pre-specify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
    void drawElements(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount);
    // Render primitives from array data
    // pPrimitiveMode: What kind of primitives to render.
    // pCount: Number of indices to be rendered.
    // Specifies multiple geometric primitives with very few subroutine calls.
    // Instead of passing each individual vertex, normal, texture coordinate... you can pre-specify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
    void drawArrays(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount);

    // Bind a FBO to a framebuffer target
    void bindFramebuffer(const GLType::FramebufferTarget& pFramebufferTargetType, const unsigned int& pFBOHandle);
    // Unbind any current framebuffer, this binds the default OpenGL framebuffer.
    void unbindFramebuffer();
    void checkFramebufferBufferComplete();

    // Generates a Buffer object handle.
    unsigned int GenBuffers() const;
    // Bind a named buffer object.
    void BindBuffer(const GLType::BufferType& pBufferType, const unsigned int& pBufferHandle) const;
    // Delete named buffer objects.
    void DeleteBuffer(const unsigned int& pBufferHandle) const;

    // Create a shader object.
    // Creates an empty shader object and returns a non-zero value by which it can be referenced. A shader object is used to maintain the source code strings that define a shader.
    // Like buffer and texture objects, the name space for shader objects may be shared across a set of contexts, as long as the server sides of the contexts share the same address space.
    // If the name space is shared across contexts, any attached objects and the data associated with those attached objects are shared as well.
    unsigned int CreateShader(const GLType::ShaderProgramType& pProgramType);

    // Replaces the source code in a shader object
    // Any source code previously stored in the shader object is completely replaced.
    // The source code strings are not scanned or parsed at this time; they are simply copied into the specified shader object.
    void ShaderSource(const unsigned int& pShaderHandle, const std::string& pShaderSource);

    // Compiles a shader object
    void CompileShader(const unsigned int& pShaderHandle);

    // Creates an empty program object and returns a non-zero value by which it can be referenced.
    // A program object is an object to which shader objects can be attached. This provides a mechanism to specify the shader objects that will be linked to create a program.
    // It also provides a means for checking the compatibility of the shaders that will be used to create a program (for instance, checking the compatibility between a vertex shader and a fragment shader).
    // When no longer needed as part of a program object, shader objects can be detached.
    unsigned int CreateProgram();

    // Attaches a shader object to a program object
    // Shaders that are to be linked together in a program object must first be attached to that program object.
    // All operations that can be performed on a shader object are valid whether or not the shader object is attached to a program object.
    // It is permissible to attach a shader object to a program object before source code has been loaded into the shader object or before the shader object has been compiled.
    // It is permissible to attach multiple shader objects of the same type because each may contain a portion of the complete shader.
    // It is also permissible to attach a shader object to more than one program object.
    // If a shader object is deleted while it is attached to a program object, it will be flagged for deletion, and deletion will not occur until DetachShader is called to detach it from all program objects to which it is attached.
    void AttachShader(const unsigned int& pShaderProgramHandle, const unsigned int& pShaderHandle);

    // Links a program object
    // As a result of a successful link operation, all active user-defined uniform variables belonging to program will be initialized to 0
    // The active uniform variables will be assigned a location that can be queried using GL program introspection.
    // Any active user-defined attribute variables that have not been bound to a generic vertex attribute index will be bound to one at this time.
    // When a program object has been successfully linked, the program object can be made part of current state by calling glUseProgram
    // If the program object currently in use is relinked unsuccessfully the executables and associated state will remain part of the current state until a
    // subsequent call to glUseProgram removes it from use. After it is removed from use, it cannot be made part of current state until it has been successfully relinked.
    void LinkProgram(const unsigned int& pShaderProgramHandle);

    // Deletes a shader object
    // Frees the memory and invalidates the name associated with the shader object specified by shader.
    // This command effectively undoes the effects of a call to glCreateShader.
    // If a shader object to be deleted is attached to a program object, it will be flagged for deletion, but it will
    // not be deleted until it is no longer attached to any program object, for any rendering context (i.e. it must be
    // detached from wherever it was attached before it will be deleted).
    // A value of 0 for shader will be silently ignored.
    // To determine whether an object has been flagged for deletion, call glGetShader with arguments shader and GL_DELETE_STATUS.
    void DeleteShader(const unsigned int& pShaderHandle);

    // Installs a program object as part of current rendering state
    // While a program object is in use, applications are free to modify attached shader objects, compile attached shader objects, attach additional shader objects, and detach or delete shader objects.
    // None of these operations will affect the executables that are part of the current state.
    // However, relinking the program object that is currently in use will install the program object as part of the current rendering state if the link operation was successful (glLinkProgram).
    // If the program object currently in use is relinked unsuccessfully, its link status will be set to GL_FALSE, but the executables and associated state will remain part of the current state until a subsequent call to glUseProgram removes it from use.
    // After it is removed from use, it cannot be made part of current state until it has been successfully relinked.
    void UseProgram(const unsigned int& pShaderProgramHandle);

    // Set the global viewport
    // Specify the width and height of the viewport. When a context is first attached to a window, width and height are set to the dimensions of that window.
    // The viewport specifies the affine transformation of x and y from normalized device coordinates to window coordinates.
    // Let x nd y nd be normalized device coordinates. Then the window coordinates x w y w are computed as follows:
    // x w = x nd + 1 ⁢ width 2 + x
    // y w = y nd + 1 ⁢ height 2 + y
    void setViewport(const int& pWidth, const int& pHeight);

    // Outputs the current GLState with options to change flags.
    void renderImGui();


    int getActiveUniformCount(const unsigned int& pShaderProgramHandle) const;
    int getActiveUniformBlockCount(const unsigned int& pShaderProgramHandle) const;
    GLData::UniformBlock getUniformBlock(const unsigned int& pShaderProgramHandle, const unsigned int& pUniformBlockIndex) const;
    GLData::UniformVariable getUniformVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pUniformVariableIndex) const;
    // Assign a binding point to a UniformBlock.
    // This makes the UniformBlock buffer-backed allowing UniformVariables belonging to the block to be set using setUniformBlockVariable.
    void RegisterUniformBlock(GLData::UniformBlock& pUniformBlock);

    int getShaderStorageBlockCount(const unsigned int& pShaderProgramHandle) const;
    GLData::ShaderStorageBlock getShaderStorageBlock(const unsigned int& pShaderProgramHandle, const unsigned int& pShaderBufferBlockIndex) const;
    GLData::ShaderStorageBlockVariable getShaderStorageBlockVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pShaderBufferBlockVariableIndex) const;
    // Assign a binding point to a ShaderStorageBlock
    // This makes the ShaderStorageBlock buffer-backed allowing variables belonging to the block to be set using setBufferBlockVariable.
    void RegisterShaderStorageBlock(GLData::ShaderStorageBlock& pShaderBufferBlock);

    void setUniform(const GLData::UniformVariable& pVariable, const bool& pValue)      const;
    void setUniform(const GLData::UniformVariable& pVariable, const int& pValue)       const;
    void setUniform(const GLData::UniformVariable& pVariable, const float& pValue)     const;
    void setUniform(const GLData::UniformVariable& pVariable, const glm::vec2& pValue) const;
    void setUniform(const GLData::UniformVariable& pVariable, const glm::vec3& pValue) const;
    void setUniform(const GLData::UniformVariable& pVariable, const glm::vec4& pValue) const;
    void setUniform(const GLData::UniformVariable& pVariable, const glm::mat2& pValue) const;
    void setUniform(const GLData::UniformVariable& pVariable, const glm::mat3& pValue) const;
    void setUniform(const GLData::UniformVariable& pVariable, const glm::mat4& pValue) const;

    // #GLHelperFunction
    template<class T>
    void setUniformBlockVariable(const std::string& pName, const T& pValue)
    {
        for (auto& bindingPoint : mUniformBlockBindingPoints)
        {
            const auto foundVariable = std::find_if(std::begin(bindingPoint.mVariables), std::end(bindingPoint.mVariables), [&pName](const GLData::UniformVariable& pVariable)
            {
                return pVariable.mName == pName;
            });

            if (foundVariable != std::end(bindingPoint.mVariables))
            {
                bindingPoint.mUBO.Bind(*this);
                setBlockUniform(*foundVariable, pValue);
                return;
            }
        }

        ZEPHYR_ASSERT(false, "No uniform block variable found with name '{}'", pName);
    }

    // #GLHelperFunction
    template<class T>
    void setShaderStorageBlockVariable(const std::string& pName, const T& pValue)
    {
        for (auto& bindingPoint : mShaderStorageBlockBindingPoints)
        {
            const auto foundVariable = std::find_if(std::begin(bindingPoint.mVariables), std::end(bindingPoint.mVariables), [&pName](const GLData::ShaderStorageBlockVariable& pVariable)
            {
                return pVariable.mName == pName;
            });

            if (foundVariable != std::end(bindingPoint.mVariables))
            {
                bindingPoint.mSSBO.Bind(*this);
                setShaderBlockVariable(*foundVariable, pValue);
                return;
            }
        }

        ZEPHYR_ASSERT(false, "No shader storage block variable found with name '{}'", pName)
    }
private:
	bool mDepthTest;
	GLType::DepthTestType mDepthTestType;

    bool mBlend;
    GLType::BlendFactorType mSourceBlendFactor;
    GLType::BlendFactorType mDestinationBlendFactor;

    bool mCullFaces;
    GLType::CullFacesType mCullFacesType;
    GLType::FrontFaceOrientation mFrontFaceOrientation;

    std::array<float, 4> mWindowClearColour;
    GLType::PolygonMode mPolygonMode;

    int mActiveTextureUnit;
    unsigned int mActiveFramebuffer;

    // Index data:
    // 0: Position X (0,0 = bottom-left)
    // 1: Position Y (0,0 = bottom-left)
    // 2: Size X
    // 3: Size Y
    std::array<int, 4> mViewport;

    // The remaining functions and members are helpers and not regular OpenGL parts
    struct UniformBlockBindingPoint
    {
        UniformBlockBindingPoint(GLState &pGLState) : mUBO(pGLState), mName(""), mInstances(0), mBindingPoint(0) {}

        GLData::UBO mUBO;       // The buffer for all the data used by all the UniformBlocks bound to this point.
        std::string mName = ""; // Name of the UniformBlock instances sharing this buffer.
        size_t mInstances;      // The number of UniformBlock instances using this buffer.
        unsigned int mBindingPoint; // The location of this binding point in the parent mUniformBlockBindingPoints vector.
        std::vector<GLData::UniformVariable> mVariables;  // Copy of all the UniformVariables this buffer... buffers.
    };
    std::vector<UniformBlockBindingPoint> mUniformBlockBindingPoints;

    struct ShaderStorageBlockBindingPoint
    {
        ShaderStorageBlockBindingPoint(GLState &pGLState) : mSSBO(pGLState), mName(""), mInstances(0), mBindingPoint(0) {}

        GLData::SSBO mSSBO;     // The buffer for all the data used by all the ShaderStorageBlock bound to this point.
        std::string mName;      // Name of the UniformBlock instances sharing this buffer.
        size_t mInstances;      // The number of UniformBlock instances using this buffer.
        unsigned int mBindingPoint; // The location of this binding point in the parent mShaderStorageBlockBindingPoints vector.
        std::vector<GLData::ShaderStorageBlockVariable> mVariables;  // Copy of all the UniformVariables this buffer... buffers.
    };
    std::vector<ShaderStorageBlockBindingPoint> mShaderStorageBlockBindingPoints;

    void setShaderBlockVariable(const GLData::ShaderStorageBlockVariable& pVariable, const float& pValue);
    void setShaderBlockVariable(const GLData::ShaderStorageBlockVariable& pVariable, const glm::vec2& pValue);
    void setShaderBlockVariable(const GLData::ShaderStorageBlockVariable& pVariable, const glm::vec3& pValue);
    void setShaderBlockVariable(const GLData::ShaderStorageBlockVariable& pVariable, const glm::vec4& pValue);
    void setShaderBlockVariable(const GLData::ShaderStorageBlockVariable& pVariable, const glm::mat4& pValue);

    void setBlockUniform(const GLData::UniformVariable& pVariable, const float& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec2& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec3& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec4& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::mat4& pValue);

    std::string getErrorMessage() const;
    std::string getErrorMessage(const GLType::Function& pCallingFunction) const;
};