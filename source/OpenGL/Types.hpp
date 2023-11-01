// Types uses GLState to define helper functions that encapsulate commone usage of GL functions.
// Types uses raw OpenGL functions in the cpp where we can trust the usage is correct.
// They are RAII wrappers handling the memory owned and stored by OpenGL.
#pragma once

#include "GLState.hpp"

#include "ResourceManager.hpp" // Used to implement GLSL UniformBlock and ShaderStorageBlock buffer-backing.

// STD
#include <optional>
#include <vector>

namespace Data
{
    class Model;
    class CompositeMesh;
    class Mesh;
}
namespace Utility
{
    class Image;
}

namespace OpenGL
{
    // These are per-vertex attributes found in Shader.
    // Each attribute must be named the same in Shader objects. get_attribute_identifier() returns the expected identifier used in shader.
    // Each attribute must be in the same location in all Shader instances, specified as "layout (location = X)"". get_attribute_index() returns the location expected.
    // VBO depends on VertexAttribute.
    enum class VertexAttribute : uint8_t
    {
        Position3D,
        Normal3D,
        ColourRGB,
        ColourRGBA,
        TextureCoordinate2D
    };

    // Namespace to hide implementation details and not pollute OpenGL namespace.
    namespace impl
    {
        // Defines a requirement for a Type T to have a list of VertexAttributes. Used by VBO to hint OpenGL how to read the data being buffered.
        template <typename T>
        concept Has_Shader_Attributes_Layout = requires(T x) {{ T::Attributes[0] } -> std::convertible_to<VertexAttribute>; };

        // Returns the number of components the specified attribute consists of.
        // E.g. "vec3" in GLSL shaders would return 3 as it's composed of 3 components (X, Y and Z)
        int get_attribute_component_count(VertexAttribute p_attribute);
        // Returns the location of a specified attribute type. All shaders repeat the same attribute layout positions.
        // Specified as "layout (location = X)" in GLSL shaders.
        int get_attribute_index(VertexAttribute p_attribute);
        // Returns the stride of the attribute i.e. the sizeof.
        int get_attribute_stride(VertexAttribute p_attribute);
        // The data type of each component in the attribute. e.g. vec3=GL_FLOAT, int=GL_INT
        ShaderDataType get_attribute_type(VertexAttribute p_attribute);
        // Returns the attribute as a string matching the naming used within GLSL shaders.
        // e.g. All vertex position attributes will use the identifier "VertexPosition"
        const char* get_attribute_identifier(VertexAttribute p_attribute);
    }

    // Vertex Array Object (VAO)
    // Stores all of the state needed to supply vertex data. VAO::bind() needs to be called before setting the state using VBO's and EBO's.
    // It stores the format of the vertex data as well as the Buffer Objects (see below) providing the vertex data arrays.
    // Note: If you change any of the data in the buffers referenced by an existing VAO (VBO/EBO), those changes will be seen by users of the VAO.
    class VAO
    {
    public:
        VAO() noexcept;
        ~VAO() noexcept;
        VAO(const VAO& pOther) = delete;
        VAO& operator=(const VAO& pOther) = delete;
        VAO(VAO&& pOther) noexcept;
        VAO& operator=(VAO&& pOther) noexcept;

        void bind() const;
        const GLHandle& getHandle() const { return m_handle; };

    private:
        GLHandle m_handle;
    };

    class EBO
    {
    public:
        EBO() noexcept;
        ~EBO() noexcept;

        EBO(const EBO& pOther) = delete;
        EBO& operator=(const EBO& pOther) = delete;
        EBO(EBO&& pOther) noexcept;
        EBO& operator=(EBO&& pOther) noexcept;

        void bind() const;
        void setData(const std::vector<int>& pIndexData) const;

    private:
        GLHandle m_handle;
    };

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHADER TYPES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Variables found in Shader objects and their UniformBlocks.
    // UniformBlockVariable provides data the Shader class uses to actually set the data.
    // Not all members are relevent in all situations. This is documented per type.
    class UniformBlockVariable
    {
        friend class Shader; // Shader requires access to set the data via the m_offset or the m_location.

        std::string m_name;    // The identifier used for the variable in the GLSL shader.
        ShaderDataType m_type;
        GLint m_offset;        // The byte offset relative to the base of the buffer range.
        GLint m_array_size;    // For array variables the number of active array elements. 0 if not an array.
        GLint m_array_stride;  // Byte different between consecutive elements in an array type. 0 if not an array.
        GLint m_matrix_stride; // Stride between columns of a column-major matrix or rows of a row-major matrix. For non-matrix or array of matrices, 0. For UniformBlock variables -1.
        GLint m_location;      // Location for variable. For variables defined with layout qualifier this is the specified location. For non-loose uniform -1.
        GLint m_is_row_major;  // whether an active variable is a row-major matrix. For non-matrix variables 0.

    public:

        // Extract information about the uniform or UniformBlock variable in p_parent_shader_program with index p_variable_index.
        //@param p_parent_shader_program Shader that owns this UniformVariable.
        //@param p_variable_index Index of the variable in the p_parent_shader_program.
        UniformBlockVariable(GLHandle p_parent_shader_program, GLuint p_variable_index) noexcept;
        constexpr bool operator==(const UniformBlockVariable& p_other) const = default;
    };

    // Uniform Buffer Object
    // A Buffer Object that is used to store uniform data for a shader program.
    // They can be used to share uniforms between different programs, as well as quickly change between sets of uniforms for the same program object.
    // Used to provide buffer-backed storage for UniformBlockVariables that are part of UniformBlocks.
    // https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object
    class UBO
    {
        // The available binding points for UniformBlocks and UBOs. False = unused index.
        // When UBOs are constructed they bind themselves to the first available index and resign themselevs on destruction.
        // UBO and binding points have a 1:1 relationship, whereas multipl UniformBlock objects can bind themselves to one binding point.
        // The size of the list can never be larger than GL_MAX_UNIFORM_BUFFER_BINDINGS.
        static inline std::vector<bool> s_binding_points = {};

        GLHandle m_handle;
        GLsizei m_size; // Size in bytes of the entire buffer.
    public:

        // Construct a buffer to back a UniformBlock of p_variables.
        // After construction the UBO has reserved p_size bytes and is ready to set_data.
        // The construction also sets an appropriately available m_uniform_binding_point which a UniformBlock can use to attatch itself to using uniform_block_binding.
        //@param p_uniform_block_name The name of the UniformBlock requesting backing. Used to match against other UBOs.
        //@param p_size Byte size of the buffer. i.e. a sum of the sizes of p_variables.
        //@param p_variables The list of variables that need backing.
        UBO(const std::string& p_uniform_block_name, GLsizei p_size, const std::vector<UniformBlockVariable>& p_variables) noexcept;
        ~UBO() noexcept;

        UBO(const UBO& pOther)            = delete;
        UBO& operator=(const UBO& pOther) = delete;
        UBO(UBO&& pOther)            noexcept;
        UBO& operator=(UBO&& pOther) noexcept;

        void bind() const;

        GLuint m_uniform_binding_point; // The assigned binding point for this UBO.
        std::vector<UniformBlockVariable> m_variables; // A copy of the variables this buffer is backing.
        std::string m_uniform_block_name; // Identifier of the UniformBlock or blocks that use this UBO as backing.
    };

    // UniformBlocks are GLSL interface blocks which group UniformBlockVariable's.
    // UniformBlock's are buffer-backed using uniform buffer objects UBO.
    // Blocks declared with the GLSL shared keyword can be used with any program that defines a block with the same elements in the same order.
    // Matching blocks in different shader stages will, when linked into the same program, be presented as a single interface block.
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Uniform_blocks
    class UniformBlock
    {
        friend class Shader; // Shader requires access to uniform_block_binding_points allowing it to expose the set_block_uniform API.

        std::string m_name;               // Identifier of the block in m_parent_shader_program.
        GLuint m_block_index;             // Index of the UniformBlock in its m_parent_shader_program.
        GLHandle m_parent_shader_program;

        std::vector<UniformBlockVariable> m_variables; // All the variables this block defines.
        std::optional<Utility::ResourceRef<UBO>> m_buffer_backing; // The UBO that backs the UniformBlockVariables. In UniformBlocks marked shared, the UBO is reused and can be set once.

        // Pool of UBO objects that can be used to back the UniformBlock.
        static inline Utility::ResourceManager<UBO> uniform_block_binding_points = {};
    public:
        // Process a UniformBlock which is part of p_shader_program at p_uniform_block_index.
        //@param p_shader_program Shader program that owns this UniformBlock.
        //@param p_uniform_block_index Index of the uniform block, not to be confused with buffer binding point.
        UniformBlock(GLHandle p_shader_program, GLuint p_uniform_block_index) noexcept;
    };

    // Variables found in Shader objects and their ShaderStorageBlocks.
    // Provides data the Shader class uses to actually set the data.
    class ShaderStorageBlockVariable
    {
    public:
        std::string m_identifier; // The identifier used for the variable in the GLSL shader.
        ShaderDataType m_type;
        GLint m_offset; // The byte offset relative to the base of the buffer range.

        // Array-only - number of active array elements. The size is in units of the type associated with the property m_type.
        // For active variables not corresponding to an array of basic types, the value is 0.
        GLint m_array_size;
        // Array-only - byte difference between consecutive elements in an array type.
        // For active variables not declared as an array of basic types, value is 0.
        // For active variables not backed by a buffer object regardless of the variable type, value is -1.
        GLint m_array_stride;
        // Matrix-only - stride between columns of a column-major matrix or rows of a row-major matrix.
        // For active variables not declared as a matrix or array of matrices, value is 0.
        // For active variables not backed by a buffer object, value is -1, regardless of the variable type.
        GLint m_matrix_stride;
        // Matrix-only - is it a row-major matrix.
        // For active variables backed by a buffer object, declared as a single matrix or array of matrices, and stored in row-major order, value is 1.
        // For all other active variables, value is 0.
        GLint m_is_row_major;
        // Number of active array elements of the top-level shader storage block member.
        // If the top-level block member is not an array, the value is 1.
        // If it is an array with no declared size, the value is 0, assignable size can be found calling array_size().
        GLint m_top_level_array_size;
        // Stride between array elements of the top-level shader storage block member.
        // For arrays, the value written is the difference, in basic machine units, between the offsets of the active variable for consecutive elements in the top-level array.
        // For top-level block members not declared as an array, value is 0.
        GLint m_top_level_array_stride;

        // Extract information about the ShaderStorageBlock variable in p_parent_shader_program with index p_variable_index.
        //@param p_parent_shader_program Shader that owns this ShaderStorageBlockVariable.
        //@param p_variable_index Index of the variable in the p_parent_shader_program.
        ShaderStorageBlockVariable(GLHandle p_parent_shader_program, GLuint p_variable_index) noexcept;
        constexpr bool operator==(const ShaderStorageBlockVariable& p_other) const = default;
    };

    // A Shader Storage block Object (ShaderStorageBlock)
    // SSBOs are a lot like Uniform Buffer Objects (UBO's). SSBOs are bound to ShaderStorageBlockBindingPoint's, just as UBO's are bound to UniformBlockBindingPoint's.
    // Compared to UBO's SSBOs:
    // Can be much larger. The spec guarantees that SSBOs can be up to 128MB. Most implementations will let you allocate a size up to the limit of GPU memory.
    // Are writable, even atomically. SSBOs reads and writes use incoherent memory accesses, so they need the appropriate barriers, just as Image Load Store operations.
    // Can have variable storage, up to whatever buffer range was bound for that particular buffer. This means that you can have an array of arbitrary length in an SSBO (at the end, rather).
    // The actual size of the array, based on the range of the buffer bound, can be queried at runtime in the shader using the length function on the unbounded array variable.
    // SSBO access will likely be slower than UBO access. At the very least, UBOs will be no slower than SSBOs.
    // https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
    class SSBO
    {
    public:
        // The available binding points for ShaderStorageBlocks and SSBOs. False = unused index.
        // When SSBOs are constructed they bind themselves to the first available index and resign themselves on destruction.
        // SSBO and binding points have a 1:1 relationship, whereas multiple ShaderStorageBlock objects can bind themselves to one binding point.
        // The size of the list can never be larger than GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS.
        static inline std::vector<bool> s_binding_points = {};

        GLHandle m_handle;
        GLsizei m_size; // Size in bytes of the entire buffer. The fixed portion of the SSBO cannot exceed GL_MAX_SHADER_STORAGE_BLOCK_SIZE.

        SSBO(const std::string& p_storage_block_identifier, GLsizei p_size, const std::vector<ShaderStorageBlockVariable>& p_variables) noexcept;
        ~SSBO() noexcept;

        SSBO(const SSBO& p_other)            = delete;
        SSBO& operator=(const SSBO& p_other) = delete;
        SSBO(SSBO&& p_other)            noexcept;
        SSBO& operator=(SSBO&& p_other) noexcept;

        void bind() const;

        GLuint m_binding_point; // The assigned binding point for this SSBO.
        std::vector<ShaderStorageBlockVariable> m_variables; // A copy of the variables this buffer is backing.
        std::string m_identifier; // Identifier of the ShaderStorageBlock(s) that use this SSBO as backing.
    };

    // ShaderStorageBlock are GLSL interface blocks which group ShaderStorageBlockVariable's.
    // ShaderStorageBlock's are buffer-backed using Shader Storage Block Objects objects SSBO.
    // Blocks declared with the GLSL shared keyword can be used with any program that defines a block with the same elements in the same order.
    // Matching blocks in different shader stages will, when linked into the same program, be presented as a single interface block.
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Shader_storage_blocks
    class ShaderStorageBlock
    {
        friend class Shader; // Shader requires access to shader_storage_block_binding_points allowing it to expose the set_block_uniform API.

        std::string m_identifier; // Identifier of the block in m_parent_shader_program.
        GLuint m_index;           // Index of the StorageBlock in its m_parent_shader_program.
        GLHandle m_parent_shader_program;

        std::vector<ShaderStorageBlockVariable> m_variables;        // All the variables this block defines.
        std::optional<Utility::ResourceRef<SSBO>> m_buffer_backing; // The SSBO that backs the ShaderStorageBlockVariable. In ShaderStorageBlock marked shared, the SSBO is reused and can be set once.

        // Pool of SSBO objects that can be used to back the StorageBlock.
        static inline Utility::ResourceManager<SSBO> s_shader_storage_block_binding_points = {};
    public:
        // Process a ShaderStorageBlock which is part of p_shader_program at p_shader_storage_block_index.
        //@param p_shader_program Shader program that owns this StorageBlock.
        //@param p_shader_storage_block_index Index of the shader storage block, not to be confused with buffer binding point.
        ShaderStorageBlock(GLHandle p_shader_program, GLuint p_shader_storage_block_index) noexcept;

        ~ShaderStorageBlock()                                       = default;
        ShaderStorageBlock(ShaderStorageBlock&& p_other)            = default;
        ShaderStorageBlock& operator=(ShaderStorageBlock&& p_other) = default;

        ShaderStorageBlock(const ShaderStorageBlock& p_other)            = delete;
        ShaderStorageBlock& operator=(const ShaderStorageBlock& p_other) = delete;
    };

    // Handle for an OpenGL VBO. Data can be pushed to the GPU by calling setData with the type of vertex attribute being pushed.
    class VBO
    {
    public:
        VBO() noexcept;
        ~VBO() noexcept;

        VBO(const VBO& pOther)            = delete;
        VBO& operator=(const VBO& pOther) = delete;
        VBO(VBO&& pOther) noexcept;
        VBO& operator=(VBO&& pOther) noexcept;

        void bind() const;

        // Push a vector of Data type to the GPU.
        // The Data type being pushed is required to have a member array "Attributes".
        // Attributes is a list of Shader::Attributes in the same order they appear in the Data class.
        template <typename Data>
        requires impl::Has_Shader_Attributes_Layout<Data>
        void set_data(const std::vector<Data>& p_data)
        {
            { // Setup the stride and size for the buffer itself.
                m_stride = 0;
                m_size   = 0;
                for (const VertexAttribute& attrib : Data::Attributes)
                {
                    m_stride += impl::get_attribute_stride(attrib);
                }
                m_size = m_stride * p_data.size();
            }

            size_t running_offset = 0;
            for (const VertexAttribute& attrib : Data::Attributes)
            {
                const auto index           = impl::get_attribute_index(attrib);
                const auto component_count = impl::get_attribute_component_count(attrib);
                const auto type            = impl::get_attribute_type(attrib);

                enable_vertex_attrib_array(index);
                vertex_attrib_pointer(index, component_count, type, false, m_stride, (void*)running_offset);
                running_offset += impl::get_attribute_stride(attrib);
            }
            buffer_data(BufferType::ArrayBuffer, m_size, p_data.data(), BufferUsage::StaticDraw);
        }
        void setData(const std::vector<glm::vec3>& pVec3Data, const VertexAttribute& pAttributeType);
        void setData(const std::vector<glm::vec2>& pVec2Data, const VertexAttribute& pAttributeType);
        void clear();

        // Copy the contents of pSource into pDestination. Any data pDestination owned before is deleted.
        // Implemented as a static of VBO to only allow explicit copying.
        static void copy(const VBO& pSource, VBO& pDestination);

    private:
        GLHandle m_handle;
        size_t m_size;   // Size in bytes of the buffer. i.e. the amount of GPU memory the whole buffer holds.
        size_t m_stride; // Number of bytes from the start of one element to the next i.e. size of one element.
    };

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHADER TYPES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Texture
    {
    public:
        Texture();
        Texture(const Utility::Image& p_image);
        ~Texture();

        Texture(const Texture& pOther) = delete;
        Texture& operator=(const Texture& pOther) = delete;
        Texture(Texture&& pOther) noexcept;
        Texture& operator=(Texture&& pOther) noexcept;

        void bind() const;
        const GLHandle& getHandle() const { return m_handle; };
    private:
        GLHandle m_handle;
    };

    // Render Buffer Object
    // RBO's contain images optimized for use as render targets, and are the logical choice when you do not need to sample (i.e. in a post-pass shader) from the produced image.
    // If you need to resample (such as when reading depth back in a second shader pass), use Texture instead.
    // RBO's are created and used specifically with Framebuffer Objects (FBO's).
    class RBO
    {
    public:
        RBO()  noexcept;
        ~RBO() noexcept;

        RBO(const RBO& pOther)            = delete;
        RBO& operator=(const RBO& pOther) = delete;
        RBO(RBO&& pOther)
        noexcept;
        RBO& operator=(RBO&& pOther) noexcept;

        void bind() const;
        const GLHandle& getHandle() const { return m_handle; };

    private:
        GLHandle m_handle;
    };

    // Framebuffer object.
    // Allows creation of user-defined framebuffers that can be rendered to without disturbing the main screen.
    class FBO
    {
    public:
        FBO()  noexcept;
        ~FBO() noexcept;

        FBO(const FBO& pOther)            = delete;
        FBO& operator=(const FBO& pOther) = delete;
        FBO(FBO&& pOther) noexcept;
        FBO& operator=(FBO&& pOther) noexcept;

        void bind() const;
        void clearBuffers() const;
        void bindColourTexture() const;
        void attachColourBuffer(const int& pWidth, const int& pHeight);
        void detachColourBuffer();
        void attachDepthBuffer(const int& pWidth, const int& pHeight);
        void attach_depth_buffer(const glm::uvec2& p_resolution);
        void detachDepthBuffer();
        void resize(const int& pWidth, const int& pHeight);
        bool isComplete() const;

        static void unbind();

        std::optional<Texture> mColourAttachment;
        std::optional<Texture> m_depth_map;
        std::optional<RBO> mDepthAttachment;
        int mBufferClearBitField; // Bit field sent to OpenGL clear buffers before next draw.

    private:
        GLHandle m_handle;
    };

    class Mesh
    {
    public:
        Mesh()  noexcept = default;
        ~Mesh() noexcept = default;

        Mesh(const Mesh& pOther)            = delete;
        Mesh& operator=(const Mesh& pOther) = delete;
        Mesh(Mesh&& pOther)                 noexcept;
        Mesh& operator=(Mesh&& pOther)      noexcept;

        // Construct an OpenGL mesh from mesh data.
        Mesh(const Data::Mesh& pMeshData) noexcept;
        void draw() const;

        VAO mVAO;
        std::optional<VBO> mVertexPositions;
        std::optional<VBO> mVertexNormals;
        std::optional<VBO> mVertexTextureCoordinates;
        std::optional<EBO> mEBO;
        GLsizei mDrawSize; // Depending on if the mesh is indexed or array based, this is the element size or array size parameter to glDraw.
    };
}