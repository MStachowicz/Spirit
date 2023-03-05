#pragma once

#include "glm/fwd.hpp"

#include "Logger.hpp"

#include <string>
#include <array>
#include <vector>
#include <optional>
#include <memory>

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
        Unknown
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
    enum class GLSLVariableType
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
    enum class BufferUsage
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
    enum class ShaderProgramType
    {
        Vertex,
        Geometry,
        Fragment
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
        TransformFeedbackVarying
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
        TransformFeedbackBufferStride
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
        GreaterEqual
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
    };
    enum class CullFacesType
    {
        Back,          // Culls only the back faces (Default OpenGL setting).
        Front,         // Culls only the front faces.
        FrontAndBack  // Culls both the front and back faces.
    };
    enum class FrontFaceOrientation
    {
        Clockwise,          // Clockwise polygons are identified as front-facing.
        CounterClockwise,   // Counter-clockwise polygons are identified as front-facing (Default OpenGL setting).
    };
    // Polygon rasterization mode
    // Vertices are marked as boundary/non-boundary with an edge flag generated internally by OpenGL when it decomposes triangle stips and fans.
    enum class PolygonMode
    {
        Point, // Polygon vertices that are marked as the start of a boundary edge are drawn as points. Point attributes such as GL_POINT_SIZE and GL_POINT_SMOOTH control the rasterization of the points.
        Line,  // Boundary edges of the polygon are drawn as line segments. Line attributes such as GL_LINE_WIDTH and GL_LINE_SMOOTH control the rasterization of the lines.
        Fill  // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon. (Default OpenGL setting).
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
    };
    enum class FramebufferTarget
    {
        DrawFramebuffer,
        ReadFramebuffer,
        Framebuffer
    };
    enum class ErrorType
    {
        InvalidEnum,
        InvalidValue,
        InvalidOperation,
        InvalidFramebufferOperation,
        OutOfMemory,
        StackUnderflow,
        StackOverflow
    };
    enum class Function
    {
        Viewport,
        DrawElements,
        DrawArrays,
        DrawElementsInstanced,
        DrawArraysInstanced,
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
        BufferData,
        BufferSubData,
        BindBufferRange,
        UniformBlockBinding,
        ShaderStorageBlockBinding,
        CopyBufferSubData
    };

    std::string toString(const Function& pFunction);
    std::string toString(const ShaderProgramType& pShaderProgramType);
    std::string toString(const DataType &pDataType);
    std::string toString(const GLSLVariableType& pVariableType);
    std::string toString(const BufferType& pBufferType);
    std::string toString(const BufferUsage& pBufferUsage);
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
    int convert(const GLSLVariableType& pVariableType);
    int convert(const BufferType& pBufferType);
    int convert(const BufferUsage& pBufferUsage);
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
    // Stores an array of memory allocated using OpenGL context on the GPU.
    struct Buffer
    {
        void Bind(const GLState& pGLState) const;
        void Delete(const GLState& pGLState);
        void Reserve(GLState& pGLState, const size_t& pBytesToReserve);
        void PushData(GLState& pGLState, const std::vector<int>& pData);
        void PushData(GLState& pGLState, const std::vector<float>& pData);

        const GLType::BufferType mType;
        const GLType::BufferUsage mUsage;
        unsigned int getHandle() { return mHandle; };
        size_t GetReservedSize() const { return mReservedSize; };
    protected:
        // Protected Buffer constructor turning Buffer into a pure interface.
        Buffer(const GLState& pGLState, const GLType::BufferType& pType, const GLType::BufferUsage& pUsage);
        unsigned int mHandle; // The handle/name of this Buffer in OpenGL. This value can change when a Buffer is extended.
        size_t mReservedSize; // The number of Bytes this Buffer occupies in GPU memory.
        size_t mUsedSize; // The number of Bytes actually pushed to the GPU memory occupied by the Buffer.
    };
    // Uniform Buffer Object
    // A Buffer Object that is used to store uniform data for a shader program. They can be used to share
    // uniforms between different programs, as well as quickly change between sets of uniforms for the same program object.
    // Used to provide buffer-backed storage for uniforms.
    // https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object
    struct UBO : public Buffer
    {
        friend class GLState;

        void AssignBindingPoint(GLState& pGLState, const unsigned int& pBindingPoint);

       private:
        UBO(const GLState& pGLState, const GLType::BufferUsage& pUsage)
            : Buffer(pGLState, GLType::BufferType::UniformBuffer, pUsage) {}
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
        friend class GLState;

        void AssignBindingPoint(GLState& pGLState, const unsigned int& pBindingPoint);
        // Double the size of this SSBO in GPU memory.
        // This is a costly operation, where possible call a Buffer::Resize to the predicted size that will be required.
        // SSBOs are extended when a variable-sized-array ShaderStorageBlockVariable is being set and its offset index passes the end of the SSBO backing it.
        // All data set before the Extend is lost.
        void Extend(GLState& pGLState);
       private:
        SSBO(const GLState& pGLState, const GLType::BufferUsage& pUsage)
        : Buffer(pGLState, GLType::BufferType::ShaderStorageBuffer, pUsage) {}

        std::optional<unsigned int> mBindingPoint;
    };

    // Represents a variable in GLSL offering an interface to set to its data. A GLSLVariable can be found in multiple configurations:
    // A loose uniform belonging to a shader program without an interface block parent (mBlockIndex == -1).
    // A UniformBlockVariable defined inside a UniformBlock.
    // A ShaderStorageBlockVariable defined inside a ShaderStorageBlock interface.
    struct GLSLVariable
    {
        GLType::GLSLVariableType mVariableType;
        std::optional<GLType::DataType> mDataType;

        std::optional<std::string> mName;
        // Number of array elements. The size is in units of the type associated with the property mDataType.
        // For variables not corresponding to an array of basic types, the value is 0.
        std::optional<int> mArraySize;
        // The offset of the variable relative to the base of the buffer range holding its value (UBO and SSBO variables)
        std::optional<int> mOffset;
        // The index of the interface block containing the variable.
        // If the variable is not the member of an interface block, the value is -1.
        std::optional<int> mBlockIndex;
        // The stride between array elements.
        // For variables declared an array of basic types, the value is the difference, in basic machine units, between the offsets of consecutive elements in an array.
        // For variables not declared as an array of basic types, the value is 0.
        // For variables not backed by a buffer object, the value is -1, regardless of the variable type.
        std::optional<int> mArrayStride;
        // The stride between columns of a column-major matrix or rows of a row-major matrix.
        // For variables declared a single matrix or array of matrices, the value is the difference, in basic machine units, between the offsets of consecutive columns or rows in each matrix.
        // For variables not declared as a matrix or array of matrices, the value is 0.
        // For variables not backed by a buffer object, the value is -1, regardless of the variable type.
        std::optional<int> mMatrixStride;
        // Identifying whether a variable is a row-major matrix.
        // For variables backed by a buffer object, declared as a single matrix or array of matrices, and stored in row-major order, the value is 1.
        // For all other variables, the value is 0.
        std::optional<int> mIsRowMajor;

        virtual void Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex = 0)      = 0;
        virtual void Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex = 0)       = 0;
        virtual void Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex = 0)     = 0;
        virtual void Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex = 0) = 0;
        virtual void Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex = 0) = 0;
        virtual void Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex = 0) = 0;
        virtual void Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex = 0) = 0;
        virtual void Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex = 0) = 0;
        virtual void Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex = 0) = 0;

       protected:
        GLSLVariable(const GLType::GLSLVariableType& pType) : mVariableType(pType) {}
    };

    // A shader-program global variable.
    // Declared with the "uniform" storage qualifier in GLSL
    // Uniform variables don't change from one shader invocation to the next within a particular rendering call thus their value is uniform among all invocations. This makes them unlike shader stage inputs and outputs, which are often different for each invocation of a shader stage.
    // These act as parameters that the user of a shader program can pass to that program. Their values are stored in a program object.
    // https://www.khronos.org/opengl/wiki/Uniform_(GLSL)
    struct UniformVariable : public GLSLVariable
    {
        // The index of the atomic counter buffer containing the variable.
        // If the variable is not an atomic counter uniform, the value is -1.
        // Uniform Only
        std::optional<int> mAtomicCounterBufferIndex;
        // The assigned location for a uniform, input, output, or subroutine uniform variable.
        // For input, output, or uniform variables with locations specified by a layout qualifier, the specified location is used.
        // For vertex shader input or fragment shader output variables without a layout qualifier, the location assigned when a program is linked.
        // For all other input and output variables, the value is -1.
        // For uniforms in uniform blocks, the value is -1.
        // Uniform only
        std::optional<int> mLocation;

        UniformVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pVariableIndex);

        virtual void Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex = 0) override;
    };
    // A GLSL variable that belongs to a UniformBlock
    struct UniformBlockVariable : public GLSLVariable
    {
        UniformBlockVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pVariableIndex);
        UBO* mBufferBacking = nullptr;

        virtual void Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex = 0) override;
    };
    // A GLSL variable that belongs to a ShaderStorageBlock
    struct ShaderStorageBlockVariable: public GLSLVariable
    {
        // The number of array elements of the top-level shader storage block member containing to the variable.
        // If the top-level block member is not declared as an array, the value is 1.
        // If the top-level block member is an array with no declared size, the value is 0.
        // Buffer block only
        std::optional<int> mTopLevelArraySize;
        // The stride between array elements of the top-level shader storage block member containing the variable.
        // For top-level block members declared as arrays, the value is the difference, in basic machine units, between the offsets of the variable for consecutive elements in the top-level array.
        // For top-level block members not declared as an array, the value is 0.
        // Buffer block only
        std::optional<int> mTopLevelArrayStride;

        bool mIsVariableArray = false; // Is this variable declared as an Array with no size constraint.

        ShaderStorageBlockVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pVariableIndex);
        SSBO* mBufferBacking = nullptr;

        virtual void Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex = 0) override;
        virtual void Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex = 0) override;
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
        std::vector<UniformBlockVariable> mVariables;
        std::vector<int> mVariableIndices;
    };
    // Represents an OpenGLWide index for attatching UBO objects to so their memory can be shared across Shader instances.
    struct UniformBlockBindingPoint
    {
        UniformBlockBindingPoint(GLState& pGLState, UniformBlock& pUniformBlock, const unsigned int& pIndex);

        void BindUniformBlock(UniformBlock& pUniformBlock)
        {
            mInstances++;

            for (auto& variable : pUniformBlock.mVariables)
                variable.mBufferBacking = mUBO;
        }

        GLData::UBO* mUBO;           // The actual buffer all the GLSL::UniformBlocks bound to this index use.
        unsigned int mBindingPoint; // The location of this binding point in the parent mUniformBlockBindingPoints vector.
        std::string mName;          // Name of the GLSL::UniformBlock instances sharing this buffer.
        size_t mInstances;          // The number of UniformBlock instances using this buffer.
        std::vector<GLData::UniformBlockVariable> mVariables;  // Copy of the variables inside the UniformBlocks this BindingPoint backs.
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

        bool mShared = false; // Whether the block should be bound to a ShaderStorageBlockBindingPoint matching.
        std::optional<unsigned int> mBindingPoint; // The binding of the block to a corrresponding ShaderStorageBlockBindingPoint
        std::vector<ShaderStorageBlockVariable> mVariables;
        std::vector<int> mVariableIndices;
    };
    struct ShaderStorageBlockBindingPoint
    {
        ShaderStorageBlockBindingPoint(GLState& pGLState, ShaderStorageBlock& pStorageBlock, const unsigned int pIndex);

        void BindStorageBlock(ShaderStorageBlock& pStorageBlock)
        {
            mInstances++;

            for (auto& variable : pStorageBlock.mVariables)
                variable.mBufferBacking = mSSBO;
        }

        GLData::SSBO* mSSBO;     // The buffer for all the data used by all the ShaderStorageBlock bound to this point.
        unsigned int mBindingPoint; // The location of this binding point in the parent mShaderStorageBlockBindingPoints vector.
        std::string mName;      // Name of the ShaderStorageBlock instances sharing this buffer.
        size_t mInstances;      // The number of ShaderStorageBlock instances using this buffer.
        std::vector<GLData::ShaderStorageBlockVariable> mVariables;
    };

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
    struct VBO : public Buffer
	{
        VBO(const GLState& pGLState, const GLType::BufferUsage& pUsage)
            : Buffer(pGLState, GLType::BufferType::ArrayBuffer, pUsage) {}
        void PushVertexAttributeData(GLState& pGLState, const std::vector<float>& pData, const int& pAttributeIndex, const int& pAttributeSize);
	};
    // Element Buffer Object
    // Buffer storing vertex index data defining which order to draw vertex data stored in a VBO.
    // EBO's are used only if a mesh uses Indexed drawing.
	struct EBO : public Buffer
	{
        EBO(const GLState& pGLState, const GLType::BufferUsage& pUsage)
        : Buffer(pGLState, GLType::BufferType::ElementArrayBuffer, pUsage) {}
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
    GLState(const GLState&) = delete;

    // Checks if GLState matches what OpenGL state machine is set to.
    bool validateState();

    bool getDepthTest() const { return mDepthTest; };
	void toggleDepthTest(const bool& pDepthTest);

    GLType::DepthTestType getDepthTestType() const { return mDepthTestType; };
	void setDepthTestType(const GLType::DepthTestType& pType);

	// Specifies if objects with alpha values <1 should be blended using function set with setBlendFunction().
    void toggleBlending(const bool& pBlend);
    // Specifies how the RGBA factors of source and destination are blended to give the final pixel colour when encountering transparent objects.
    void setBlendFunction(const GLType::BlendFactorType& pSourceFactor, const GLType::BlendFactorType& pDestinationFactor);

    bool getCullFaces() const { return mCullFaces; };
    // Specifies if facets specified by setFrontFaceOrientation are candidates for culling.
    void toggleCullFaces(const bool& pCull);
    // Specifies which facets are candidates for culling.
    void setCullFacesType(const GLType::CullFacesType& pCullFaceType);
    // Specifies the orientation of front-facing polygons. Used to mark facets for culling.
    void setFrontFaceOrientation(const GLType::FrontFaceOrientation& pFrontFaceOrientation);

    // Specifies the red, green, blue, and alpha values to clear the color buffers. Values are clamped to the range 0-1.
    void setClearColour(const std::array<float, 4>& pColour);
    // Specifies the red, green, blue, and alpha values to clear the color buffers. Values are clamped to the range 0-1.
    void setClearColour(const glm::vec4& pColour);

    // Controls the interpretation of polygons for rasterization.
    // pPolygonMode: Specifies how polygons will be rasterized.
    // Affects only the final rasterization of polygons - a polygon's vertices are lit and the polygon is clipped/culled before these modes are applied.
    void setPolygonMode(const GLType::PolygonMode& pPolygonMode);

    // Selects active texture unit subsequent texture state calls will affect. The number of texture units an implementation supports is implementation dependent, but must be at least 80.
    void setActiveTextureUnit(const int& pUnitPosition);

    // Render primitives from array data.
    // drawArrays specifies multiple geometric primitives with very few subroutine calls. Instead of calling a GL procedure to pass each individual vertex, normal, texture coordinate, edge flag, or color, you can prespecify separate arrays of vertices, normals, and colors and use them to construct a sequence of primitives with a single call to glDrawArrays.
    // When drawArrays is called, it uses pArraySize sequential elements from each enabled array to construct a sequence of geometric primitives, beginning with element first. pPrimitiveMode specifies what kind of primitives are constructed and how the array elements construct those primitives.
    // Vertex attributes that are modified by drawArrays have an unspecified value after drawArrays returns. Attributes that aren't modified remain well defined.
    void drawArrays(const GLType::PrimitiveMode& pPrimitiveMode, const int& pArraySize);
    // Draw multiple instances of a range of elements
    // drawArraysInstanced behaves identically to drawArrays except that pInstanceCount instances of the range of elements are executed and the value of the internal counter instanceID advances for each iteration.
    // instanceID is an internal 32-bit integer counter that may be read by a vertex shader as gl_InstanceID.
    void drawArraysInstanced(const GLType::PrimitiveMode& pPrimitiveMode, const int& pArraySize, const int& pInstanceCount);
    // Render primitives from array data.
    // drawElements specifies multiple geometric primitives with very few subroutine calls. Instead of calling a GL function to pass each individual vertex, normal, texture coordinate, edge flag, or color, you can prespecify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
    // When drawElements is called, it uses pElementSize sequential elements from an enabled array, starting at indices to construct a sequence of geometric primitives. pPrimitiveMode specifies what kind of primitives are constructed and how the array elements construct these primitives. If more than one array is enabled, each is used.
    // Vertex attributes that are modified by drawElements have an unspecified value after drawElements returns. Attributes that aren't modified maintain their previous values.
    void drawElements(const GLType::PrimitiveMode& pPrimitiveMode, const int& pElementsSize);
    // Draw multiple instances of a set of elements.
    // drawElementsInstanced behaves identically to drawElements except that pInstanceCount of the set of elements are executed and the value of the internal counter instanceID advances for each iteration.
    // instanceID is an internal 32-bit integer counter that may be read by a vertex shader as gl_InstanceID.
    void drawElementsInstanced(const GLType::PrimitiveMode& pPrimitiveMode, const int& pElementsSize, const int& pInstanceCount);

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

    // Creates and initializes a buffer object's data store. The Buffer currently bound to target is used.
    // While creating the new storage, any pre-existing data store is deleted. The new data store is created with the specified size in bytes and usage.
    // If data is not NULL, the data store is initialized with data from this pointer. In its initial state, the new data store is not mapped, it has a NULL mapped pointer, and its mapped access is GL_READ_WRITE.
    // pBufferUsage is a hint to the GL implementation as to how a buffer object's data store will be accessed. This enables the OpenGL to make more intelligent decisions that may significantly impact buffer performance.
    // It does not, however, constrain the actual usage of the data store.
    // If pData is NULL, a data store of the specified size is still created, but its contents remain uninitialized and thus undefined.
    // Clients must align data elements consistently with the requirements of the client platform, with an additional base-level requirement:
    // an offset within a buffer to a datum comprising N bytes be a multiple of N.
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
    void BufferData(const GLType::BufferType& pBufferType, const size_t& pSizeInBytes, const void* pData, const GLType::BufferUsage& pBufferUsage);

    // Update a subset of a Buffer object's data store.
    // Data starting at byte pOffset offset and extending for size pSizeInBytes is copied to the data store from the memory pointed to by data.
    // pOffset and pSizeInBytes must define a range lying entirely within the buffer object's data store.
    void BufferSubData(const GLType::BufferType& pBufferType, const size_t& pOffset, const size_t& pSizeInBytes, const void* pData);

    // Copy all or part of the data store of a buffer object to the data store of another buffer object.
    // pSize is copied from the source buffer at pSourceOffset to the destination buffer at pDestinationOffset. pSourceOffset, pDestinationOffset and pSize are in terms of basic machine units.
    // Any of the BufferType targets may be used, but the targets CopyReadBuffer and CopyWriteBuffer are provided specifically to allow copies between buffers without disturbing other GL state.
    // If the source and destination are the same buffer object, then the source and destination ranges must not overlap.
    // OpenGL 4.6 - Replace with bindless
    void CopyBufferSubData(const GLType::BufferType& pSourceTarget, const GLType::BufferType& pDestinationTarget, const long long int& pSourceOffset, const long long int& pDestinationOffset, const long long int& pSize);

    // Bind a range within a buffer object to an indexed buffer target
    // BufferType must be one of UniformBuffer, ShaderStorageBuffer, AtomicCounterBuffer or TransformFeedbackBuffer
    // The range bound starts at pOffset
    void BindBufferRange(const GLType::BufferType& pType, const unsigned int& pBufferHandle, const unsigned int& pBindingPoint, const unsigned int& pOffset, const size_t& pBindSizeInBytes);

    void UniformBlockBinding(const unsigned int& pShaderHandle, const unsigned int& pUniformBlockIndexShader, const unsigned int& pBindingPoint);
    void ShaderStorageBlockBinding(const unsigned int& pShaderHandle, const unsigned int& pUniformBlockIndexShader, const unsigned int& pBindingPoint);

    int getActiveUniformCount(const unsigned int& pShaderProgramHandle) const;
    int getActiveUniformBlockCount(const unsigned int& pShaderProgramHandle) const;
    GLData::UniformBlock getUniformBlock(const unsigned int& pShaderProgramHandle, const unsigned int& pUniformBlockIndex) const;
    // Assign a binding point to a UniformBlock.
    // This makes the UniformBlock buffer-backed allowing UniformVariables belonging to the block to be set using setUniformBlockVariable.
    void RegisterUniformBlock(GLData::UniformBlock& pUniformBlock);

    int getShaderStorageBlockCount(const unsigned int& pShaderProgramHandle) const;
    GLData::ShaderStorageBlock getShaderStorageBlock(const unsigned int& pShaderProgramHandle, const unsigned int& pShaderBufferBlockIndex) const;
    // Assign a binding point to a ShaderStorageBlock
    // This makes the ShaderStorageBlock buffer-backed allowing variables belonging to the block to be set using setBufferBlockVariable.
    void RegisterShaderStorageBlock(GLData::ShaderStorageBlock& pShaderStorageBlock);

    // #GLHelperFunction
    template<class T>
    void setUniformBlockVariable(const std::string& pName, const T& pValue)
    {
       for (auto& bindingPoint : mUniformBlockBindingPoints)
       {
           const auto foundVariable = std::find_if(std::begin(bindingPoint.mVariables), std::end(bindingPoint.mVariables), [&pName](const GLData::UniformBlockVariable& pVariable)
           {
               return pVariable.mName == pName;
           });

           if (foundVariable != std::end(bindingPoint.mVariables))
           {
               ZEPHYR_ASSERT(bindingPoint.mUBO == (*foundVariable).mBufferBacking , "Variable buffer backing doesnt match the binding point assigned to the parent UniformBlock. Check the RegisterUniformBlock function.");
               ZEPHYR_ASSERT((*foundVariable).mBufferBacking != nullptr, "Setting a UniformBlockVariable with no Buffer backing.");
               bindingPoint.mUBO->Bind(*this);
               (*foundVariable).Set(*this, pValue);
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
               ZEPHYR_ASSERT(bindingPoint.mSSBO == (*foundVariable).mBufferBacking , "Variable buffer backing doesnt match the binding point assigned to the parent ShaderStorageBlock. Check the RegisterShaderStorageBlock function.");
               ZEPHYR_ASSERT((*foundVariable).mBufferBacking != nullptr, "Setting a ShaderStorageBlockVariable with no Buffer backing.");
               bindingPoint.mSSBO->Bind(*this);
               (*foundVariable).Set(*this, pValue);
               return;
           }
       }

       ZEPHYR_ASSERT(false, "No shader storage block variable found with name '{}'", pName)
    }

    // GLState acts as a factory for Buffers allowing single responsibility and access to memory that does not move.

    GLData::UBO* const CreateUBO(const GLType::BufferUsage& pBufferUsage)
    {
        mUBOs.emplace_back(new GLData::UBO(*this, pBufferUsage));
        return mUBOs.back().get();
    }
    GLData::SSBO* const CreateSSBO(const GLType::BufferUsage& pBufferUsage)
    {
        mSSBOs.emplace_back(new GLData::SSBO(*this, pBufferUsage));
        return mSSBOs.back().get();
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

    std::vector<GLData::UniformBlockBindingPoint> mUniformBlockBindingPoints;
    std::vector<GLData::ShaderStorageBlockBindingPoint> mShaderStorageBlockBindingPoints;


    // Buffers are referened in many areas of GLState and OpenGLRenderer, therefore they must exist in a permanent memory location to not break these references.
    std::vector<std::unique_ptr<GLData::UBO>> mUBOs;
    std::vector<std::unique_ptr<GLData::SSBO>> mSSBOs;

    std::optional<std::vector<std::string>> GetErrorMessagesOverride(const GLType::Function& pCallingFunction, const GLType::ErrorType& pErrorType) const;
    std::string getErrorMessage() const;
    std::string getErrorMessage(const GLType::Function& pCallingFunction) const;
};