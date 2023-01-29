#include "Types.hpp"

// COMPONENT
#include "Mesh.hpp"
#include "Texture.hpp"

// UTILITY
#include "Logger.hpp"

// OPENGL
#include "glad/gl.h"


namespace OpenGL
{
    VAO::VAO() noexcept
        : mHandle{0}
    {
        glGenVertexArrays(1, &mHandle);
        if constexpr (LogGLTypeEvents) LOG_INFO("VAO constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    VAO::~VAO() noexcept
    {
        // We initialise mHandle to 0 and reset to 0 on move and depend on this to check if there is a vertex array to delete here.
        // We still want to call glDeleteVertexArrays if the array was generated but not used with bind and/or VBOs hence
        // not using glIsVertexArray here.
        if (mHandle != 0)
            glDeleteVertexArrays(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("VAO destroyed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    VAO::VAO(VAO&& pOther) noexcept
        : mHandle{std::move(pOther.mHandle)}
    { // Steal the handle of the other VAO and set it to 0 to prevent the pOther destructor calling glDeleteVertexArrays
        pOther.mHandle = 0;

        if constexpr (LogGLTypeEvents) LOG_INFO("VAO move-constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    VAO& VAO::operator=(VAO&& pOther) noexcept
    {
        if (this != &pOther)
        {
            // Free the existing resource.
            if (mHandle != 0)
                glDeleteVertexArrays(1, &mHandle);

            // Copy the data pointer from the source object.
            mHandle = pOther.mHandle;
            // Release the handle so ~VAO doesnt call glDeleteVertexArrays on mHandle.
            pOther.mHandle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("VAO move-assigned with GLHandle {} at address {}", mHandle, (void*)(this));
        return *this;
    }
    void VAO::bind() const
    {
        glBindVertexArray(mHandle);
    }



    EBO::EBO() noexcept
        : mHandle{0}
    {
        glGenBuffers(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("EBO constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    EBO::~EBO() noexcept
    {
        if (mHandle != 0)
            glDeleteBuffers(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("EBO destroyed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    EBO::EBO(EBO&& pOther) noexcept
        : mHandle{std::move(pOther.mHandle)}
    {
        pOther.mHandle = 0;

        if constexpr (LogGLTypeEvents) LOG_INFO("EBO move-constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    EBO& EBO::operator=(EBO&& pOther) noexcept
    {
        if (this != &pOther)
        {
            // Free the existing resource.
            if (mHandle != 0)
                glDeleteBuffers(1, &mHandle);

            // Copy the data pointer from the source object.
            mHandle = pOther.mHandle;
            // Release the handle so ~EBO doesnt call glDeleteBuffers on mHandle.
            pOther.mHandle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("EBO move-assigned with GLHandle {} at address {}", mHandle, (void*)(this));
        return *this;
    }
    void EBO::bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
    }
    void EBO::setData(const std::vector<int>& pIndexData) const
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pIndexData.size() * sizeof(int), &pIndexData.front(), GL_STATIC_DRAW);
    }



    VBO::VBO() noexcept
    : mHandle{0}
    , mSize{0}
    {
        glGenBuffers(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("VBO created with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    VBO::~VBO() noexcept
    {
        if (mHandle != 0)
            glDeleteBuffers(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("VBO destroyed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    VBO::VBO(VBO&& pOther) noexcept
        : mHandle{std::move(pOther.mHandle)}
        , mSize{std::move(pOther.mSize)}
    {
        pOther.mHandle = 0;
        pOther.mSize = 0;

        if constexpr (LogGLTypeEvents) LOG_INFO("VBO move-constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    VBO& VBO::operator=(VBO&& pOther) noexcept
    {
        if (this != &pOther)
        {
            // Free the existing resource.
            if (mHandle != 0)
                glDeleteBuffers(1, &mHandle);

            // Copy the data pointer from the source object.
            mHandle = pOther.mHandle;
            mSize = pOther.mSize;
            // Release the handle so ~VBO doesnt call glDeleteBuffers on mHandle.
            pOther.mHandle = 0;
            pOther.mSize = 0;
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("VBO move-assigned with GLHandle {} at address {}", mHandle, (void*)(this));
        return *this;
    }
    void VBO::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    }
    void VBO::setData(const std::vector<glm::vec3>& pVec3Data, const Shader::Attribute& pAttributeType)
    {
        mSize = pVec3Data.size() * sizeof(glm::vec3);
        glBufferData(GL_ARRAY_BUFFER, mSize, &pVec3Data.front(), GL_STATIC_DRAW);

        auto index = Shader::getAttributeLocation(pAttributeType);
        auto count = Shader::getAttributeComponentCount(pAttributeType);
        glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(index);
    }
    void VBO::setData(const std::vector<glm::vec2>& pVec2Data, const Shader::Attribute& pAttributeType)
    {
        mSize = pVec2Data.size() * sizeof(glm::vec2);
        glBufferData(GL_ARRAY_BUFFER, mSize, &pVec2Data.front(), GL_STATIC_DRAW);

        auto index = Shader::getAttributeLocation(pAttributeType);
        auto count = Shader::getAttributeComponentCount(pAttributeType);
        glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(index);
    }
    void VBO::clear()
    {
        bind();
        mSize = 0;
        glBufferData(GL_ARRAY_BUFFER, mSize, NULL, GL_STATIC_DRAW);
    }
    void VBO::copy(const VBO& pSource, VBO& pDestination)
    {
        glBindBuffer(GL_COPY_WRITE_BUFFER, pDestination.mHandle);
        // glBufferData deletes pre-existing data, we additionally call with pData as NULL which
        // gives us a buffer of pSource.mSize uninitialised.
        glBufferData(GL_COPY_WRITE_BUFFER, pSource.mSize, NULL, GL_STREAM_COPY);

        glBindBuffer(GL_COPY_READ_BUFFER, pSource.mHandle);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, pSource.mSize);

        pDestination.mSize = pSource.mSize;
    }




    Texture::~Texture()
    {
        if (mHandle != 0)
            glDeleteTextures(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("Texture destroyed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    Texture::Texture()
        : mHandle{0}
    {
        glGenTextures(1, &mHandle);
        if constexpr (LogGLTypeEvents) LOG_INFO("Texture constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    Texture::Texture(Texture&& pOther) noexcept
        : mHandle{std::move(pOther.mHandle)}
    {
        pOther.mHandle = 0;

        if constexpr (LogGLTypeEvents) LOG_INFO("Texture move-constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    Texture& Texture::operator=(Texture&& pOther) noexcept
    {
        if (this != &pOther)
        {
            // Free the existing resource.
            if (mHandle != 0)
                glDeleteTextures(1, &mHandle);

            // Copy the data pointer from the source object.
            mHandle = pOther.mHandle;
            // Release the handle so ~Texture doesnt call glDeleteBuffers on mHandle.
            pOther.mHandle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("Texture move-assigned with GLHandle {} at address {}", mHandle, (void*)(this));
        return *this;
    }
    Texture::Texture(const Data::Texture& pTextureData)
        : mHandle{0}
    {
        glGenTextures(1, &mHandle);
        glBindTexture(GL_TEXTURE_2D, mHandle);

        GLenum format = 0;
        if (pTextureData.mNumberOfChannels == 1)      format = GL_RED;
        else if (pTextureData.mNumberOfChannels == 3) format = GL_RGB;
        else if (pTextureData.mNumberOfChannels == 4) format = GL_RGBA;
        ZEPHYR_ASSERT(format != 0, "Could not find channel type for this number of texture channels");

        glTexImage2D(GL_TEXTURE_2D, 0, format, pTextureData.mWidth, pTextureData.mHeight, 0, format, GL_UNSIGNED_BYTE, pTextureData.getData());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // GL_REPEAT - (default wrapping method)
        // GL_CLAMP_TO_EDGE - when using transparency to stop interpolation at borders causing semi-transparent artifacts.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);

        if constexpr (LogGLTypeEvents) LOG_INFO("Texture constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    void Texture::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, mHandle);
    };

    RBO::RBO() noexcept
        : mHandle{0}
    {
        glGenRenderbuffers(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("RBO constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    RBO::~RBO() noexcept
    {
        if (mHandle != 0)
            glDeleteRenderbuffers(1, &mHandle);

        if constexpr (LogGLTypeEvents) LOG_INFO("RBO destroyed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    RBO::RBO(RBO&& pOther) noexcept
        : mHandle{std::move(pOther.mHandle)}
    {
        pOther.mHandle = 0;

        if constexpr (LogGLTypeEvents) LOG_INFO("RBO move-constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    RBO& RBO::operator=(RBO&& pOther) noexcept
    {
        if (this != &pOther)
        {
            // Free the existing resource.
            if (mHandle != 0)
                glDeleteRenderbuffers(1, &mHandle);

            // Copy the data pointer from the source object.
            mHandle = pOther.mHandle;
            // Release the handle so ~RBO doesnt call glDeleteBuffers on mHandle.
            pOther.mHandle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("RBO move-assigned with GLHandle {} at address {}", mHandle, (void*)(this));
        return *this;
    }
    void RBO::bind() const
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
    }



    FBO::FBO() noexcept
        : mHandle{0}
        , mColourAttachment{}
        , mDepthAttachment{}
        , mBufferClearBitField{0}
    {
        glGenFramebuffers(1, &mHandle);
        if constexpr (LogGLTypeEvents) LOG_INFO("FBO constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    FBO::~FBO() noexcept
    {
        if (mHandle != 0)
            glDeleteFramebuffers(1, &mHandle);

        //if (mColourAttachment.has_value() && mColourAttachment->getHandle() != 0)
        //    glDeleteTextures(1, &mColourAttachment->getHandle());
        //if (mDepthAttachment.has_value() && mDepthAttachment->getHandle() != 0)
        //    glDeleteRenderbuffers(1, &mDepthAttachment->getHandle());

        if constexpr (LogGLTypeEvents) LOG_INFO("FBO destroyed with GLHandle {} at address {}", mHandle, (void*)(this));
    }

    FBO::FBO(FBO&& pOther) noexcept
        : mHandle{std::move(pOther.mHandle)}
        , mColourAttachment{std::move(pOther.mColourAttachment)}
        , mDepthAttachment{std::move(pOther.mDepthAttachment)}
        , mBufferClearBitField{std::move(pOther.mBufferClearBitField)}
    {
        pOther.mHandle = 0;
        if constexpr (LogGLTypeEvents) LOG_INFO("FBO move-constructed with GLHandle {} at address {}", mHandle, (void*)(this));
    }
    FBO& FBO::operator=(FBO&& pOther) noexcept
    {
        if (this != &pOther)
        {
            // Free the existing resource.
            if (mHandle != 0)
                glDeleteFramebuffers(1, &mHandle);

            // Copy the data pointer from the source object.
            mHandle = pOther.mHandle;
            // Release the handle so ~FBO doesnt call glDeleteBuffers on mHandle.
            pOther.mHandle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("FBO move-assigned with GLHandle {} at address {}", mHandle, (void*)(this));
        return *this;
    }


    void FBO::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
    }
    void FBO::clearBuffers() const
    {
        glClear(mBufferClearBitField);
    }
    void FBO::bindColourTexture() const
    {
        ZEPHYR_ASSERT(mColourAttachment.has_value(), "Attempting to bind colour texture of an FBO with no colour attatchment");
        mColourAttachment->bind();
    }
    void FBO::attachColourBuffer(const int& pWidth, const int& pHeight)
    {
        ZEPHYR_ASSERT(!mColourAttachment.has_value(), "FBO already has an attached colour");

        bind();
        mColourAttachment = Texture();
        mColourAttachment->bind();

        {// Attaching a colour output texture to FBO
            // Data param is passed as NULL - we're only allocating memory and filling the texture when we render to the FBO.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pWidth, pHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            // GL_NEAREST so that we don't interpolate multiple samples from the intermediate texture to the final screen render.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_LINEAR
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColourAttachment->getHandle(), 0);
        }


        //{// Attaching a depth buffer to texture
        //    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
        //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mColourAttachment->getHandle(), 0);
        //}
        //{// Attaching a stencil buffer to texture
        //    glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, 800, 600, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
        //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mColourAttachment->getHandle(), 0);
        //}
        //{// Attaching both a stencil and depth buffer as a single texture. 32bit = 24 bits of depth info + 8 bits stencil info
        //    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mColourAttachment->getHandle(), 0);
        //}

        mBufferClearBitField |= GL_COLOR_BUFFER_BIT;
        unbind();
    }
    void FBO::detachColourBuffer()
    {
        ZEPHYR_ASSERT(mColourAttachment.has_value(), "There is no attached colour to remove from FBO");
        mColourAttachment.reset();
        mBufferClearBitField &= ~GL_COLOR_BUFFER_BIT;
    }
    void FBO::attachDepthBuffer(const int& pWidth, const int& pHeight)
    {
        ZEPHYR_ASSERT(!mDepthAttachment.has_value(), "FBO already has an attached depth buffer");

        bind();
        mDepthAttachment = RBO();
        mDepthAttachment->bind();

        // Allocate the storage for the buffer then unbind it to make sure we're not accidentally rendering to the wrong framebuffer.
        // Lastly attach it to this FBO
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pWidth, pHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthAttachment->getHandle());
        //glBindRenderbuffer(GL_RENDERBUFFER, 0);

        mBufferClearBitField |= GL_DEPTH_BUFFER_BIT;
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind
    }
    void FBO::detachDepthBuffer()
    {
        ZEPHYR_ASSERT(mDepthAttachment.has_value(), "There is no attached depth buffer to remove from FBO");
        mDepthAttachment.reset();
        mBufferClearBitField &= ~GL_DEPTH_BUFFER_BIT;
    }
    void FBO::resize(const int& pWidth, const int& pHeight)
    {
        if (mColourAttachment.has_value())
        {
            detachColourBuffer();
            attachColourBuffer(pWidth, pHeight);
        }
        if (mDepthAttachment.has_value())
        {
            detachDepthBuffer();
            attachDepthBuffer(pWidth, pHeight);
        }
    }
    bool FBO::isComplete() const
    {
        // This function returns the result of glCheckFramebufferStatus of this FBO.
        // It additionally returns the GLState to the originally bound currentBoundFBO.
        int currentBoundFBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentBoundFBO);

        bind();
        const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glBindFramebuffer(GL_FRAMEBUFFER, currentBoundFBO); // Rebind the originally bound handle.
        return complete;
    }
    void FBO::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Mesh::Mesh(Mesh&& pOther) noexcept
        : mVAO{std::move(pOther.mVAO)}
        , mVertexPositions{std::move(pOther.mVertexPositions)}
        , mVertexNormals{std::move(pOther.mVertexNormals)}
        , mVertexTextureCoordinates{std::move(pOther.mVertexTextureCoordinates)}
        , mEBO{std::move(pOther.mEBO)}
        , mDrawSize{std::move(pOther.mDrawSize)}
    {
        if constexpr (LogGLTypeEvents) LOG_INFO("OpenGL::Mesh move-constructed with VAO {} at address {}", mVAO.getHandle(), (void*)(this));
    }
    Mesh& Mesh::operator=(Mesh&& pOther) noexcept
    {
        if (this != &pOther)
        {
            mVAO                      = std::move(pOther.mVAO);
            mVertexPositions          = std::move(pOther.mVertexPositions);
            mVertexNormals            = std::move(pOther.mVertexNormals);
            mVertexTextureCoordinates = std::move(pOther.mVertexTextureCoordinates);
            mEBO                      = std::move(pOther.mEBO);
            mDrawSize                 = std::move(pOther.mDrawSize);
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("OpenGL::Mesh move-assigned with VAO {} at address {}", mVAO.getHandle(), (void*)(this));
        return *this;
    }

    Mesh::Mesh(const Data::Mesh& pMeshData) noexcept
        : mVAO{}
        , mVertexPositions{}
        , mVertexNormals{}
        , mVertexTextureCoordinates{}
        , mEBO{}
        , mDrawSize{0}
    {
        mVAO.bind(); // Have to bind VAO before buffering VBO and EBO data

        if (!pMeshData.mIndices.empty())
        {
            mEBO = EBO();
            mEBO->bind();
            mEBO->setData(pMeshData.mIndices);
            mDrawSize = static_cast<int>(pMeshData.mIndices.size());
        }
        else
            mDrawSize = static_cast<int>(pMeshData.mPositions.size()) / 3; // Divide verts by 3 as we draw the vertices by Triangles count.

        if (!pMeshData.mPositions.empty())
        {
            mVertexPositions = VBO();
            mVertexPositions->bind();
            mVertexPositions->setData(pMeshData.mPositions, Shader::Attribute::Position3D);
        }
        if (!pMeshData.mNormals.empty())
        {
            mVertexNormals = VBO();
            mVertexNormals->bind();
            mVertexNormals->setData(pMeshData.mNormals, Shader::Attribute::Normal3D);
        }
        if (!pMeshData.mTextureCoordinates.empty())
        {
            mVertexTextureCoordinates = VBO();
            mVertexTextureCoordinates->bind();
            mVertexTextureCoordinates->setData(pMeshData.mTextureCoordinates, Shader::Attribute::TextureCoordinate2D);
        }

        if constexpr (LogGLTypeEvents) LOG_INFO("OpenGL::Mesh constructed with VAO {} at address {}", mVAO.getHandle(), (void*)(this));
    }
}