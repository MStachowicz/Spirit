#pragma once

#include "Utility.hpp"
#include "glm/fwd.hpp"

#include "string"
#include "array"
#include "unordered_map"
#include "optional"

// Wraps all the GL types into enums and provides helper functions to extract the values or string representations.
// All the enums come with a matching array to allow iterating over the enums in ImGui and converting to string by O(1) indexing.
namespace GLType
{
    enum BufferDrawType
    {
        Colour,
        Depth,
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::BufferDrawType::Count)> bufferDrawTypes{
        "Colour",
        "Depth"};

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
    static inline const std::array<std::string, util::toIndex(GLType::DataType::Count)> dataTypes{
        "Float", "vec2", "vec3", "vec4",
        "Double", "DVec2", "DVec3", "DVec4",
        "Int", "IVec2", "IVec3", "IVec4",
        "UnsignedInt", "UVec2", "UVec3", "UVec4",
        "Bool", "BVec2", "BVec3", "BVec4",
        "Mat2", "Mat3", "Mat4",
        "Mat2x3", "Mat2x4", "Mat3x2", "Mat3x4", "Mat4x2", "Mat4x3",
        "Dmat2", "Dmat3", "Dmat4",
        "Dmat2x3", "Dmat2x4", "Dmat3x2", "Dmat3x4", "Dmat4x2", "Dmat4x3",
        "Sampler1D", "Sampler2D", "Sampler3D",
        "SamplerCube",
        "Sampler1DShadow", "Sampler2DShadow",
        "Sampler1DArray", "Sampler2DArray",
        "Sampler1DArrayShadow", "Sampler2DArrayShadow",
        "Sampler2DMS", "Sampler2DMSArray",
        "SamplerCubeShadow",
        "SamplerBuffer",
        "Sampler2DRect",
        "Sampler2DRectShadow",
        "Isampler1D", "Isampler2D", "Isampler3D",
        "IsamplerCube",
        "Isampler1DArray", "Isampler2DArray",
        "Isampler2DMS",
        "Isampler2DMSArray",
        "IsamplerBuffer",
        "Isampler2DRect",
        "Usampler1D", "Usampler2D", "Usampler3D",
        "UsamplerCube",
        "Usampler2DArray",
        "Usampler2DMS",
        "Usampler2DMSArray",
        "UsamplerBuffer",
        "Usampler2DRect"
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

    static inline const std::array<std::string, util::toIndex(GLType::ShaderResourceType::Count)> shaderResourceTypes{
        "Uniform",
        "UniformBlock",
        "ShaderStorageBlock",
        "BufferVariable",
        "Buffer",
        "ProgramInput",
        "ProgramOutput",
        "AtomicCounterBuffer",
        //"AtomicCounterShader",
        "VertexSubroutineUniform",
        "FragmentSubroutineUniform",
        "GeometrySubroutineUniform",
        "ComputeSubroutineUniform",
        "TessControlSubroutineUniform",
        "TessEvaluationSubroutineUniform",
        "TransformFeedbackBuffer",
        "TransformFeedbackVarying"};

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
    static inline const std::array<std::string, util::toIndex(GLType::ShaderResourceProperty::Count)> shaderResourceProperties{
        "NameLength",
        "Type",
        "ArraySize",
        "Offset",
        "BlockIndex",
        "ArrayStride",
        "MatrixStride",
        "IsRowMajor",
        "AtomicCounterBufferIndex",
        "TextureBuffer",
        "BufferBinding",
        "BufferDataSize",
        "NumActiveVariables",
        "ActiveVariables",
        "ReferencedByVertexShader",
        "ReferencedByTessControlShader",
        "ReferencedByTessEvaluationShader",
        "ReferencedByGeometryShader",
        "ReferencedByFragmentShader",
        "ReferencedByComputeShader",
        "NumCompatibleSubroutines",
        "CompatibleSubroutines",
        "TopLevelArraySize",
        "TopLevelArrayStride",
        "Location",
        "LocationIndex",
        "IsPerPatch",
        "LocationComponent",
        "TransformFeedbackBufferIndex",
        "TransformFeedbackBufferStride"};

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
    static inline const std::array<std::string, util::toIndex(GLType::DepthTestType::Count)> depthTestTypes{
        "Always",
        "Never",
        "Less",
        "Equal",
        "Not equal",
        "Greater than",
        "Less than or equal",
        "Greater than or equal"};

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
    static inline const std::array<std::string, util::toIndex(GLType::BlendFactorType::Count)> blendFactorTypes{{
        "Zero",
        "One",
        "Source Colour",
        "One Minus Source Colour",
        "Destination Colour",
        "One Minus Destination Colour",
        "Source Alpha",
        "One Minus Source Alpha",
        "Destination Alpha",
        "One Minus Destination Alpha",
        "Constant Colour",
        "One Minus Constant Colour",
        "Constant Alpha",
        "One Minus Constant Alpha"}};

    enum class CullFacesType
    {
        Back,          // Culls only the back faces (Default OpenGL setting).
        Front,         // Culls only the front faces.
        FrontAndBack,  // Culls both the front and back faces.
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::CullFacesType::Count)> cullFaceTypes {
        "Back",
        "Front",
        "Front and Back"};

    enum class FrontFaceOrientation
    {
        Clockwise,          // Clockwise polygons are identified as front-facing.
        CounterClockwise,   // Counter-clockwise polygons are identified as front-facing (Default OpenGL setting).
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::FrontFaceOrientation::Count)> frontFaceOrientationTypes {
        "Clockwise",
        "CounterClockwise"};

    // Polygon rasterization mode
    // Vertices are marked as boundary/non-boundary with an edge flag generated internally by OpenGL when it decomposes triangle stips and fans.
    enum class PolygonMode
    {
        Point, // Polygon vertices that are marked as the start of a boundary edge are drawn as points. Point attributes such as GL_POINT_SIZE and GL_POINT_SMOOTH control the rasterization of the points.
        Line,  // Boundary edges of the polygon are drawn as line segments. Line attributes such as GL_LINE_WIDTH and GL_LINE_SMOOTH control the rasterization of the lines.
        Fill,  // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon. (Default OpenGL setting).
        Count
    };
    static inline const std::array<std::string, util::toIndex(PolygonMode::Count)> polygonModeTypes{
        "Point",
        "Line",
        "Fill"};

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
    static inline const std::array<std::string, util::toIndex(PrimitiveMode::Count)> primitiveModeTypes{
        "Points"
        "LineStrip"
        "LineLoop"
        "Lines"
        "LineStripAdjacency"
        "LinesAdjacency"
        "TriangleStrip"
        "TriangleFan"
        "Triangles"
        "TriangleStripAdjacency"
        "TrianglesAdjacency"
        "Patches"};

    enum class FramebufferTarget
    {
        DrawFramebuffer,
        ReadFramebuffer,
        Framebuffer,
        Count
    };
    static inline const std::array<std::string, util::toIndex(FramebufferTarget::Count)> FrameBufferTargetTypes{
        "DrawFramebuffer",
        "ReadFramebuffer",
        "Framebuffer"};

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

    // Messages from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetError.xhtml
    static inline const std::array<std::string, util::toIndex(ErrorType::Count)> ErrorMessages{
        "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag",
        "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag",
        "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag",
        "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag",
        "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded",
        "GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow",
        "GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow"
    };

    inline std::string toString(const DataType& pDataType)                              { return dataTypes[util::toIndex(pDataType)]; }
    inline std::string toString(const ShaderResourceType& pResourceType)                { return shaderResourceTypes[util::toIndex(pResourceType)]; }
    inline std::string toString(const ShaderResourceProperty& pShaderResourceProperty)  { return shaderResourceProperties[util::toIndex(pShaderResourceProperty)]; }
    inline std::string toString(const DepthTestType& pDepthTestType)                    { return depthTestTypes[util::toIndex(pDepthTestType)]; }
    inline std::string toString(const BufferDrawType& pBufferDrawType)                  { return bufferDrawTypes[util::toIndex(pBufferDrawType)]; }
    inline std::string toString(const BlendFactorType& pBlendFactorType)                { return blendFactorTypes[util::toIndex(pBlendFactorType)]; }
    inline std::string toString(const CullFacesType& pCullFacesType)                    { return cullFaceTypes[util::toIndex(pCullFacesType)]; }
    inline std::string toString(const FrontFaceOrientation& pFrontFaceOrientation)      { return frontFaceOrientationTypes[util::toIndex(pFrontFaceOrientation)]; }
    inline std::string toString(const PolygonMode& pPolygonMode)                        { return polygonModeTypes[util::toIndex(pPolygonMode)]; }
    inline std::string toString(const PrimitiveMode& pPrimitiveMode)                    { return primitiveModeTypes[util::toIndex(pPrimitiveMode)]; }
    inline std::string toString(const FramebufferTarget& pFramebufferTarget)            { return FrameBufferTargetTypes[util::toIndex(pFramebufferTarget)]; }
    inline std::string toString(const ErrorType& pErrorType)                            { return ErrorMessages[util::toIndex(pErrorType)]; }

    DataType convert(const int& pDataType);

    int convert(const DataType& pDataType);
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
    // Vertex Buffer Object
    // Buffer storing per-vertex array data.
    struct VBO
	{
		void generate();
		void bind() const;
        void pushData(const std::vector<float>& pData, const int& attributeIndex, const int& attributeSize);
		void release();
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
    // Element Buffer Object
    // Buffer storing vertex index data defining which order to draw vertex data stored in a VBO.
    // EBO's are used only if a mesh uses Indexed drawing.
	struct EBO
	{
		void generate();
		void bind() const;
		void release();
        void pushData(const std::vector<int>& pIndexData);
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
    // Uniform Buffer Object
    // A Buffer Object that is used to store uniform data for a shader program. They can be used to share
    // uniforms between different programs, as well as quickly change between sets of uniforms for the same program object.
    // Used to provide buffer-backed storage for uniforms.
    // https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object
    struct UBO
    {
		void generate();
		void bind() const;
		void release();
        void pushData(const int& pSize, const int& pUniformIndex);
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
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
		unsigned int getHandle() { return mHandle; };
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
        int mSize              = -1;
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

    // Assign a binding point to a uniform block.
    // This makes the Uniform block buffer-backed allowing UniformVariables belonging to the block to be set using setUniformBlockVariable.
    void bindUniformBlock(GLData::UniformBlock& pUniformBlock);

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
                bindingPoint.mUBO.bind();
                setBlockUniform(*foundVariable, pValue);
                return;
            }
        }

        ZEPHYR_ASSERT(false, "No uniform block variable found with name '{}'", pName)
    }

    // Set the global viewport
    // Specify the width and height of the viewport. When a context is first attached to a window, width and height are set to the dimensions of that window.
    // The viewport specifies the affine transformation of x and y from normalized device coordinates to window coordinates.
    // Let x nd y nd be normalized device coordinates. Then the window coordinates x w y w are computed as follows:
    // x w = x nd + 1 ⁢ width 2 + x
    // y w = y nd + 1 ⁢ height 2 + y
    void setViewport(const int& pWidth, const int& pHeight);

    // Outputs the current GLState with options to change flags.
    void renderImGui();

    static int getActiveUniformBlockCount(const unsigned int& pShaderHandle);
    static GLData::UniformBlock getUniformBlock(const unsigned int& pShaderHandle, const unsigned int& pUniformBlockIndex);
    static GLData::UniformVariable getUniformVariable(const unsigned int& pShaderHandle, const unsigned int& pUniformVariableIndex);
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
    // Use this to replace the Shader::bindingpoint
    struct UniformBlockBindingPoint
    {
        GLData::UBO mUBO;       // The buffer for all the data used by all the UniformBlocks bound to this point.
        std::string mName = ""; // Name of the UniformBlock instances sharing this buffer.
        size_t mInstances;      // The number of UniformBlock instances using this buffer.
        unsigned int mBindingPoint; // The location of this binding point in the parent mUniformBlockBindingPoints vector.
        std::vector<GLData::UniformVariable> mVariables;  // Copy of all the UniformVariables this buffer... buffers.
    };
    std::vector<UniformBlockBindingPoint> mUniformBlockBindingPoints;
    void setBlockUniform(const GLData::UniformVariable& pVariable, const float& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec2& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec3& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec4& pValue);
    void setBlockUniform(const GLData::UniformVariable& pVariable, const glm::mat4& pValue);

    std::string getErrorMessage();
    std::string getErrorMessage(const std::unordered_map<GLType::ErrorType, std::string>& pErrorMessageOverrides);
};