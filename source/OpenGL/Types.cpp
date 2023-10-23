#include "Types.hpp"

// COMPONENT
#include "Mesh.hpp"
#include "Texture.hpp"

// UTILITY
#include "Logger.hpp"
#include "File.hpp"

// OPENGL
#include "glad/gl.h"

#include <array>

namespace OpenGL
{
    VAO::VAO() noexcept
        : m_handle{0}
    {
        glGenVertexArrays(1, &m_handle);
        if constexpr (LogGLTypeEvents) LOG("VAO constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    VAO::~VAO() noexcept
    {
        // We initialise m_handle to 0 and reset to 0 on move and depend on this to check if there is a vertex array to delete here.
        // We still want to call glDeleteVertexArrays if the array was generated but not used with bind and/or VBOs hence
        // not using glIsVertexArray here.
        if (m_handle != 0)
            glDeleteVertexArrays(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("VAO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    VAO::VAO(VAO&& p_other) noexcept
        : m_handle{std::move(p_other.m_handle)}
    { // Steal the handle of the other VAO and set it to 0 to prevent the p_other destructor calling glDeleteVertexArrays
        p_other.m_handle = 0;

        if constexpr (LogGLTypeEvents) LOG("VAO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    VAO& VAO::operator=(VAO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
                glDeleteVertexArrays(1, &m_handle);

            // Copy the data pointer from the source object.
            m_handle = p_other.m_handle;
            // Release the handle so ~VAO doesnt call glDeleteVertexArrays on m_handle.
            p_other.m_handle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG("VAO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
        return *this;
    }
    void VAO::bind() const
    {
        glBindVertexArray(m_handle);
    }



    EBO::EBO() noexcept
        : m_handle{0}
    {
        glGenBuffers(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("EBO constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    EBO::~EBO() noexcept
    {
        if (m_handle != 0)
            glDeleteBuffers(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("EBO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    EBO::EBO(EBO&& p_other) noexcept
        : m_handle{std::move(p_other.m_handle)}
    {
        p_other.m_handle = 0;

        if constexpr (LogGLTypeEvents) LOG("EBO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    EBO& EBO::operator=(EBO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
                glDeleteBuffers(1, &m_handle);

            // Copy the data pointer from the source object.
            m_handle = p_other.m_handle;
            // Release the handle so ~EBO doesnt call glDeleteBuffers on m_handle.
            p_other.m_handle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG("EBO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
        return *this;
    }
    void EBO::bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle);
    }
    void EBO::setData(const std::vector<int>& pIndexData) const
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pIndexData.size() * sizeof(int), &pIndexData.front(), GL_STATIC_DRAW);
    }



    VBO::VBO() noexcept
    : m_handle{0}
    , m_size{0}
    {
        glGenBuffers(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("VBO created with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    VBO::~VBO() noexcept
    {
        if (m_handle != 0)
            glDeleteBuffers(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("VBO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    VBO::VBO(VBO&& p_other) noexcept
        : m_handle{std::move(p_other.m_handle)}
        , m_size{std::move(p_other.m_size)}
    {
        p_other.m_handle = 0;
        p_other.m_size = 0;

        if constexpr (LogGLTypeEvents) LOG("VBO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    VBO& VBO::operator=(VBO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
                glDeleteBuffers(1, &m_handle);

            // Copy the data pointer from the source object.
            m_handle = p_other.m_handle;
            m_size = p_other.m_size;
            // Release the handle so ~VBO doesnt call glDeleteBuffers on m_handle.
            p_other.m_handle = 0;
            p_other.m_size = 0;
        }

        if constexpr (LogGLTypeEvents) LOG("VBO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
        return *this;
    }
    void VBO::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_handle);
    }

    void VBO::setData(const std::vector<glm::vec3>& pVec3Data, const VertexAttribute& pAttributeType)
    {
        m_size = pVec3Data.size() * sizeof(glm::vec3);
        glBufferData(GL_ARRAY_BUFFER, m_size, &pVec3Data.front(), GL_STATIC_DRAW);

        auto index = impl::get_attribute_index(pAttributeType);
        auto count = impl::get_attribute_component_count(pAttributeType);
        glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(index);
    }
    void VBO::setData(const std::vector<glm::vec2>& pVec2Data, const VertexAttribute& pAttributeType)
    {
        m_size = pVec2Data.size() * sizeof(glm::vec2);
        glBufferData(GL_ARRAY_BUFFER, m_size, &pVec2Data.front(), GL_STATIC_DRAW);

        auto index = impl::get_attribute_index(pAttributeType);
        auto count = impl::get_attribute_component_count(pAttributeType);
        glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(index);
    }
    void VBO::clear()
    {
        bind();
        m_size = 0;
        glBufferData(GL_ARRAY_BUFFER, m_size, NULL, GL_STATIC_DRAW);
    }
    void VBO::copy(const VBO& pSource, VBO& pDestination)
    {
        glBindBuffer(GL_COPY_WRITE_BUFFER, pDestination.m_handle);
        // glBufferData deletes pre-existing data, we additionally call with p_data as NULL which
        // gives us a buffer of pSource.m_size uninitialised.
        glBufferData(GL_COPY_WRITE_BUFFER, pSource.m_size, NULL, GL_STREAM_COPY);

        glBindBuffer(GL_COPY_READ_BUFFER, pSource.m_handle);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, pSource.m_size);

        pDestination.m_size = pSource.m_size;
    }

    Texture::~Texture()
    {
        if (m_handle != 0)
            glDeleteTextures(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("Texture destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    Texture::Texture()
        : m_handle{0}
    {
        glGenTextures(1, &m_handle);
        if constexpr (LogGLTypeEvents) LOG("Texture constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    Texture::Texture(const Utility::Image& p_image)
        : m_handle{0}
    {
        glGenTextures(1, &m_handle);
        glBindTexture(GL_TEXTURE_2D, m_handle);

        GLenum format = 0;
        if (p_image.m_number_of_channels == 1)      format = GL_RED;
        else if (p_image.m_number_of_channels == 3) format = GL_RGB;
        else if (p_image.m_number_of_channels == 4) format = GL_RGBA;
        ASSERT(format != 0, "Could not find channel type for this number of texture channels");

        glTexImage2D(GL_TEXTURE_2D, 0, format, p_image.m_width, p_image.m_height, 0, format, GL_UNSIGNED_BYTE, p_image.get_data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // GL_REPEAT - (default wrapping method)
        // GL_CLAMP_TO_EDGE - when using transparency to stop interpolation at borders causing semi-transparent artifacts.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);

        if constexpr (LogGLTypeEvents) LOG("Texture constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    Texture::Texture(Texture&& p_other) noexcept
        : m_handle{std::move(p_other.m_handle)}
    {
        p_other.m_handle = 0;

        if constexpr (LogGLTypeEvents) LOG("Texture move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    Texture& Texture::operator=(Texture&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
                glDeleteTextures(1, &m_handle);

            // Copy the data pointer from the source object.
            m_handle = p_other.m_handle;
            // Release the handle so ~Texture doesnt call glDeleteBuffers on m_handle.
            p_other.m_handle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG("Texture move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
        return *this;
    }
    void Texture::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, m_handle);
    };

    RBO::RBO() noexcept
        : m_handle{0}
    {
        glGenRenderbuffers(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("RBO constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    RBO::~RBO() noexcept
    {
        if (m_handle != 0)
            glDeleteRenderbuffers(1, &m_handle);

        if constexpr (LogGLTypeEvents) LOG("RBO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    RBO::RBO(RBO&& p_other) noexcept
        : m_handle{std::move(p_other.m_handle)}
    {
        p_other.m_handle = 0;

        if constexpr (LogGLTypeEvents) LOG("RBO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    RBO& RBO::operator=(RBO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
                glDeleteRenderbuffers(1, &m_handle);

            // Copy the data pointer from the source object.
            m_handle = p_other.m_handle;
            // Release the handle so ~RBO doesnt call glDeleteBuffers on m_handle.
            p_other.m_handle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG("RBO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
        return *this;
    }
    void RBO::bind() const
    {
        glBindRenderbuffer(GL_RENDERBUFFER, m_handle);
    }



    FBO::FBO() noexcept
        : m_handle{0}
        , mColourAttachment{}
        , m_depth_map{}
        , mDepthAttachment{}
        , mBufferClearBitField{0}
    {
        glGenFramebuffers(1, &m_handle);
        if constexpr (LogGLTypeEvents) LOG("FBO constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    FBO::~FBO() noexcept
    {
        if (m_handle != 0)
            glDeleteFramebuffers(1, &m_handle);

        //if (mColourAttachment.has_value() && mColourAttachment->getHandle() != 0)
        //    glDeleteTextures(1, &mColourAttachment->getHandle());
        //if (mDepthAttachment.has_value() && mDepthAttachment->getHandle() != 0)
        //    glDeleteRenderbuffers(1, &mDepthAttachment->getHandle());

        if constexpr (LogGLTypeEvents) LOG("FBO destroyed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    FBO::FBO(FBO&& p_other) noexcept
        : m_handle{std::move(p_other.m_handle)}
        , mColourAttachment{std::move(p_other.mColourAttachment)}
        , m_depth_map{std::move(p_other.m_depth_map)}
        , mDepthAttachment{std::move(p_other.mDepthAttachment)}
        , mBufferClearBitField{std::move(p_other.mBufferClearBitField)}
    {
        p_other.m_handle = 0;
        if constexpr (LogGLTypeEvents) LOG("FBO move-constructed with GLHandle {} at address {}", m_handle, (void*)(this));
    }
    FBO& FBO::operator=(FBO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
                glDeleteFramebuffers(1, &m_handle);

            // Copy the data pointer from the source object.
            m_handle = p_other.m_handle;
            // Release the handle so ~FBO doesnt call glDeleteBuffers on m_handle.
            p_other.m_handle = 0;
        }

        if constexpr (LogGLTypeEvents) LOG("FBO move-assigned with GLHandle {} at address {}", m_handle, (void*)(this));
        return *this;
    }

    void FBO::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
    }
    void FBO::clearBuffers() const
    {
        glClear(mBufferClearBitField);
    }
    void FBO::bindColourTexture() const
    {
        ASSERT(mColourAttachment.has_value(), "Attempting to bind colour texture of an FBO with no colour attatchment");
        mColourAttachment->bind();
    }
    void FBO::attachColourBuffer(const int& pWidth, const int& pHeight)
    {
        ASSERT(!mColourAttachment.has_value(), "FBO already has an attached colour");

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
        ASSERT(mColourAttachment.has_value(), "There is no attached colour to remove from FBO");
        mColourAttachment.reset();
        mBufferClearBitField &= ~GL_COLOR_BUFFER_BIT;
    }
    void FBO::attachDepthBuffer(const int& pWidth, const int& pHeight)
    {
        ASSERT(!mDepthAttachment.has_value(), "[OPENGL][FBO] FBO already has an attached depth buffer.");
        ASSERT(!m_depth_map.has_value(), "[OPENGL][FBO] FBO already has an attached depth buffer Texture map.");

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
    void FBO::attach_depth_buffer(const glm::uvec2& p_resolution)
    {
        ASSERT(!mDepthAttachment.has_value() && !m_depth_map.has_value(), "[OPENGL][FBO] FBO already has an attached depth buffer.");

        mBufferClearBitField |= GL_DEPTH_BUFFER_BIT;

        m_depth_map = Texture();
        m_depth_map->bind();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, p_resolution.x, p_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_map->getHandle(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void FBO::detachDepthBuffer()
    {
        ASSERT(mDepthAttachment.has_value() || m_depth_map.has_value(), "[OPENGL][FBO] There is no attached depth buffer to remove from FBO.");

        if (mDepthAttachment.has_value())
            mDepthAttachment.reset();
        else if (m_depth_map.has_value())
            m_depth_map.reset();

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
        else if (m_depth_map.has_value())
        {
            detachDepthBuffer();
            attach_depth_buffer(glm::uvec2(pWidth, pHeight));
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

    ShaderStorageBlockVariable::ShaderStorageBlockVariable(GLHandle p_parent_shader_program, GLuint p_variable_index) noexcept
        : m_identifier{""}
        , m_type{ShaderDataType::Unknown}
        , m_offset{-1}
        , m_array_size{-1}
        , m_array_stride{-1}
        , m_matrix_stride{-1}
        , m_is_row_major{-1}
        , m_top_level_array_size{-1}
        , m_top_level_array_stride{-1}
    {
        // Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
        // https://www.khronos.org/opengl/wiki/Program_Introspection
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetProgramResource.xhtml
        static constexpr size_t property_count = 9;
        static constexpr GLenum property_query[property_count] = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE};
        GLint property_values[property_count] = {-1};
        glGetProgramResourceiv(p_parent_shader_program, GL_BUFFER_VARIABLE, p_variable_index, property_count, &property_query[0], property_count, NULL, &property_values[0]);

        m_identifier.resize(property_values[0]);
        glGetProgramResourceName(p_parent_shader_program, GL_BUFFER_VARIABLE, p_variable_index, property_values[0], NULL, m_identifier.data());
        ASSERT(!m_identifier.empty(), "Failed to get name of uniform variable in shader with handle {}", p_parent_shader_program);
        m_identifier.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        m_type                   = convert(property_values[1]);
        m_offset                 = property_values[2];
        m_array_size             = property_values[3];
        m_array_stride           = property_values[4];
        m_matrix_stride          = property_values[5];
        m_is_row_major           = property_values[6];
        m_top_level_array_size   = property_values[7];
        m_top_level_array_stride = property_values[8];
    }

    SSBO::SSBO(const std::string& p_storage_block_identifier, GLsizei p_size, const std::vector<ShaderStorageBlockVariable>& p_variables) noexcept
        : m_handle{0}
        , m_size{p_size}
        , m_binding_point{0}
        , m_variables{p_variables}
        , m_identifier{p_storage_block_identifier}
    {
        glGenBuffers(1, &m_handle);

        bind();
        // Supplying NULL as p_data to buffer_data reserves the Bytes but does not assign to them.
        buffer_data(BufferType::ShaderStorageBuffer, m_size, NULL, BufferUsage::StaticDraw);

        // Bind ourselves to the first available binding point.
        {
            // #TODO Don't create an array of max_size because this could be arbitrarily large - Serach for first false || push_back then + check if the index hasnt reached the max instead.
            if (s_binding_points.empty()) // First SSBO construction
                s_binding_points = std::vector<bool>(get_max_shader_storage_binding_points(), false);

            auto it = std::find(s_binding_points.begin(), s_binding_points.end(), false);
            ASSERT(it != s_binding_points.end(), "[OPENGL][SSBO] No remaining shader storage block binding points to bind to."); // Always
            (*it) = true;
            m_binding_point = std::distance(s_binding_points.begin(), it);
        }

        bind_buffer_range(BufferType::ShaderStorageBuffer, m_binding_point, m_handle, 0, m_size);

        if constexpr (LogGLTypeEvents) LOG("SSBO '{}' created with GLHandle {}, size {}B binding point {} at address {}", m_identifier, m_handle, m_size, m_binding_point, (void*)(this));
    }
    SSBO::~SSBO() noexcept
    {
        if (m_handle != 0)
        {
            glDeleteBuffers(1, &m_handle);
            s_binding_points[m_binding_point] = false;
            if constexpr (LogGLTypeEvents) LOG("SSBO '{}' free resource. GLHandle: {} Size: {}B bounding point: {}", m_identifier, m_handle, m_size, m_binding_point);
        }

        if constexpr (LogGLTypeEvents) LOG("UBO '{}' destroyed address: {}", m_identifier, m_handle, (void*)(this));
    }
    SSBO::SSBO(SSBO&& p_other) noexcept
        : m_handle{std::exchange(p_other.m_handle, 0)}
        , m_size{std::exchange(p_other.m_size, 0)}
        , m_binding_point{std::exchange(p_other.m_binding_point, 0)}
        , m_variables{std::exchange(p_other.m_variables, {})}
        , m_identifier{p_other.m_identifier}
    {
        if constexpr (LogGLTypeEvents) LOG("SSBO '{}' move-constructed with GLHandle {} at address {}", m_identifier, m_handle, (void*)(this));
    }
    SSBO& SSBO::operator=(SSBO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
            {
                glDeleteBuffers(1, &m_handle);
                s_binding_points[m_binding_point] = false;
                if constexpr (LogGLTypeEvents) LOG("SSBO '{}' free resource. GLHandle: {} Size: {}B bounding point: {}", m_identifier, m_handle, m_size, m_binding_point);
            }

            // Take the m_handle from p_other source object.
            // Assign the p_other m_handle to 0 so ~SSBO doesnt call glDeleteBuffers on m_handle.
            m_handle        = std::exchange(p_other.m_handle, 0);
            m_size          = std::exchange(p_other.m_size, 0);
            m_binding_point = std::exchange(p_other.m_binding_point, 0);
            m_variables     = std::exchange(p_other.m_variables, {});
            m_identifier    = p_other.m_identifier; // p_other retains the m_identifier for output in destruction.
        }

        if constexpr (LogGLTypeEvents) LOG("SSBO '{}' move-assigned with GLHandle {} at address {}", m_identifier, m_handle, (void*)(this));
        return *this;
    }
    void SSBO::bind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    }

    ShaderStorageBlock::ShaderStorageBlock(GLHandle p_shader_program, GLuint p_shader_storage_block_index) noexcept
        : m_identifier{""}
        , m_index{p_shader_storage_block_index}
        , m_parent_shader_program{p_shader_program}
        , m_variables{}
        , m_buffer_backing{std::nullopt}
    {
        static constexpr size_t Property_Count = 4;
        // GL_BUFFER_BINDING is unused, returns the binding point (layout(binding = X)) for the ShaderStorageBlock
        // We set this manually after constructing a SSBO and binding this ShaderStorageBlock to its m_buffer_backing m_uniform_binding_point.
        static constexpr GLenum property_query[Property_Count] = {GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
        GLint property_values[Property_Count] = {-1};
        glGetProgramResourceiv(p_shader_program, GL_SHADER_STORAGE_BLOCK, m_index, Property_Count, &property_query[0], Property_Count, NULL, &property_values[0]);

        m_identifier.resize(property_values[0]);
        glGetProgramResourceName(p_shader_program, GL_SHADER_STORAGE_BLOCK, m_index, property_values[0], NULL, m_identifier.data());
        ASSERT(!m_identifier.empty(), "Failed to get name of shader storage block in shader with handle {}", p_shader_program);
        m_identifier.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        ASSERT(m_index == glGetProgramResourceIndex(p_shader_program, GL_SHADER_STORAGE_BLOCK, m_identifier.c_str()), "shader storage block index given doesnt match the shader program index for the same name block!");

        const GLint active_variables_count = property_values[1];
        if (active_variables_count > 0)
        {
            // Get the array of active variable indices associated with the shader storage block. (GL_ACTIVE_VARIABLES)
            // The indices correspond in size to GL_NUM_ACTIVE_VARIABLES

            std::vector<GLint> variable_indices(active_variables_count);
            static constexpr GLenum active_variable_query[1] = {GL_ACTIVE_VARIABLES};
            glGetProgramResourceiv(p_shader_program, GL_SHADER_STORAGE_BLOCK, m_index, 1, active_variable_query, active_variables_count, NULL, &variable_indices[0]);

            for (GLint variable_index : variable_indices)
                m_variables.emplace_back(p_shader_program, static_cast<GLuint>(variable_index));
        }

        const GLint buffer_data_size = property_values[3];
        ASSERT(buffer_data_size <= get_max_shader_storage_block_size(), "[OPENGL][SHADER] ShaderStorageBlock larger than max size.");

        // To find a SSBO that can back this shader storage block:
        // The names of the blocks have to match.
        // The variables must be the same and listed in the same order.
        // #TODO: the block must be marked as shared
        m_buffer_backing = s_shader_storage_block_binding_points.getOrCreate([this](const SSBO& p_SSBO)
        {
            if (p_SSBO.m_identifier == m_identifier)
            {
                if (p_SSBO.m_variables == m_variables)
                    return true;
                else
                {
                    ASSERT(false, "[OPENGL][SHADER] shader storage block '{}' identifier repeated with different variables! Did you mess up the order or names of types in the block? Have any variables been optimised away?", m_identifier);
                    return false;
                }
            }
            else
                return false;
        }, m_identifier, buffer_data_size, m_variables);

        ASSERT(m_variables.size() == active_variables_count && (*m_buffer_backing)->m_variables == m_variables, "Failed to retrieve all the ShaderStorageBlock of block '{}'", m_identifier);

        // Bind the ShaderStorageBlock to the binding point the buffer backing SSBO is bound to.
        // SSBO constrcutor called the corresponding bind_buffer_range to the same binding point.
        shader_storage_block_binding(m_parent_shader_program, m_index, (*m_buffer_backing)->m_binding_point);

        if constexpr (LogGLTypeEvents) LOG("[OPENGL][SHADER] ShaderStorageBlock '{}' bound to point index {}", m_identifier, (*m_buffer_backing)->m_binding_point);
    }

    UniformBlockVariable::UniformBlockVariable(GLHandle p_shader_program, GLuint p_block_variable_index) noexcept
        : m_name{""}
        , m_type{ShaderDataType::Unknown}
        , m_offset{-1}
        , m_array_size{-1}
        , m_array_stride{-1}
        , m_matrix_stride{-1}
        , m_location{-1}
        , m_is_row_major{false}
    {
        // Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
        // https://www.khronos.org/opengl/wiki/Program_Introspection
        static constexpr size_t property_count = 8;
        static constexpr GLenum property_query[property_count] = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_LOCATION, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR};
        GLint property_values[property_count] = {-1};
        glGetProgramResourceiv(p_shader_program, GL_UNIFORM, p_block_variable_index, property_count, &property_query[0], property_count, NULL, &property_values[0]);

        m_name.resize(property_values[0]);
        glGetProgramResourceName(p_shader_program, GL_UNIFORM, p_block_variable_index, property_values[0], NULL, m_name.data());
        ASSERT(!m_name.empty(), "Failed to get name of uniform variable in shader with handle {}", p_shader_program);
        m_name.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        m_type           = convert(property_values[1]);
        m_offset         = property_values[2];
        m_location       = property_values[3];
        m_array_size     = property_values[4];
        m_array_stride   = property_values[5];
        m_matrix_stride  = property_values[6];
        m_is_row_major   = property_values[7];
    }

    UBO::UBO(const std::string& p_uniform_block_name, GLsizei p_size, const std::vector<UniformBlockVariable>& p_variables) noexcept
        : m_handle{0}
        , m_size{p_size}
        , m_uniform_binding_point{0}
        , m_variables{p_variables}
        , m_uniform_block_name{p_uniform_block_name}
    {
        glGenBuffers(1, &m_handle);

        bind();
        // Supplying NULL as p_data to buffer_data reserves the Bytes but does not assign to them.
        buffer_data(BufferType::UniformBuffer, m_size, NULL, BufferUsage::StaticDraw);

        // Bind ourselves to the first available binding point.
        {
            // #TODO Don't create an array of max_size because this could be arbitrarily large - Serach for first false || push_back then + check if the index hasnt reached the max instead.
            if (s_binding_points.empty()) // First UBO construction
                s_binding_points = std::vector<bool>(get_max_uniform_binding_points(), false);

            auto it = std::find(s_binding_points.begin(), s_binding_points.end(), false);
            ASSERT(it != s_binding_points.end(), "[OPENGL][UBO] No remaining uniform block binding points to bind to."); // Always
            (*it) = true;
            m_uniform_binding_point = std::distance(s_binding_points.begin(), it);
        }

        bind_buffer_range(BufferType::UniformBuffer, m_uniform_binding_point, m_handle, 0, m_size);

        if constexpr (LogGLTypeEvents) LOG("UBO '{}' created with GLHandle {}, size {}B binding point {} at address {}", m_uniform_block_name, m_handle, m_size, m_uniform_binding_point, (void*)(this));
    }
    UBO::~UBO() noexcept
    {
        if (m_handle != 0)
        {
            glDeleteBuffers(1, &m_handle);
            s_binding_points[m_uniform_binding_point] = false;
            if constexpr (LogGLTypeEvents) LOG("UBO '{}' free resource. GLHandle: {} Size: {}B bounding point: {}", m_uniform_block_name, m_handle, m_size, m_uniform_binding_point);
        }

        if constexpr (LogGLTypeEvents) LOG("UBO '{}' destroyed address: {}", m_uniform_block_name, m_handle, (void*)(this));
    }
    UBO::UBO(UBO&& p_other) noexcept
        : m_handle{std::exchange(p_other.m_handle, 0)} // Ensure the p_other m_handle is set to 0 to prevent its destructor clearing memory.
        , m_size{std::exchange(p_other.m_size, 0)}
        , m_uniform_binding_point{std::exchange(p_other.m_uniform_binding_point, 0)}
        , m_variables{std::exchange(p_other.m_variables, {})}
        , m_uniform_block_name{p_other.m_uniform_block_name} // p_other retains the m_uniform_block_name for output in destruction.
    {
        if constexpr (LogGLTypeEvents) LOG("UBO '{}' move-constructed with GLHandle {} at address {}", m_uniform_block_name, m_handle, (void*)(this));
    }
    UBO& UBO::operator=(UBO&& p_other) noexcept
    {
        if (this != &p_other)
        {
            // Free the existing resource.
            if (m_handle != 0)
            {
                glDeleteBuffers(1, &m_handle);
                s_binding_points[m_uniform_binding_point] = false;
                if constexpr (LogGLTypeEvents) LOG("UBO '{}' free resource. GLHandle: {} Size: {}B bounding point: {}", m_uniform_block_name, m_handle, m_size, m_uniform_binding_point);
            }

            // Take the m_handle from p_other source object.
            // Assign the p_other m_handle to 0 so ~UBO doesnt call glDeleteBuffers on m_handle.
            m_handle                = std::exchange(p_other.m_handle, 0);
            m_size                  = std::exchange(p_other.m_size, 0);
            m_uniform_binding_point = std::exchange(p_other.m_uniform_binding_point, 0);
            m_variables             = std::exchange(p_other.m_variables, {});
            m_uniform_block_name    = p_other.m_uniform_block_name; // p_other retains the m_uniform_block_name for output in destruction.
        }

        if constexpr (LogGLTypeEvents) LOG("UBO '{}' move-assigned with GLHandle {} at address {}", m_uniform_block_name, m_handle, (void*)(this));
        return *this;
    }
    void UBO::bind() const
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_handle);
    }

    UniformBlock::UniformBlock(GLHandle p_shader_program, GLuint p_uniform_block_index) noexcept
        : m_name{""}
        , m_block_index{p_uniform_block_index}
        , m_parent_shader_program{p_shader_program}
        , m_variables{}
        , m_buffer_backing{std::nullopt}
    {
        static constexpr size_t Property_Count = 4;
        // GL_BUFFER_BINDING is unused, returns the binding point (layout(binding = X)) for the UniformBlock
        // We set this manually after constructing a UBO and binding this UniformBlock to its m_buffer_backing m_uniform_binding_point.
        static constexpr GLenum property_query[Property_Count] = {GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
        GLint property_values[Property_Count] = {-1};
        glGetProgramResourceiv(p_shader_program, GL_UNIFORM_BLOCK, m_block_index, Property_Count, &property_query[0], Property_Count, NULL, &property_values[0]);

        m_name.resize(property_values[0]);
        glGetProgramResourceName(p_shader_program, GL_UNIFORM_BLOCK, m_block_index, property_values[0], NULL, m_name.data());
        ASSERT(!m_name.empty(), "Failed to get name of uniform block in shader with handle {}", p_shader_program);
        m_name.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        ASSERT(m_block_index == glGetProgramResourceIndex(p_shader_program, GL_UNIFORM_BLOCK, m_name.c_str()), "Uniform Block index given doesnt match the shader program index for the same name block!");

        const GLint active_variables_count = property_values[1];
        if (active_variables_count > 0)
        {
            // Get the array of active variable indices associated with the uniform block. (GL_ACTIVE_VARIABLES)
            // The indices correspond in size to GL_NUM_ACTIVE_VARIABLES

            //m_variables.reserve(active_variables_count);

            std::vector<GLint> variable_indices(active_variables_count);
            static constexpr GLenum active_variable_query[1] = {GL_ACTIVE_VARIABLES};
            glGetProgramResourceiv(p_shader_program, GL_UNIFORM_BLOCK, m_block_index, 1, active_variable_query, active_variables_count, NULL, &variable_indices[0]);

            for (GLint variable_index : variable_indices)
                m_variables.emplace_back(p_shader_program, static_cast<GLuint>(variable_index));
        }

        const GLint buffer_data_size = property_values[3];
        ASSERT(buffer_data_size <= get_max_uniform_block_size(), "[OPENGL][SHADER] UniformBlock larger than max size.");

        // To find a UBO that can back this uniform block:
        // The names of the blocks have to match.
        // The variables must be the same and listed in the same order.
        // TODO: the block must be marked as shared?
        m_buffer_backing = uniform_block_binding_points.getOrCreate([this](const UBO& p_UBO)
        {
            if (p_UBO.m_uniform_block_name == m_name)
            {
                if (p_UBO.m_variables == m_variables)
                    return true;
                else
                {
                    ASSERT(false, "[OPENGL][SHADER] Uniform block '{}' identifier repeated with different variables! Did you mess up the order or names of types in the block? Have any variables been optimised away?", m_name);
                    return false;
                }
            }
            else
                return false;
        }, m_name, buffer_data_size, m_variables);

        ASSERT(m_variables.size() == active_variables_count && (*m_buffer_backing)->m_variables == m_variables, "Failed to retrieve all the UniformBlockVariables of block '{}'", m_name);

        // Bind the UniformBlock to the binding point the buffer backing UBO is bound to. UBO constrcutor called the corresponding bind_buffer_range to the same binding point.
        uniform_block_binding(m_parent_shader_program, m_block_index, (*m_buffer_backing)->m_uniform_binding_point);

        if constexpr (LogGLTypeEvents) LOG("[OPENGL][SHADER] UniformBlock '{}' bound to point index {}", m_name, (*m_buffer_backing)->m_uniform_binding_point);
    }

    Mesh::Mesh(Mesh&& p_other) noexcept
        : mVAO{std::move(p_other.mVAO)}
        , mVertexPositions{std::move(p_other.mVertexPositions)}
        , mVertexNormals{std::move(p_other.mVertexNormals)}
        , mVertexTextureCoordinates{std::move(p_other.mVertexTextureCoordinates)}
        , mEBO{std::move(p_other.mEBO)}
        , mDrawSize{std::move(p_other.mDrawSize)}
    {
        if constexpr (LogGLTypeEvents) LOG("OpenGL::Mesh move-constructed with VAO {} at address {}", mVAO.getHandle(), (void*)(this));
    }
    Mesh& Mesh::operator=(Mesh&& p_other) noexcept
    {
        if (this != &p_other)
        {
            mVAO                      = std::move(p_other.mVAO);
            mVertexPositions          = std::move(p_other.mVertexPositions);
            mVertexNormals            = std::move(p_other.mVertexNormals);
            mVertexTextureCoordinates = std::move(p_other.mVertexTextureCoordinates);
            mEBO                      = std::move(p_other.mEBO);
            mDrawSize                 = std::move(p_other.mDrawSize);
        }

        if constexpr (LogGLTypeEvents) LOG("OpenGL::Mesh move-assigned with VAO {} at address {}", mVAO.getHandle(), (void*)(this));
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
            mVertexPositions->setData(pMeshData.mPositions, VertexAttribute::Position3D);
        }
        if (!pMeshData.mNormals.empty())
        {
            mVertexNormals = VBO();
            mVertexNormals->bind();
            mVertexNormals->setData(pMeshData.mNormals, VertexAttribute::Normal3D);
        }
        if (!pMeshData.mTextureCoordinates.empty())
        {
            mVertexTextureCoordinates = VBO();
            mVertexTextureCoordinates->bind();
            mVertexTextureCoordinates->setData(pMeshData.mTextureCoordinates, VertexAttribute::TextureCoordinate2D);
        }

        if constexpr (LogGLTypeEvents) LOG("OpenGL::Mesh constructed with VAO {} at address {}", mVAO.getHandle(), (void*)(this));
    }

    void Mesh::draw() const
    {
        mVAO.bind();

        if (mEBO.has_value())
            draw_elements(PrimitiveMode::Triangles, mDrawSize);
        else
            draw_arrays(PrimitiveMode::Triangles, 0, mDrawSize);
    }

    namespace impl
    {
        int get_attribute_index(VertexAttribute p_attribute)
        {
            switch (p_attribute)
            {// Each index has to be unique. If changing any pre-existing attributes. Change all the occurrences in the GLSL shaders.
                case VertexAttribute::Position3D:          return 0; // X, Y and Z position components
                case VertexAttribute::Normal3D:            return 1; // X, Y and Z direction components
                case VertexAttribute::ColourRGB:           return 2; // Red, Green and Blue components
                case VertexAttribute::ColourRGBA:          return 4; // Red, Green and Blue components
                case VertexAttribute::TextureCoordinate2D: return 3; // X and Y components
                default: ASSERT(false, "Could not determine the index of the attribute p_attribute. If adding a new attribute, be sure to specify \"(location = 'NewIndex')\" in the GLSL shader source."); return 0; // Always
            }
        }
        int get_attribute_stride(VertexAttribute p_attribute)
        {
            switch (p_attribute)
            {
                case VertexAttribute::Position3D:          return sizeof(float) * 3; // X, Y and Z position components
                case VertexAttribute::Normal3D:            return sizeof(float) * 3; // X, Y and Z direction components
                case VertexAttribute::ColourRGB:           return sizeof(float) * 3; // Red, Green and Blue components
                case VertexAttribute::ColourRGBA:          return sizeof(float) * 4; // Red, Green, Blue and Alpha components
                case VertexAttribute::TextureCoordinate2D: return sizeof(float) * 2; // X and Y components
                default: ASSERT(false, "Could not determine the size of the attribute p_attribute."); return 0; // Always
            }
        }
        int get_attribute_component_count(VertexAttribute p_attribute)
        {
            switch (p_attribute)
            {
                case VertexAttribute::Position3D:          return 3; // X, Y and Z position components
                case VertexAttribute::Normal3D:            return 3; // X, Y and Z direction components
                case VertexAttribute::ColourRGB:           return 3; // Red, Green and Blue components
                case VertexAttribute::ColourRGBA:          return 4; // Red, Green, Blue and Alpha components
                case VertexAttribute::TextureCoordinate2D: return 2; // X and Y components
                default: ASSERT(false, "Could not determine the component count of the p_attribute."); return 0; // Always
            }
        }
        const char* get_attribute_identifier(VertexAttribute p_attribute)
        {
            switch (p_attribute)
            {
                case VertexAttribute::Position3D:          return "VertexPosition";
                case VertexAttribute::Normal3D:            return "VertexNormal";
                case VertexAttribute::ColourRGB:           return "VertexColour";
                case VertexAttribute::ColourRGBA:          return "VertexColour";
                case VertexAttribute::TextureCoordinate2D: return "VertexTexCoord";
                default: ASSERT(false, "Could not convert VertexAttribute to an identifier. If adding a new attribute, be sure to use the identifier added here for it."); return ""; // Always
            }
        }
        ShaderDataType get_attribute_type(VertexAttribute p_attribute)
        {
            switch (p_attribute)
            {
                case VertexAttribute::Position3D:          return ShaderDataType::Float;
                case VertexAttribute::Normal3D:            return ShaderDataType::Float;
                case VertexAttribute::ColourRGB:           return ShaderDataType::Float;
                case VertexAttribute::ColourRGBA:          return ShaderDataType::Float;
                case VertexAttribute::TextureCoordinate2D: return ShaderDataType::Float;
                default: ASSERT(false, "Could not convert VertexAttribute to a datatype. If adding a new attribute, be sure to use the corresponding data type added here for it."); return ShaderDataType::Unknown; // Always
            }
        }
    }
}