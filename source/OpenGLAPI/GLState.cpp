#include "GLState.hpp"

#include "Logger.hpp"

#include "glad/gl.h"
#include "imgui.h"

GLState::GLState()
    : mDepthTest(true)
    , mDepthTestType(GLType::DepthTestType::Less)
    , mBlend(true)
    , mSourceBlendFactor(GLType::BlendFactorType::SourceAlpha)
    , mDestinationBlendFactor(GLType::BlendFactorType::OneMinusSourceAlpha)
    , mCullFaces(true)
    , mCullFacesType(GLType::CullFacesType::Back)
    , mFrontFaceOrientation(GLType::FrontFaceOrientation::CounterClockwise)
    , mWindowClearColour{0.0f, 0.0f, 0.0f, 1.0f}
    , mPolygonMode(GLType::PolygonMode::Fill)
    , mActiveTextureUnit(0)
    , mViewport{0, 0, 0, 0} // Set in constructor
{
    toggleDepthTest(mDepthTest);
    if (mDepthTest)
        setDepthTestType(mDepthTestType);

    toggleBlending(mBlend);
    if (mBlend)
        setBlendFunction(mSourceBlendFactor, mDestinationBlendFactor);

    toggleCullFaces(mCullFaces);
    if (mCullFaces)
    {
        setCullFacesType(mCullFacesType);
        setFrontFaceOrientation(mFrontFaceOrientation);
    }

    setClearColour(mWindowClearColour);
    setPolygonMode(mPolygonMode);
    setActiveTextureUnit(mActiveTextureUnit);

    {// Set Viewport
        // glviewport is set up by the window created before GL is initialised so we query it directly and assign to our GLState tracked member mViewport
        glGetIntegerv(GL_VIEWPORT, mViewport.data());
    }

    ZEPHYR_ASSERT(validateState(), "GLState is inconsistant with actual OpenGL state.");
}

GLState& GLState::operator=(const GLState& pOther)
{
    if (mDepthTest != pOther.mDepthTest)
        toggleDepthTest(pOther.mDepthTest);
    if (mDepthTestType != pOther.mDepthTestType)
        setDepthTestType(pOther.mDepthTestType);

    if (mBlend != pOther.mBlend)
        toggleBlending(pOther.mBlend);
    if (mSourceBlendFactor != pOther.mSourceBlendFactor)
        setBlendFunction(pOther.mSourceBlendFactor, mDestinationBlendFactor);
    if (mDestinationBlendFactor != pOther.mDestinationBlendFactor)
        setBlendFunction(mSourceBlendFactor, pOther.mDestinationBlendFactor);

    if (mCullFaces != pOther.mCullFaces)
        toggleCullFaces(pOther.mCullFaces);
    if (mCullFacesType != pOther.mCullFacesType)
        setCullFacesType(pOther.mCullFacesType);
    if (mFrontFaceOrientation != pOther.mFrontFaceOrientation)
        setFrontFaceOrientation(pOther.mFrontFaceOrientation);

    if (mWindowClearColour != pOther.mWindowClearColour)
        setClearColour(pOther.mWindowClearColour);

    if (mPolygonMode != pOther.mPolygonMode)
        setPolygonMode(pOther.mPolygonMode);

    if (mActiveTextureUnit != pOther.mActiveTextureUnit)
        setActiveTextureUnit(pOther.mActiveTextureUnit);

    if (mViewport != pOther.mViewport)
        setViewport(pOther.mViewport[0], pOther.mViewport[1]);

    ZEPHYR_ASSERT(validateState(), "Copying GLState failed, there are inconsistencies between OpenGL state.");
    return *this;
}

bool GLState::validateState()
{
    { // Check depth test flags
        if (mDepthTest != static_cast<bool>(glIsEnabled(GL_DEPTH_TEST)))
            return false;

        static int depthTestType;
        glGetIntegerv(GL_DEPTH_FUNC, &depthTestType);
        if (convert(mDepthTestType) != depthTestType)
            return false;
    }

    {// Check blend flags match
        if (mBlend != static_cast<bool>(glIsEnabled(GL_BLEND)))
            return false;

        static int SourceBlendFactor;
        glGetIntegerv(GL_BLEND_SRC, &SourceBlendFactor);
        if (convert(mSourceBlendFactor) != SourceBlendFactor)
            return false;

        static int destinationBlendFactor;
        glGetIntegerv(GL_BLEND_DST, &destinationBlendFactor);
        if (convert(mDestinationBlendFactor) != destinationBlendFactor)
            return false;
    }

    {// Check cull flags match
        if (mCullFaces != static_cast<bool>(glIsEnabled(GL_CULL_FACE)))
            return false;

        static int cullFaceType;
        glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceType);
        if (convert(mCullFacesType) != cullFaceType)
            return false;

        static int frontFaceOrientation;
        glGetIntegerv(GL_FRONT_FACE, &frontFaceOrientation);
        if (convert(mFrontFaceOrientation) != frontFaceOrientation)
            return false;
    }

    {// Check clear colour
        static std::array<float, 4> clearColour;
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColour.data());
        if (clearColour != mWindowClearColour)
            return false;
    }

    { // Check polygon mode
        static int polygonMode;
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
        if (convert(mPolygonMode) != polygonMode)
            return false;
    }

    { // Check active texture unit
        static int activeTextureUnit;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureUnit);
        if (GL_TEXTURE0 + mActiveTextureUnit != activeTextureUnit)
            return false;
    }

    {// Check viewport size
        static std::array<int, 4> viewportDimensions;
        glGetIntegerv(GL_VIEWPORT, viewportDimensions.data());
        if (mViewport != viewportDimensions)
            return false;
    }

    return true;
}

// Outputs the current GLState with options to change flags.
void GLState::renderImGui()
{
    if (ImGui::ColorEdit4("Window clear colour", mWindowClearColour.data()))
        setClearColour(mWindowClearColour);

    { // Depth testing options
        if (ImGui::Checkbox("Depth test", &mDepthTest))
            toggleDepthTest(mDepthTest);

        if (mDepthTest)
        {
            ImGui::SameLine();
            if (ImGui::BeginCombo("Depth test type", GLType::toString(mDepthTestType).c_str(), ImGuiComboFlags()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::DepthTestType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::depthTestTypes[i].c_str()))
                        setDepthTestType(static_cast<GLType::DepthTestType>(i));
                }
                ImGui::EndCombo();
            }
        }
    }

    {// Blending options
        if (ImGui::Checkbox("Blending", &mBlend))
            toggleBlending(mBlend);

        if (mBlend)
        {
            ImGui::Text("Blend function:");
            ImGui::SameLine();

            const float width = ImGui::GetWindowWidth() * 0.25f;
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("Source", GLType::toString(mSourceBlendFactor).c_str()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::BlendFactorType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::blendFactorTypes[i].c_str()))
                        setBlendFunction(static_cast<GLType::BlendFactorType>(i), mDestinationBlendFactor);
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("Destination", GLType::toString(mDestinationBlendFactor).c_str(), ImGuiComboFlags()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::BlendFactorType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::blendFactorTypes[i].c_str()))
                        setBlendFunction(mSourceBlendFactor, static_cast<GLType::BlendFactorType>(i));
                }
                ImGui::EndCombo();
            }
        }

    }

    {// Cull face options
        if (ImGui::Checkbox("Cull faces", &mCullFaces))
            toggleCullFaces(mCullFaces);

        if (mCullFaces)
        {
            if (ImGui::BeginCombo("Mode", GLType::toString(mCullFacesType).c_str()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::CullFacesType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::cullFaceTypes[i].c_str()))
                        setCullFacesType(static_cast<GLType::CullFacesType>(i));
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Front face orientation", GLType::toString(mFrontFaceOrientation).c_str()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::FrontFaceOrientation::Count); i++)
                {
                    if (ImGui::Selectable(GLType::frontFaceOrientationTypes[i].c_str()))
                        setFrontFaceOrientation(static_cast<GLType::FrontFaceOrientation>(i));
                }
                ImGui::EndCombo();
            }
        }
    }
}

void GLState::toggleDepthTest(const bool& pDepthTest)
{
    mDepthTest = pDepthTest;

    if (mDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void GLState::toggleBlending(const bool& pBlend)
{
    mBlend = pBlend;

    if (mBlend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void GLState::toggleCullFaces(const bool& pCull)
{
    mCullFaces = pCull;

    if (mCullFaces)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void GLState::setDepthTestType(const GLType::DepthTestType& pType)
{
    mDepthTestType = pType;
    glDepthFunc(convert(mDepthTestType));
}

void GLState::setBlendFunction(const GLType::BlendFactorType &pSourceFactor, const GLType::BlendFactorType &pDestinationFactor)
{
    GLboolean enabled = glIsEnabled(GL_BLEND);
    ZEPHYR_ASSERT(enabled, "Blending has to be enabled to set blend function.");

    mSourceBlendFactor = pSourceFactor;
    mDestinationBlendFactor = pDestinationFactor;
    glBlendFunc(convert(mSourceBlendFactor), convert(mDestinationBlendFactor)); // It is also possible to set individual RGBA factors using glBlendFuncSeparate().

    // BlendFactors using constant require glBlendColor() to be called to set the RGBA constant values.
    ZEPHYR_ASSERT((pSourceFactor    != GLType::BlendFactorType::ConstantColour
    && pSourceFactor                != GLType::BlendFactorType::OneMinusConstantColour
    && pSourceFactor                != GLType::BlendFactorType::ConstantAlpha
    && pSourceFactor                != GLType::BlendFactorType::OneMinusConstantAlpha
    && pDestinationFactor           != GLType::BlendFactorType::ConstantColour
    && pDestinationFactor           != GLType::BlendFactorType::OneMinusConstantColour
    && pDestinationFactor           != GLType::BlendFactorType::ConstantAlpha
    && pDestinationFactor           != GLType::BlendFactorType::OneMinusConstantAlpha)
    , "Constant blend factors require glBlendColor() to set the constant. Not supported yet.");
}

void GLState::setCullFacesType(const GLType::CullFacesType& pCullFaceType)
{
    mCullFacesType = pCullFaceType;
    glCullFace(convert(mCullFacesType));
}

void GLState::setFrontFaceOrientation(const GLType::FrontFaceOrientation& pFrontFaceOrientation)
{
    mFrontFaceOrientation = pFrontFaceOrientation;
    glFrontFace(convert(mFrontFaceOrientation));
}

void GLState::setClearColour(const std::array<float, 4> &pColour)
{
    mWindowClearColour = pColour;
    glClearColor(mWindowClearColour[0], mWindowClearColour[1], mWindowClearColour[2], mWindowClearColour[3]);
}

void GLState::setPolygonMode(const GLType::PolygonMode& pPolygonMode)
{
    mPolygonMode = pPolygonMode;
    glPolygonMode(GL_FRONT_AND_BACK, GLType::convert(pPolygonMode));
}

void GLState::setActiveTextureUnit(const int& pTextureUnitPosition)
{
    mActiveTextureUnit = pTextureUnitPosition;
    glActiveTexture(GL_TEXTURE0 + pTextureUnitPosition);
    // GL_INVALID_ENUM is generated if texture is not one of GL_TEXTUREi, where i ranges from zero to the value of GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS minus one.
}

void GLState::drawElements(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount)
{
    glDrawElements(convert(pPrimitiveMode), pCount, GL_UNSIGNED_INT, 0);
    //GL_INVALID_ENUM is generated if mode is not an accepted value.
    //GL_INVALID_VALUE is generated if count is negative.
    //GL_INVALID_OPERATION is generated if a geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object.
    //GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to an enabled array or the element array and the buffer object's data store is currently mapped.
}
void GLState::drawArrays(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount)
{
    glDrawArrays(convert(pPrimitiveMode), 0, pCount);
    //GL_INVALID_ENUM is generated if mode is not an accepted value.
    //GL_INVALID_VALUE is generated if count is negative.
    //GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to an enabled array and the buffer object's data store is currently mapped.
    //GL_INVALID_OPERATION is generated if a geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object.
}

void GLState::bindFramebuffer(const GLType::FramebufferTarget& pFramebufferTargetType, const unsigned int& pFBOHandle)
{
    mActiveFramebuffer = pFBOHandle;
    glBindFramebuffer(convert(pFramebufferTargetType), pFBOHandle);
    //GL_INVALID_ENUM is generated if target is not GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER or GL_FRAMEBUFFER.
    //GL_INVALID_OPERATION is generated if framebuffer is not zero or the name of a framebuffer previously returned from a call to glGenFramebuffers.
}
void GLState::unbindFramebuffer()
{
    mActiveFramebuffer = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GLState::checkFramebufferBufferComplete()
{
    ZEPHYR_ASSERT(mActiveFramebuffer != 0, "Checking default framebuffer. Default FBO is always valid.");
    ZEPHYR_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Currently bound FBO not complete, have you called attachColourBuffer and/or attachDepthBuffer");
}

void GLState::setViewport(const int& pWidth, const int& pHeight)
{
    mViewport[2] = pWidth;
    mViewport[3] = pHeight;
    glViewport(0, 0, pWidth, pHeight);
    //GL_INVALID_VALUE is generated if either width or height is negative.
}

namespace GLData
{
    void VAO::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated VAO")
        glGenVertexArrays(1, &mHandle);
        mInitialised = true;
    }
    void VAO::bind() const
    {
        ZEPHYR_ASSERT(mInitialised, "VAO has not been generated before bind, call generate before bind");
        glBindVertexArray(mHandle);
    }
    void VAO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised VAO");
        glDeleteVertexArrays(1, &mHandle);
        mInitialised = false;
    }
    void VBO::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated VBO")
        glGenBuffers(1, &mHandle);
        mInitialised = true;
    }
    void VBO::bind() const
    {
        ZEPHYR_ASSERT(mInitialised, "VBO has not been generated before bind, call generate before bind");
        glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    }
    void VBO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised VBO");
        glDeleteBuffers(1, &mHandle);
        mInitialised = false;
    }
    void VBO::pushData(const std::vector<float>& pData, const int& attributeIndex, const int& attributeSize)
    {
        ZEPHYR_ASSERT(mInitialised, "Initialise VBO before calling pushData.");

        glBufferData(GL_ARRAY_BUFFER, pData.size() * sizeof(float), &pData.front(), GL_STATIC_DRAW);

        // const GLint attributeIndex = static_cast<GLint>(Shader::getAttributeLocation(pAttribute));
        // const GLint attributeComponentCount = static_cast<GLint>(Shader::getAttributeComponentCount(pAttribute));
        glVertexAttribPointer(attributeIndex, attributeSize, GL_FLOAT, GL_FALSE, attributeSize * sizeof(float), (void *)0);
        glEnableVertexAttribArray(attributeIndex);
    }
    void EBO::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated EBO")
        glGenBuffers(1, &mHandle);
        mInitialised = true;
    }
    void EBO::bind() const
    {
        ZEPHYR_ASSERT(mInitialised, "EBO has not been generated before bind, call generate before bind");
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
    }
    void EBO::pushData(const std::vector<int>& pIndexData)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pIndexData.size() * sizeof(int), &pIndexData.front(), GL_STATIC_DRAW);
    }
    void EBO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised EBO");
        glDeleteBuffers(1, &mHandle);
        mInitialised = false;
    }
    void RBO::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated RBO")
        glGenRenderbuffers(1, &mHandle);
        mInitialised = true;
    }
    void RBO::bind() const
    {
        ZEPHYR_ASSERT(mInitialised, "RBO has not been generated before bind, call generate before bind");
        glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
    }
    void RBO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised RBO");
        glDeleteRenderbuffers(1, &mHandle);
        mInitialised = false;
    }
    void FBO::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated FBO")
        glGenFramebuffers(1, &mHandle);
        mInitialised = true;
    }
    void FBO::bind(GLState& pGLState) const
    {
        ZEPHYR_ASSERT(mInitialised, "FBO has not been generated before bind, call generate before bind");
        pGLState.bindFramebuffer(GLType::FramebufferTarget::Framebuffer, mHandle);
    }
    void FBO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised FBO");
        glDeleteFramebuffers(1, &mHandle);

        if (mColourAttachment.has_value())
            mColourAttachment->release();
        if (mDepthAttachment.has_value())
            mDepthAttachment->release();

        mInitialised = false;
    }
    Texture& FBO::getColourTexture()
    {
        ZEPHYR_ASSERT(mInitialised, "Attempting to get texture handle on uninitialised FBO");
        ZEPHYR_ASSERT(mColourAttachment.has_value(), "Attempting to get texture on FBO with no attached texture");
        ZEPHYR_ASSERT(mColourAttachment->mInitialised, "Attempting to get uninitialised texture of FBO");

        return mColourAttachment.value();
    }
    void FBO::clearBuffers()
    {
        glClear(mBufferClearBitField);
    }
    void FBO::resize(const int& pWidth, const int& pHeight, GLState& pGLState)
    {
        if (mColourAttachment.has_value())
        {
            detachColourBuffer();
            attachColourBuffer(pWidth, pHeight, pGLState);
        }
        if (mDepthAttachment.has_value())
        {
            detachDepthBuffer();
            attachDepthBuffer(pWidth, pHeight, pGLState);
        }
    }

    void FBO::attachColourBuffer(const int& pWidth, const int& pHeight, GLState& pGLState)
    {
        ZEPHYR_ASSERT(mInitialised, "Must initialise FBO before attaching texture");
        ZEPHYR_ASSERT(!mColourAttachment.has_value(), "FBO already has an attached texture");

        bind(pGLState);
        mColourAttachment = { Texture::Type::Texture2D };
        mColourAttachment->generate();
        mColourAttachment->bind();

        {// Attaching a colour output texture to FBO
            // Data param is passed as NULL - we're only allocating memory and filling the texture when we render to the FBO.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pWidth, pHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColourAttachment->getHandle(), 0);
        }

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // GL_NEAREST so that we don't interpolate multiple samples from the intermediate texture to the final screen render.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void FBO::detachColourBuffer()
    {
        ZEPHYR_ASSERT(mColourAttachment.has_value(), "There is no attached texture to remove from FBO");
        mColourAttachment->release();
        mColourAttachment.reset();
        mBufferClearBitField &= ~GL_COLOR_BUFFER_BIT;
    }
    void FBO::attachDepthBuffer(const int& pWidth, const int& pHeight, GLState& pGLState)
    {
        ZEPHYR_ASSERT(mInitialised, "Must initialise FBO before attaching buffer");
        ZEPHYR_ASSERT(!mDepthAttachment.has_value(), "FBO already has an attached buffer");

        bind(pGLState);
        mDepthAttachment = RBO();
        mDepthAttachment->generate();
        mDepthAttachment->bind();

        // Allocate the storage for the buffer then unbind it to make sure we're not accidentally rendering to the wrong framebuffer.
        // Lastly attach it to this FBO
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pWidth, pHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthAttachment->getHandle());
        //glBindRenderbuffer(GL_RENDERBUFFER, 0);

        mBufferClearBitField |= GL_DEPTH_BUFFER_BIT;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void FBO::detachDepthBuffer()
    {
        ZEPHYR_ASSERT(mDepthAttachment.has_value(), "There is no attached RBO to remove from FBO");
        mDepthAttachment->release();
        mDepthAttachment.reset();
        mBufferClearBitField &= ~GL_DEPTH_BUFFER_BIT;
    }
    void Texture::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated Texture")
        glGenTextures(1, &mHandle);
        mInitialised = true;
    }
    void Texture::bind() const
    {
        ZEPHYR_ASSERT(mInitialised, "Texture has not been generated before bind, call generate before bind.");

        if (mType == Type::Texture2D)
            glBindTexture(GL_TEXTURE_2D, mHandle);
        else if (mType == Type::CubeMap)
            glBindTexture(GL_TEXTURE_CUBE_MAP, mHandle);
    }
    void Texture::pushData(const int& pWidth, const int& pHeight, const int& pNumberOfChannels, const unsigned char* pData, const int& pCubeMapIndexOffset /*= -1*/)
	{
        ZEPHYR_ASSERT(pData, "Invalid Texture data.");

        GLenum format = 0;
        if (pNumberOfChannels == 1)      format = GL_RED;
        else if (pNumberOfChannels == 3) format = GL_RGB;
        else if (pNumberOfChannels == 4) format = GL_RGBA;
        ZEPHYR_ASSERT(format != 0, "Could not find channel type for this number of texture channels");

        if (pCubeMapIndexOffset == -1)
        {
            ZEPHYR_ASSERT(mType == Type::Texture2D, "Trying to push Texture 2D data to non Texture 2D object.");
            glTexImage2D(GL_TEXTURE_2D, 0, format, pWidth, pHeight, 0, format, GL_UNSIGNED_BYTE, pData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // GL_REPEAT - (default wrapping method)
            // GL_CLAMP_TO_EDGE - when using transparency to stop interpolation at borders causing semi-transparent artifacts.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            ZEPHYR_ASSERT(mType == Type::CubeMap, "Trying to push CubeMap data to non-CubeMap object.");
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + pCubeMapIndexOffset, 0, format, pWidth, pHeight, 0, format, GL_UNSIGNED_BYTE, pData);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
    }
    void Texture::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised Texture");
        glDeleteTextures(1, &mHandle);
        mInitialised = false;
    }
}

namespace GLType
{
    int convert(const DepthTestType& pDepthTestType)
    {
    	switch (pDepthTestType)
    	{
            case DepthTestType::Always:		  return GL_ALWAYS;
	        case DepthTestType::Never:		  return GL_NEVER;
	        case DepthTestType::Less:		  return GL_LESS;
	        case DepthTestType::Equal:		  return GL_EQUAL;
	        case DepthTestType::LessEqual:	  return GL_LEQUAL;
	        case DepthTestType::Greater:	  return GL_GREATER;
	        case DepthTestType::NotEqual:	  return GL_NOTEQUAL;
	        case DepthTestType::GreaterEqual: return GL_GEQUAL;

            case DepthTestType::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown DepthTestType requested");
                return 0;
        }
    }
    int convert(const BlendFactorType& pBlendFactorType)
    {
    	switch (pBlendFactorType)
    	{
    		case BlendFactorType::Zero: 					 return GL_ZERO;
    		case BlendFactorType::One:						 return GL_ONE;
    		case BlendFactorType::SourceColour:				 return GL_SRC_COLOR;
    		case BlendFactorType::OneMinusSourceColour:		 return GL_ONE_MINUS_SRC_COLOR;
    		case BlendFactorType::DestinationColour:		 return GL_DST_COLOR;
    		case BlendFactorType::OneMinusDestinationColour: return GL_ONE_MINUS_DST_COLOR;
    		case BlendFactorType::SourceAlpha:				 return GL_SRC_ALPHA;
    		case BlendFactorType::OneMinusSourceAlpha:		 return GL_ONE_MINUS_SRC_ALPHA;
    		case BlendFactorType::DestinationAlpha:			 return GL_DST_ALPHA;
    		case BlendFactorType::OneMinusDestinationAlpha:  return GL_ONE_MINUS_DST_ALPHA;
    		case BlendFactorType::ConstantColour:	 		 return GL_CONSTANT_COLOR;
    		case BlendFactorType::OneMinusConstantColour: 	 return GL_ONE_MINUS_CONSTANT_COLOR;
    		case BlendFactorType::ConstantAlpha:		 	 return GL_CONSTANT_ALPHA;
    		case BlendFactorType::OneMinusConstantAlpha: 	 return GL_ONE_MINUS_CONSTANT_ALPHA;

            case BlendFactorType::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown BlendFactorType requested");
                return 0;
        }
    }
    int convert(const CullFacesType& pCullFacesType)
    {
        switch (pCullFacesType)
        {
            case CullFacesType::Back:         return GL_BACK;
            case CullFacesType::Front:        return GL_FRONT;
            case CullFacesType::FrontAndBack: return GL_FRONT_AND_BACK;

            case CullFacesType::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown CullFacesType requested");
                return 0;
        }
    }
    int convert(const FrontFaceOrientation& pFrontFaceOrientation)
    {
        switch (pFrontFaceOrientation)
        {
            case FrontFaceOrientation::Clockwise:        return GL_CW;
            case FrontFaceOrientation::CounterClockwise: return GL_CCW;

            case FrontFaceOrientation::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown FrontFaceOrientation requested");
                return 0;
        }
    }
    int convert(const PolygonMode& pPolygonMode)
    {
        switch (pPolygonMode)
        {
            case PolygonMode::Point: return GL_POINT;
            case PolygonMode::Line:  return GL_LINE;
            case PolygonMode::Fill:  return GL_FILL;
            case PolygonMode::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown PolygonMode requested");
                return 0;
        }
    }
    int convert(const PrimitiveMode& pPrimitiveMode)
    {
        switch (pPrimitiveMode)
        {
            case PrimitiveMode::Points:                 return GL_POINTS;
            case PrimitiveMode::LineStrip:              return GL_LINE_STRIP;
            case PrimitiveMode::LineLoop:               return GL_LINE_LOOP;
            case PrimitiveMode::Lines:                  return GL_LINES;
            case PrimitiveMode::LineStripAdjacency:     return GL_LINE_STRIP_ADJACENCY;
            case PrimitiveMode::LinesAdjacency:         return GL_LINES_ADJACENCY;
            case PrimitiveMode::TriangleStrip:          return GL_TRIANGLE_STRIP;
            case PrimitiveMode::TriangleFan:            return GL_TRIANGLE_FAN;
            case PrimitiveMode::Triangles:              return GL_TRIANGLES;
            case PrimitiveMode::TriangleStripAdjacency: return GL_TRIANGLE_STRIP_ADJACENCY;
            case PrimitiveMode::TrianglesAdjacency:     return GL_TRIANGLES_ADJACENCY;
            case PrimitiveMode::Patches:                return GL_PATCHES;
            case PrimitiveMode::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown PrimitiveMode requested");
                return 0;
        }
    }

    int convert(const FramebufferTarget& pFramebufferTarget)
    {
        switch (pFramebufferTarget)
        {
            case FramebufferTarget::DrawFramebuffer: return GL_DRAW_FRAMEBUFFER;
            case FramebufferTarget::ReadFramebuffer: return GL_READ_FRAMEBUFFER;
            case FramebufferTarget::Framebuffer:     return GL_FRAMEBUFFER;
            case FramebufferTarget::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown FramebufferTarget requested");
                return 0;
        }
    }
}