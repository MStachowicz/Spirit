#pragma once

#include "Shader.hpp"

// STD
#include <optional>
#include <vector>

namespace Data
{
    class Texture;
    class Model;
    class CompositeMesh;
    class Mesh;
}

namespace OpenGL
{
    constexpr inline static bool LogGLTypeEvents = false;
    // A GLHandle is a pointer to memory owned by this OpenGL context on the GPU.
    // Classes that own a GLHandle require memory management as if having an owning pointer.
    using GLHandle = unsigned int;

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
        const GLHandle& getHandle() const { return mHandle; };

    private:
        GLHandle mHandle;
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
        GLHandle mHandle;
    };

    // Handle for an OpenGL VBO. Data can be pushed to the GPU by calling setData with the type of vertex attribute being pushed.
    class VBO
    {
    public:
        VBO() noexcept;
        ~VBO() noexcept;

        VBO(const VBO& pOther) = delete;
        VBO& operator=(const VBO& pOther) = delete;
        VBO(VBO&& pOther) noexcept;
        VBO& operator=(VBO&& pOther) noexcept;

        void bind() const;
        void setData(const std::vector<glm::vec3>& pVec3Data, const Shader::Attribute& pAttributeType);
        void setData(const std::vector<glm::vec2>& pVec2Data, const Shader::Attribute& pAttributeType);
        void clear();

        // Copy the contents of pSource into pDestination. Any data pDestination owned before is deleted.
        // Implemented as a static of VBO to only allow explicit copying.
        static void copy(const VBO& pSource, VBO& pDestination);
    private:
        GLHandle mHandle;
        size_t mSize; // Size in bytes of the data this VBO holds in GPU memory.
    };

    class Texture
    {
    public:
        Texture();
        ~Texture();

        Texture(const Texture& pOther) = delete;
        Texture& operator=(const Texture& pOther) = delete;
        Texture(Texture&& pOther) noexcept;
        Texture& operator=(Texture&& pOther) noexcept;

        Texture(const Data::Texture& pTextureData);

        void bind() const;
        const GLHandle& getHandle() const { return mHandle; };
    private:
        GLHandle mHandle;
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
        const GLHandle& getHandle() const { return mHandle; };

    private:
        GLHandle mHandle;
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
        void detachDepthBuffer();
        void resize(const int& pWidth, const int& pHeight);
        bool isComplete() const;

        static void unbind();
    private:
        GLHandle mHandle;

        std::optional<Texture> mColourAttachment;
        std::optional<RBO> mDepthAttachment;
        int mBufferClearBitField; // Bit field sent to OpenGL clear buffers before next draw.
    };



    class Mesh
    {
    public:
        Mesh() noexcept  = default;
        ~Mesh() noexcept = default;

        Mesh(const Mesh& pOther) = delete;
        Mesh& operator=(const Mesh& pOther) = delete;
        Mesh(Mesh&& pOther) noexcept;
        Mesh& operator=(Mesh&& pOther) noexcept;

        // Construct an OpenGL mesh from mesh data.
        Mesh(const Data::Mesh& pMeshData) noexcept;

        VAO mVAO;
        std::optional<VBO> mVertexPositions;
        std::optional<VBO> mVertexNormals;
        std::optional<VBO> mVertexTextureCoordinates;
        std::optional<EBO> mEBO;
        int mDrawSize; // Depending on if the mesh is indexed or array based, this is the number of
    };
}