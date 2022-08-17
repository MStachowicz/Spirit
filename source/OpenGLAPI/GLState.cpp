#include "GLState.hpp"

#include "Utility.hpp"

// External libs
#include "glm/mat4x4.hpp" // mat4, dmat4
#include "glm/gtc/type_ptr.hpp" //  glm::value_ptr
#include "glad/gl.h"
#include "imgui.h"

#include <set>
#include <unordered_map>
#include <algorithm>
#include <iterator>

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
                static const size_t depthTestEnumCount = 8; // This has to match the number of enums in GLType::DepthTestType
                for (size_t i = 0; i < depthTestEnumCount; i++)
                {
                    const auto depthTestType = static_cast<GLType::DepthTestType>(i);
                    if (ImGui::Selectable(GLType::toString(depthTestType).c_str()))
                        setDepthTestType(depthTestType);
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
            static const size_t blendFactorEnumCount = 14; // This has to match the number of enums in GLType::BlendFactorType

            const float width = ImGui::GetWindowWidth() * 0.25f;
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("Source", GLType::toString(mSourceBlendFactor).c_str()))
            {
                for (size_t i = 0; i < blendFactorEnumCount; i++)
                {
                    const auto blendFactorType = static_cast<GLType::BlendFactorType>(i);
                    if (ImGui::Selectable(GLType::toString(blendFactorType).c_str()))
                        setBlendFunction(blendFactorType, mDestinationBlendFactor);
                }

                ImGui::EndCombo();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("Destination", GLType::toString(mDestinationBlendFactor).c_str(), ImGuiComboFlags()))
            {
                for (size_t i = 0; i < blendFactorEnumCount; i++)
                {
                    const auto blendFactorType = static_cast<GLType::BlendFactorType>(i);
                    if (ImGui::Selectable(GLType::toString(blendFactorType).c_str()))
                        setBlendFunction(mSourceBlendFactor, blendFactorType);
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
                static const size_t frontFaceEnumCount = 3; // This has to match the number of enums in GLType::FrontFaceOrientation
                for (size_t i = 0; i < frontFaceEnumCount; i++)
                {
                    const auto cullFaceType = static_cast<GLType::CullFacesType>(i);
                    if (ImGui::Selectable(GLType::toString(cullFaceType).c_str()))
                        setCullFacesType(cullFaceType);
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Front face orientation", GLType::toString(mFrontFaceOrientation).c_str()))
            {
                static const size_t frontFaceEnumCount = 2; // This has to match the number of enums in GLType::FrontFaceOrientation
                for (size_t i = 0; i < frontFaceEnumCount; i++)
                {
                    const auto frontFace = static_cast<GLType::FrontFaceOrientation>(i);
                    if (ImGui::Selectable(GLType::toString(frontFace).c_str()))
                        setFrontFaceOrientation(frontFace);
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

void GLState::drawArrays(const GLType::PrimitiveMode& pPrimitiveMode, const int& pArraySize)
{
    glDrawArrays(convert(pPrimitiveMode), 0, pArraySize);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DrawArrays));
}
void GLState::drawArraysInstanced(const GLType::PrimitiveMode& pPrimitiveMode, const int& pArraySize, const int& pInstanceCount)
{
	glDrawArraysInstanced(GLType::convert(pPrimitiveMode), 0, pArraySize, pInstanceCount);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DrawArraysInstanced));
}
void GLState::drawElements(const GLType::PrimitiveMode& pPrimitiveMode, const int& pElementsSize)
{
    glDrawElements(GLType::convert(pPrimitiveMode), pElementsSize, GL_UNSIGNED_INT, 0);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DrawElements));
}
void GLState::drawElementsInstanced(const GLType::PrimitiveMode& pPrimitiveMode, const int& pElementsSize, const int& pInstanceCount)
{
	glDrawElementsInstanced(GLType::convert(pPrimitiveMode), pElementsSize, GL_UNSIGNED_INT, 0, pInstanceCount);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DrawElementsInstanced));
}

void GLState::bindFramebuffer(const GLType::FramebufferTarget& pFramebufferTargetType, const unsigned int& pFBOHandle)
{
    mActiveFramebuffer = pFBOHandle;
    glBindFramebuffer(convert(pFramebufferTargetType), pFBOHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::BindFramebuffer));
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

unsigned int GLState::GenBuffers() const
{
    unsigned int bufferHandle;
    glGenBuffers(1, &bufferHandle);
    return bufferHandle;
}

void GLState::BindBuffer(const GLType::BufferType& pBufferType, const unsigned int& pBufferHandle) const
{
    glBindBuffer(convert(pBufferType), pBufferHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::BindBuffer));
}

void GLState::DeleteBuffer(const unsigned int& pBufferHandle) const
{
    glDeleteBuffers(1, &pBufferHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DeleteBuffer));
}

unsigned int GLState::CreateShader(const GLType::ShaderProgramType& pProgramType)
{
    unsigned int shaderID = glCreateShader(GLType::convert(pProgramType));
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::CreateShader));
    ZEPHYR_ASSERT(shaderID != 0, "Error occurred creating the shader object");
    return shaderID;
}

void GLState::ShaderSource(const unsigned int& pShaderHandle, const std::string& pShaderSource)
{
    const char* programSource = pShaderSource.c_str();
    glShaderSource(pShaderHandle, 1, &programSource, NULL);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::ShaderSource));
}

void GLState::CompileShader(const unsigned int& pShaderHandle)
{
    glCompileShader(pShaderHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::CompileShader));

    GLint success;
    glGetShaderiv(pShaderHandle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint infoLogCharCount = 0;
        glGetShaderiv(pShaderHandle, GL_INFO_LOG_LENGTH, &infoLogCharCount);
        std::string infoLog = "";
        infoLog.resize(infoLogCharCount);
        glGetShaderInfoLog(pShaderHandle, infoLogCharCount, NULL, infoLog.data());
        infoLog.pop_back();
        ZEPHYR_ASSERT(false, "Shader compilation failed\n{}", infoLog);
    }
}

unsigned int GLState::CreateProgram()
{
    unsigned int programHandle = glCreateProgram();
    ZEPHYR_ASSERT(programHandle != 0, "Error occurred creating the shader program object");
    return programHandle;
}

void GLState::AttachShader(const unsigned int& pShaderProgramHandle, const unsigned int& pShaderHandle)
{
    glAttachShader(pShaderProgramHandle, pShaderHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::AttachShader));
}

void GLState::LinkProgram(const unsigned int& pShaderProgramHandle)
{
    // If program contains shader objects of type GL_VERTEX_SHADER, and optionally of type GL_GEOMETRY_SHADER, but does not contain shader objects of type GL_FRAGMENT_SHADER,
    // the vertex shader executable will be installed on the programmable vertex processor,
    // the geometry shader executable, if present, will be installed on the programmable geometry processor,
    // but no executable will be installed on the fragment processor.
    // The results of rasterizing primitives with such a program will be UNDEFINED.
    glLinkProgram(pShaderProgramHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::LinkProgram));

    GLint success;
    glGetProgramiv(pShaderProgramHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint infoLogCharCount = 0;
        glGetShaderiv(pShaderProgramHandle, GL_INFO_LOG_LENGTH, &infoLogCharCount);
        std::string infoLog = "";
        infoLog.resize(infoLogCharCount);
        glGetShaderInfoLog(pShaderProgramHandle, infoLogCharCount, NULL, infoLog.data());
        infoLog.pop_back();
        ZEPHYR_ASSERT(false, "Shader program linking failed\n{}", infoLog);
    }
}

void GLState::DeleteShader(const unsigned int& pShaderHandle)
{
    glDeleteShader(pShaderHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DeleteShader));
}

void GLState::UseProgram(const unsigned int& pShaderProgramHandle)
{
    glUseProgram(pShaderProgramHandle);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::UseProgram));
}

void GLState::RegisterUniformBlock(GLData::UniformBlock& pUniformBlock)
{
    GLData::UniformBlockBindingPoint* bindingPoint = nullptr;

    // #C++20 Convert to std::contains on vector.
    const auto it = std::find_if(mUniformBlockBindingPoints.begin(), mUniformBlockBindingPoints.end(), [&pUniformBlock](const GLData::UniformBlockBindingPoint& bindingPoint)
    { return bindingPoint.mName == pUniformBlock.mName; });

    if (it == mUniformBlockBindingPoints.end())
    {
        // If there is no binding point for this UniformBlock create it and its UBO.
        // This allows the uniform variables inside the block to be set using setUniformBlockVariable()
        // This makes the uniform block share resource with all other matching blocks as long as they use the same mBindingPoint and have matching interfaces.
        mUniformBlockBindingPoints.push_back(GLData::UniformBlockBindingPoint(*this, pUniformBlock, static_cast<unsigned int>(mUniformBlockBindingPoints.size())));
        bindingPoint = &mUniformBlockBindingPoints.back();

        bindingPoint->mUBO->Bind(*this);
        bindingPoint->mUBO->Reserve(*this, pUniformBlock.mBufferDataSize);
        bindingPoint->mUBO->AssignBindingPoint(*this, bindingPoint->mBindingPoint);
    }
    else
    {
        // If the UniformBlock has been encountered before. We bind it to the same mBindingPoint of the previously
        // created UniformBlockBufferBacking meaning the blocks share the GPU memory.
        bindingPoint = &(*it);
        bindingPoint->BindUniformBlock(pUniformBlock);
        bindingPoint->mUBO->Bind(*this);
    }

    ZEPHYR_ASSERT(bindingPoint != nullptr, "Could not find a valid binding point for GLSL Uniform Block '{}'", pUniformBlock.mName);
    pUniformBlock.mBindingPoint = bindingPoint->mBindingPoint;
    UniformBlockBinding(pUniformBlock.mParentShaderHandle, pUniformBlock.mBlockIndex, bindingPoint->mBindingPoint);
}

void GLState::RegisterShaderStorageBlock(GLData::ShaderStorageBlock& pShaderStorageBlock)
{
    GLData::ShaderStorageBlockBindingPoint* bindingPoint = nullptr;

    // #C++20 Convert to std::contains on vector.
    const auto it = std::find_if(mShaderStorageBlockBindingPoints.begin(), mShaderStorageBlockBindingPoints.end(), [&pShaderStorageBlock](const GLData::ShaderStorageBlockBindingPoint& bindingPoint)
    { return bindingPoint.mName == pShaderStorageBlock.mName; });

    if (it == mShaderStorageBlockBindingPoints.end())
    {
        // If there is no binding point for this ShaderBufferBlock create it and its UBO.
        // This allows the ShaderBufferBlockVariable's inside the block to be set using setShaderStorageBlockVariable()
        // This makes the uniform block share resource with all other matching blocks as long as they use the same mBindingPoint and have matching interfaces.
        mShaderStorageBlockBindingPoints.push_back(GLData::ShaderStorageBlockBindingPoint(*this, pShaderStorageBlock, static_cast<unsigned int>(mShaderStorageBlockBindingPoints.size())));
        bindingPoint = &mShaderStorageBlockBindingPoints.back();

        // Reserve the size of the ShaderStorageBlock in the GPU memory
        bindingPoint->mSSBO->Bind(*this);
        bindingPoint->mSSBO->Reserve(*this, pShaderStorageBlock.mBufferDataSize);
        bindingPoint->mSSBO->AssignBindingPoint(*this, bindingPoint->mBindingPoint);
    }
    else
    {
        // If the StorageBlock has been encountered before. We bind it to the same mBindingPoint of the previously
        // created ShaderStorageBlockBindingPoint meaning the blocks share the GPU memory.
        bindingPoint = &(*it);
        bindingPoint->mInstances++;
        bindingPoint->mSSBO->Bind(*this);
    }

    ZEPHYR_ASSERT(bindingPoint != nullptr, "Could not find a valid binding point for shader buffer block '{}'", pShaderStorageBlock.mName);
    pShaderStorageBlock.mBindingPoint = bindingPoint->mBindingPoint;
    ShaderStorageBlockBinding(pShaderStorageBlock.mParentShaderHandle, pShaderStorageBlock.mBlockIndex, bindingPoint->mBindingPoint);
}

int GLState::getShaderStorageBlockCount(const unsigned int& pShaderProgramHandle) const
{
    GLint blockCount = 0;
    glGetProgramInterfaceiv(pShaderProgramHandle, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &blockCount);
    ZEPHYR_ASSERT_MSG(getErrorMessage());
    return blockCount;
}

int GLState::getActiveUniformBlockCount(const unsigned int& pShaderProgramHandle) const
{
    GLint blockCount = 0;
    glGetProgramInterfaceiv(pShaderProgramHandle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &blockCount);
    return blockCount;
}

int GLState::getActiveUniformCount(const unsigned int& pShaderProgramHandle) const
{
   GLint uniformCount = 0;
   glGetProgramInterfaceiv(pShaderProgramHandle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);
   return uniformCount;
}

GLData::UniformBlock GLState::getUniformBlock(const unsigned int& pShaderProgramHandle, const unsigned int& pUniformBlockIndex) const
{
    static const std::array<GLenum, 4> propertyQuery = {GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};

	std::array<GLint, propertyQuery.size()> uniformBlockValues = {0};
    glGetProgramResourceiv(pShaderProgramHandle, GL_UNIFORM_BLOCK, pUniformBlockIndex, static_cast<GLsizei>(propertyQuery.size()), propertyQuery.data(),  static_cast<GLsizei>(uniformBlockValues.size()), NULL, uniformBlockValues.data());

    GLData::UniformBlock uniformBlock;
    { // Get the name of the uniform block
        uniformBlock.mName.resize(uniformBlockValues[0]);
        glGetProgramResourceName(pShaderProgramHandle, GL_UNIFORM_BLOCK, pUniformBlockIndex, uniformBlockValues[0], NULL, uniformBlock.mName.data());
        ZEPHYR_ASSERT(!uniformBlock.mName.empty(), "Failed to get name of uniform block in shader with handle {}", pShaderProgramHandle);
        uniformBlock.mName.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.
    }
    uniformBlock.mBlockIndex           = glGetProgramResourceIndex(pShaderProgramHandle, GL_UNIFORM_BLOCK, uniformBlock.mName.c_str());
    uniformBlock.mActiveVariablesCount = uniformBlockValues[1];
    uniformBlock.mBindingPoint         = static_cast<unsigned int>(uniformBlockValues[2]);
    uniformBlock.mBufferDataSize       = uniformBlockValues[3];
    uniformBlock.mParentShaderHandle   = pShaderProgramHandle;

    // TODO: check There is also a limitation on the available storage per uniform buffer.
	// This is queried through GL_MAX_UNIFORM_BLOCK_SIZE. This is in basic machine units (ie: bytes).
    if (uniformBlock.mActiveVariablesCount > 0)
    {
        // Get the array of active variable indices associated with the uniform block. (GL_ACTIVE_VARIABLES)
        // The indices correspond in size to GL_NUM_ACTIVE_VARIABLES
        uniformBlock.mVariableIndices.resize(uniformBlock.mActiveVariablesCount);
        const GLenum activeUnifProp[1] = {GL_ACTIVE_VARIABLES};
        glGetProgramResourceiv(pShaderProgramHandle, GL_UNIFORM_BLOCK, uniformBlock.mBlockIndex, 1, activeUnifProp, uniformBlock.mActiveVariablesCount, NULL, uniformBlock.mVariableIndices.data());

        for (int variableIndex = 0; variableIndex < uniformBlock.mActiveVariablesCount; variableIndex++)
            uniformBlock.mVariables.push_back(GLData::UniformBlockVariable(pShaderProgramHandle, uniformBlock.mVariableIndices[variableIndex]));
    }

    return uniformBlock;
}

GLData::ShaderStorageBlock GLState::getShaderStorageBlock(const unsigned int& pShaderProgramHandle, const unsigned int& pShaderBufferBlockIndex) const
{
    static const std::array<GLenum, 4> propertyQuery = {GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};

	std::array<GLint, propertyQuery.size()> bufferBlockValues = {0};
    glGetProgramResourceiv(pShaderProgramHandle, GL_SHADER_STORAGE_BLOCK, pShaderBufferBlockIndex, static_cast<GLsizei>(propertyQuery.size()), propertyQuery.data(),  static_cast<GLsizei>(bufferBlockValues.size()), NULL, bufferBlockValues.data());

    GLData::ShaderStorageBlock shaderStorageBlock;
    { // Get the name of the shader storage block
        shaderStorageBlock.mName.resize(bufferBlockValues[0]);
        glGetProgramResourceName(pShaderProgramHandle, GL_SHADER_STORAGE_BLOCK, pShaderBufferBlockIndex, bufferBlockValues[0], NULL, shaderStorageBlock.mName.data());
        ZEPHYR_ASSERT(!shaderStorageBlock.mName.empty(), "Failed to get name of shader storage block in shader with handle {}", pShaderProgramHandle);
        shaderStorageBlock.mName.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.
    }

    shaderStorageBlock.mBlockIndex           = glGetProgramResourceIndex(pShaderProgramHandle, GL_SHADER_STORAGE_BLOCK, shaderStorageBlock.mName.c_str());
    shaderStorageBlock.mActiveVariablesCount = bufferBlockValues[1];
    shaderStorageBlock.mBindingPoint         = static_cast<unsigned int>(bufferBlockValues[2]);
    shaderStorageBlock.mBufferDataSize       = bufferBlockValues[3];
    shaderStorageBlock.mParentShaderHandle   = pShaderProgramHandle;

    if (shaderStorageBlock.mActiveVariablesCount > 0)
    {
        // Get the array of active variable indices associated with the uniform block. (GL_ACTIVE_VARIABLES)
        // The indices correspond in size to GL_NUM_ACTIVE_VARIABLES
        shaderStorageBlock.mVariableIndices.resize(shaderStorageBlock.mActiveVariablesCount);
        const GLenum activeUnifProp[1] = {GL_ACTIVE_VARIABLES};
        glGetProgramResourceiv(pShaderProgramHandle, GL_SHADER_STORAGE_BLOCK, shaderStorageBlock.mBlockIndex, 1, activeUnifProp, shaderStorageBlock.mActiveVariablesCount, NULL, shaderStorageBlock.mVariableIndices.data());

        for (int i = 0; i < shaderStorageBlock.mActiveVariablesCount; i++)
            shaderStorageBlock.mVariables.push_back(GLData::ShaderStorageBlockVariable(pShaderProgramHandle, shaderStorageBlock.mVariableIndices[i]));
    }

    return shaderStorageBlock;
}


void GLState::setViewport(const int& pWidth, const int& pHeight)
{
    mViewport[2] = pWidth;
    mViewport[3] = pHeight;
    glViewport(0, 0, pWidth, pHeight);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::Viewport));
}

void GLState::BufferData(const GLType::BufferType& pBufferType, const size_t& pSizeInBytes, const void* pData, const GLType::BufferUsage& pBufferUsage)
{
    glBufferData(convert(pBufferType), pSizeInBytes, pData, convert(pBufferUsage));
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::BufferData));
}

void GLState::BufferSubData(const GLType::BufferType& pBufferType, const size_t& pOffset, const size_t& pSizeInBytes, const void* pData)
{
    glBufferSubData(convert(pBufferType), pOffset, pSizeInBytes, pData);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::BufferSubData));
}

void GLState::CopyBufferSubData(const GLType::BufferType& pSourceTarget, const GLType::BufferType& pDestinationTarget, const long long int& pSourceOffset, const long long int& pDestinationOffset, const long long int& pSize)
{
    glCopyBufferSubData(GLType::convert(pSourceTarget), GLType::convert(pDestinationTarget), pSourceOffset, pDestinationOffset, pSize);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::CopyBufferSubData));
}

void GLState::BindBufferRange(const GLType::BufferType& pType, const unsigned int& pBufferHandle, const unsigned int& pBindingPoint, const unsigned int& pOffset, const size_t& pBindSizeInBytes)
{
    glBindBufferRange(convert(pType), pBindingPoint, pBufferHandle, pOffset, pBindSizeInBytes);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::BindBufferRange));
}

void GLState::UniformBlockBinding(const unsigned int& pShaderHandle, const unsigned int& pUniformBlockIndexShader, const unsigned int& pBindingPoint)
{
    glUniformBlockBinding(pShaderHandle, pUniformBlockIndexShader, pBindingPoint);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::UniformBlockBinding));
}

void GLState::ShaderStorageBlockBinding(const unsigned int& pShaderHandle, const unsigned int& pShaderStorageBlockIndexShader, const unsigned int& pBindingPoint)
{
    glShaderStorageBlockBinding(pShaderHandle, pShaderStorageBlockIndexShader, pBindingPoint);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::ShaderStorageBlockBinding));
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

    Buffer::Buffer(const GLState& pGLState, const GLType::BufferType& pType, const GLType::BufferUsage& pUsage)
        : mType(pType)
        , mUsage(pUsage)
        , mHandle(pGLState.GenBuffers())
        , mReservedSize(0)
        , mUsedSize(0)
    {}

    void Buffer::Bind(const GLState& pGLState) const
    {
        pGLState.BindBuffer(mType, mHandle);
    }
    void Buffer::Delete(const GLState& pGLState)
    {
        pGLState.DeleteBuffer(mHandle);
    }
    void Buffer::Reserve(GLState& pGLState, const size_t& pBytesToReserve)
    {
        ZEPHYR_ASSERT(mType == GLType::BufferType::ShaderStorageBuffer || mReservedSize == 0, "Reserving buffer memory on an already reserved buffer.");
        mReservedSize = pBytesToReserve;
        pGLState.BufferData(mType, mReservedSize, NULL, mUsage); // Supplying NULL as Data to BufferData reserves the Bytes but does not assign to them.
    }
    void Buffer::PushData(GLState& pGLState, const std::vector<int>& pData)
    {
        mUsedSize += pData.size() * sizeof(int);
        if (mReservedSize == 0)
            mReservedSize = mUsedSize;

        ZEPHYR_ASSERT(mUsedSize <= mReservedSize, "Attempting to push more bytes of data than allocated to the Buffer.");
        pGLState.BufferData(mType, mUsedSize, &pData.front(), mUsage);
    }
    void Buffer::PushData(GLState& pGLState, const std::vector<float>& pData)
    {
        mUsedSize += pData.size() * sizeof(float);
        if (mReservedSize == 0)
            mReservedSize = mUsedSize;

        ZEPHYR_ASSERT(mUsedSize <= mReservedSize, "Attempting to push more bytes of data than allocated to the Buffer.");
        pGLState.BufferData(mType, mUsedSize, &pData.front(), mUsage);
    }

    UniformVariable::UniformVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pVariableIndex)
        : GLSLVariable(GLType::GLSLVariableType::Uniform)
    {
        // Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
        // https://www.khronos.org/opengl/wiki/Program_Introspection
        static const std::array<GLenum, 9> propertyQuery = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_LOCATION, GL_BLOCK_INDEX, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR};
        std::array<GLint, propertyQuery.size()> propertyValues = {0};
        glGetProgramResourceiv(pShaderProgramHandle, GL_UNIFORM, pVariableIndex, static_cast<GLsizei>(propertyQuery.size()), propertyQuery.data(), static_cast<GLsizei>(propertyValues.size()), NULL, propertyValues.data());

        mName.emplace();
        mName->resize(propertyValues[0]);
        glGetProgramResourceName(pShaderProgramHandle, GL_UNIFORM, pVariableIndex, propertyValues[0], NULL, mName->data());
        ZEPHYR_ASSERT(!mName->empty(), "Failed to get name of uniform variable in shader with handle {}", pShaderProgramHandle);
        mName->pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        mDataType     = GLType::convert(propertyValues[1]);
        mOffset       = propertyValues[2];
        mLocation     = propertyValues[3];
        mBlockIndex   = propertyValues[4];
        mArraySize    = propertyValues[5];
        mArrayStride  = propertyValues[6];
        mMatrixStride = propertyValues[7];
        mIsRowMajor   = propertyValues[8];
    }

    UniformBlockVariable::UniformBlockVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pVariableIndex)
        : GLSLVariable(GLType::GLSLVariableType::UniformBlock)
    {
        // Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
        // https://www.khronos.org/opengl/wiki/Program_Introspection
        static const std::array<GLenum, 8> propertyQuery = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR};
        std::array<GLint, propertyQuery.size()> propertyValues = {0};
        glGetProgramResourceiv(pShaderProgramHandle, GL_UNIFORM, pVariableIndex, static_cast<GLsizei>(propertyQuery.size()), propertyQuery.data(), static_cast<GLsizei>(propertyValues.size()), NULL, propertyValues.data());

        mName.emplace();
        mName->resize(propertyValues[0]);
        glGetProgramResourceName(pShaderProgramHandle, GL_UNIFORM, pVariableIndex, propertyValues[0], NULL, mName->data());
        ZEPHYR_ASSERT(!mName->empty(), "Failed to get name of uniform variable in shader with handle {}", pShaderProgramHandle);
        mName->pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        mDataType     = GLType::convert(propertyValues[1]);
        mOffset       = propertyValues[2];
        mBlockIndex   = propertyValues[3];
        mArraySize    = propertyValues[4];
        mArrayStride  = propertyValues[5];
        mMatrixStride = propertyValues[6];
        mIsRowMajor   = propertyValues[7];
    }

    ShaderStorageBlockVariable::ShaderStorageBlockVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pVariableIndex)
        : GLSLVariable(GLType::GLSLVariableType::BufferBlock)
    {
        // Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
        // https://www.khronos.org/opengl/wiki/Program_Introspection

        static const std::array<GLenum, 10> propertyQuery = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE};

        std::array<GLint, propertyQuery.size()> propertyValues = {0};
        glGetProgramResourceiv(pShaderProgramHandle, GL_BUFFER_VARIABLE, pVariableIndex, static_cast<GLsizei>(propertyQuery.size()), propertyQuery.data(), static_cast<GLsizei>(propertyValues.size()), NULL, propertyValues.data());

        mName.emplace();
        mName->resize(propertyValues[0]);
        glGetProgramResourceName(pShaderProgramHandle, GL_BUFFER_VARIABLE, pVariableIndex, propertyValues[0], NULL, mName->data());
        ZEPHYR_ASSERT(!mName->empty(), "Failed to get name of shader buffer block variable in shader with handle {}", pShaderProgramHandle);
        mName->pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

        mDataType            = GLType::convert(propertyValues[1]);
        mOffset              = propertyValues[2];
        mBlockIndex          = propertyValues[3];
        mArraySize           = propertyValues[4];
        mArrayStride         = propertyValues[5];
        mMatrixStride        = propertyValues[6];
        mIsRowMajor          = propertyValues[7];
        mTopLevelArraySize   = propertyValues[8];
        mTopLevelArrayStride = propertyValues[9];

        if (mArrayStride > 0 && mArraySize == 0)
            mIsVariableArray = true;
    }

    void UniformVariable::Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Bool, "Attempting to set bool data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniform1i(mLocation.value(), (GLint)pValue); // Setting a boolean is treated as integer
    }
    void UniformBlockVariable::Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Bool, "Attempting to set bool data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, &pValue);
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const bool& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Bool, "Attempting to set bool data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, &pValue);
    }

    void UniformVariable::Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        // Setting texture sampler types uses int to set their bound texture unit.
        // The actual texture being sampled is set by setting active an texture unit and using bindTexture.
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Int
                   || mDataType.value() == GLType::DataType::Sampler2D
                   || mDataType.value() == GLType::DataType::SamplerCube
                   , "Attempting to set int data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniform1i(mLocation.value(), (GLint)pValue);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        // Setting texture sampler types uses int to set their bound texture unit.
        // The actual texture being sampled is set by setting active an texture unit and using bindTexture.
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Int
                   || mDataType.value() == GLType::DataType::Sampler2D
                   || mDataType.value() == GLType::DataType::SamplerCube
                   , "Attempting to set int data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));


        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, &pValue);
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const int& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        // Setting texture sampler types uses int to set their bound texture unit.
        // The actual texture being sampled is set by setting active an texture unit and using bindTexture.
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Int
                   || mDataType.value() == GLType::DataType::Sampler2D
                   || mDataType.value() == GLType::DataType::SamplerCube
                   , "Attempting to set int data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));

        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, &pValue);
    }

    void UniformVariable::Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Float, "Attempting to set float data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniform1f(mLocation.value(), pValue);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Float, "Attempting to set float data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, &pValue);
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const float& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Float, "Attempting to set float data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, &pValue);
    }

    void UniformVariable::Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec2, "Attempting to set vec2 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniform2fv(mLocation.value(), 1, &pValue[0]);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec2, "Attempting to set vec2 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const glm::vec2& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec2, "Attempting to set vec2 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }

    void UniformVariable::Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec3, "Attempting to set vec3 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniform3fv(mLocation.value(), 1, &pValue[0]);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec3, "Attempting to set vec3 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const glm::vec3& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec3, "Attempting to set vec3 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }

    void UniformVariable::Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec4, "Attempting to set vec4 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniform4fv(mLocation.value(), 1, &pValue[0]);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec4, "Attempting to set vec4 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const glm::vec4& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Vec4, "Attempting to set vec4 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }

    void UniformVariable::Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat2, "Attempting to set mat2 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniformMatrix2fv(mLocation.value(), 1, GL_FALSE, &pValue[0][0]);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat2, "Attempting to set mat2 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const glm::mat2& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat2, "Attempting to set mat2 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }

    void UniformVariable::Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat3, "Attempting to set mat3 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        glUniformMatrix3fv(mLocation.value(), 1, GL_FALSE, &pValue[0][0]);
    }
    void UniformBlockVariable::Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat3, "Attempting to set mat3 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const glm::mat3& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat3, "Attempting to set mat3 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }

    void UniformVariable::Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        glUniformMatrix4fv(mLocation.value(), 1, GL_FALSE, glm::value_ptr(pValue));
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat4, "Attempting to set mat4 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
    }
    void UniformBlockVariable::Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat4, "Attempting to set mat4 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        pGLState.BufferSubData(GLType::BufferType::UniformBuffer, mOffset.value(), size, glm::value_ptr(pValue));
    }
    void ShaderStorageBlockVariable::Set(GLState& pGLState, const glm::mat4& pValue, const size_t& pArrayIndex/*= 0*/)
    {
        ZEPHYR_ASSERT(mDataType.value() == GLType::DataType::Mat4, "Attempting to set mat4 data on {} {} ({})", GLType::toString(mDataType.value()), mName.value(), GLType::toString(mVariableType));
        static const auto size = sizeof(pValue);
        mBufferBacking->Bind(pGLState);

        const size_t variableOffset = static_cast<size_t>(mOffset.value() + (static_cast<size_t>(mArrayStride.value()) * pArrayIndex));
        if(mIsVariableArray && variableOffset >= mBufferBacking->GetReservedSize())
        {
            mBufferBacking->Extend(pGLState);
            ZEPHYR_ASSERT(!(variableOffset >= mBufferBacking->GetReservedSize()),
                          "Extending SSBO didn't reserve enough space to set variable. Extend should take a minimum extend size instead of only doubling already reserved size.");
        }

        pGLState.BufferSubData(GLType::BufferType::ShaderStorageBuffer, variableOffset, size, glm::value_ptr(pValue));
    }

    UniformBlockBindingPoint::UniformBlockBindingPoint(GLState& pGLState, UniformBlock& pUniformBlock, const unsigned int& pIndex)
        : mUBO(pGLState.CreateUBO(GLType::BufferUsage::StaticDraw))
        , mBindingPoint(pIndex)
        , mName(pUniformBlock.mName)
        , mInstances(0)
        , mVariables(pUniformBlock.mVariables)
    {
        for (auto& variable : mVariables)
            variable.mBufferBacking = mUBO;

        BindUniformBlock(pUniformBlock);
    }

    ShaderStorageBlockBindingPoint::ShaderStorageBlockBindingPoint(GLState& pGLState, ShaderStorageBlock& pStorageBlock, const unsigned int pIndex)
        : mSSBO(pGLState.CreateSSBO(GLType::BufferUsage::StaticDraw))
        , mBindingPoint(pIndex)
        , mName(pStorageBlock.mName)
        , mInstances(0)
        , mVariables(pStorageBlock.mVariables)
    {
        for (auto& variable : mVariables)
            variable.mBufferBacking = mSSBO;

        BindStorageBlock(pStorageBlock);
    }

    void VBO::PushVertexAttributeData(GLState& pGLState, const std::vector<float>& pData, const int& pAttributeIndex, const int& pAttributeSize)
    {
        PushData(pGLState, pData);
        glVertexAttribPointer(pAttributeIndex, pAttributeSize, GL_FLOAT, GL_FALSE, pAttributeSize * sizeof(float), (void *)0);
        glEnableVertexAttribArray(pAttributeIndex);
    }

    void UBO::AssignBindingPoint(GLState& pGLState, const unsigned int& pBindingPoint)
    {
        // When binding buffer range, we use the reserved buffer size since we dont have any actual data pushed until buffer variables are set.
        pGLState.BindBufferRange(mType, mHandle, pBindingPoint, 0, mReservedSize);
    }

    void SSBO::AssignBindingPoint(GLState& pGLState, const unsigned int& pBindingPoint)
    {
        mBindingPoint = pBindingPoint;
        // When binding buffer range, we use the reserved buffer size since we dont have any actual data pushed until buffer variables are set.
        pGLState.BindBufferRange(mType, mHandle, mBindingPoint.value() , 0, mReservedSize);
    }
    void SSBO::Extend(GLState& pGLState)
    {
        // Creates a new Buffer of extended size then copies the current data into the new buffer.
        // The original buffer is then deleted and the new buffer is assigned as this Buffer.

        const size_t previousSize = mReservedSize;
        const size_t newSize = mReservedSize == 0 ? 1 : mReservedSize * 2;

        auto newSSBOHandle = pGLState.GenBuffers();
        pGLState.BindBuffer(GLType::BufferType::CopyWriteBuffer, newSSBOHandle);
        pGLState.BufferData(GLType::BufferType::CopyWriteBuffer, newSize, NULL, GLType::BufferUsage::StreamCopy);
        pGLState.BindBuffer(GLType::BufferType::CopyReadBuffer, mHandle);
        pGLState.CopyBufferSubData(GLType::BufferType::CopyReadBuffer, GLType::BufferType::CopyWriteBuffer, 0, 0, mReservedSize);
        pGLState.DeleteBuffer(mHandle);

        mHandle = newSSBOHandle;
        pGLState.BindBuffer(GLType::BufferType::ShaderStorageBuffer, mHandle);
        mReservedSize = newSize;
        if (mBindingPoint.has_value())
            AssignBindingPoint(pGLState, mBindingPoint.value());

        ZEPHYR_ASSERT(mReservedSize == newSize, "Failed to resize the current buffer.");
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
    DataType convert(const int& pDataType)
    {
        switch (pDataType)
        {
            case GL_FLOAT :                                     return DataType::Float;
            case GL_FLOAT_VEC2 :                                return DataType::Vec2;
            case GL_FLOAT_VEC3 :                                return DataType::Vec3;
            case GL_FLOAT_VEC4 :                                return DataType::Vec4;
            case GL_DOUBLE :                                    return DataType::Double;
            case GL_DOUBLE_VEC2 :                               return DataType::DVec2;
            case GL_DOUBLE_VEC3 :                               return DataType::DVec3;
            case GL_DOUBLE_VEC4 :                               return DataType::DVec4;
            case GL_INT :                                       return DataType::Int;
            case GL_INT_VEC2 :                                  return DataType::IVec2;
            case GL_INT_VEC3 :                                  return DataType::IVec3;
            case GL_INT_VEC4 :                                  return DataType::IVec4;
            case GL_UNSIGNED_INT :                              return DataType::UnsignedInt;
            case GL_UNSIGNED_INT_VEC2 :                         return DataType::UVec2;
            case GL_UNSIGNED_INT_VEC3 :                         return DataType::UVec3;
            case GL_UNSIGNED_INT_VEC4 :                         return DataType::UVec4;
            case GL_BOOL :                                      return DataType::Bool;
            case GL_BOOL_VEC2 :                                 return DataType::BVec2;
            case GL_BOOL_VEC3 :                                 return DataType::BVec3;
            case GL_BOOL_VEC4 :                                 return DataType::BVec4;
            case GL_FLOAT_MAT2 :                                return DataType::Mat2;
            case GL_FLOAT_MAT3 :                                return DataType::Mat3;
            case GL_FLOAT_MAT4 :                                return DataType::Mat4;
            case GL_FLOAT_MAT2x3 :                              return DataType::Mat2x3;
            case GL_FLOAT_MAT2x4 :                              return DataType::Mat2x4;
            case GL_FLOAT_MAT3x2 :                              return DataType::Mat3x2;
            case GL_FLOAT_MAT3x4 :                              return DataType::Mat3x4;
            case GL_FLOAT_MAT4x2 :                              return DataType::Mat4x2;
            case GL_FLOAT_MAT4x3 :                              return DataType::Mat4x3;
            case GL_DOUBLE_MAT2 :                               return DataType::Dmat2;
            case GL_DOUBLE_MAT3 :                               return DataType::Dmat3;
            case GL_DOUBLE_MAT4 :                               return DataType::Dmat4;
            case GL_DOUBLE_MAT2x3 :                             return DataType::Dmat2x3;
            case GL_DOUBLE_MAT2x4 :                             return DataType::Dmat2x4;
            case GL_DOUBLE_MAT3x2 :                             return DataType::Dmat3x2;
            case GL_DOUBLE_MAT3x4 :                             return DataType::Dmat3x4;
            case GL_DOUBLE_MAT4x2 :                             return DataType::Dmat4x2;
            case GL_DOUBLE_MAT4x3 :                             return DataType::Dmat4x3;
            case GL_SAMPLER_1D :                                return DataType::Sampler1D;
            case GL_SAMPLER_2D :                                return DataType::Sampler2D;
            case GL_SAMPLER_3D :                                return DataType::Sampler3D;
            case GL_SAMPLER_CUBE :                              return DataType::SamplerCube;
            case GL_SAMPLER_1D_SHADOW :                         return DataType::Sampler1DShadow;
            case GL_SAMPLER_2D_SHADOW :                         return DataType::Sampler2DShadow;
            case GL_SAMPLER_1D_ARRAY :                          return DataType::Sampler1DArray;
            case GL_SAMPLER_2D_ARRAY :                          return DataType::Sampler2DArray;
            case GL_SAMPLER_1D_ARRAY_SHADOW :                   return DataType::Sampler1DArrayShadow;
            case GL_SAMPLER_2D_ARRAY_SHADOW :                   return DataType::Sampler2DArrayShadow;
            case GL_SAMPLER_2D_MULTISAMPLE :                    return DataType::Sampler2DMS;
            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY :              return DataType::Sampler2DMSArray;
            case GL_SAMPLER_CUBE_SHADOW :                       return DataType::SamplerCubeShadow;
            case GL_SAMPLER_BUFFER :                            return DataType::SamplerBuffer;
            case GL_SAMPLER_2D_RECT :                           return DataType::Sampler2DRect;
            case GL_SAMPLER_2D_RECT_SHADOW :                    return DataType::Sampler2DRectShadow;
            case GL_INT_SAMPLER_1D :                            return DataType::Isampler1D;
            case GL_INT_SAMPLER_2D :                            return DataType::Isampler2D;
            case GL_INT_SAMPLER_3D :                            return DataType::Isampler3D;
            case GL_INT_SAMPLER_CUBE :                          return DataType::IsamplerCube;
            case GL_INT_SAMPLER_1D_ARRAY :                      return DataType::Isampler1DArray;
            case GL_INT_SAMPLER_2D_ARRAY :                      return DataType::Isampler2DArray;
            case GL_INT_SAMPLER_2D_MULTISAMPLE :                return DataType::Isampler2DMS;
            case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY :          return DataType::Isampler2DMSArray;
            case GL_INT_SAMPLER_BUFFER :                        return DataType::IsamplerBuffer;
            case GL_INT_SAMPLER_2D_RECT :                       return DataType::Isampler2DRect;
            case GL_UNSIGNED_INT_SAMPLER_1D :                   return DataType::Usampler1D;
            case GL_UNSIGNED_INT_SAMPLER_2D :                   return DataType::Usampler2D;
            case GL_UNSIGNED_INT_SAMPLER_3D :                   return DataType::Usampler3D;
            case GL_UNSIGNED_INT_SAMPLER_CUBE :                 return DataType::UsamplerCube;
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY :             return DataType::Usampler2DArray;
            case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE :       return DataType::Usampler2DMS;
            case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY : return DataType::Usampler2DMSArray;
            case GL_UNSIGNED_INT_SAMPLER_BUFFER :               return DataType::UsamplerBuffer;
            case GL_UNSIGNED_INT_SAMPLER_2D_RECT :              return DataType::Usampler2DRect;
            default:
                ZEPHYR_ASSERT(false, "Unknown DataType requested");
                return DataType::Unknown;
        }
    }

    std::string toString(const Function& pFunction)
    {
        switch (pFunction)
        {
            case Function::Viewport :                  return "Viewport";
            case Function::DrawArrays :                return "DrawArrays";
            case Function::DrawArraysInstanced :       return "DrawArraysInstanced";
            case Function::DrawElements :              return "DrawElements";
            case Function::DrawElementsInstanced :     return "DrawElementsInstanced";
            case Function::BindFramebuffer :           return "BindFramebuffer";
            case Function::CreateShader :              return "CreateShader";
            case Function::ShaderSource :              return "ShaderSource";
            case Function::CompileShader :             return "CompileShader";
            case Function::CreateProgram :             return "CreateProgram";
            case Function::AttachShader :              return "AttachShader";
            case Function::LinkProgram :               return "LinkProgram";
            case Function::DeleteShader :              return "DeleteShader";
            case Function::UseProgram :                return "UseProgram";
            case Function::BindBuffer :                return "BindBuffer";
            case Function::DeleteBuffer :              return "DeleteBuffer";
            case Function::BufferData :                return "BufferData";
            case Function::BufferSubData :             return "BufferSubData";
            case Function::BindBufferRange :           return "BindBufferRange";
            case Function::UniformBlockBinding :       return "UniformBlockBinding";
            case Function::ShaderStorageBlockBinding : return "ShaderStorageBlockBinding";
            case Function::CopyBufferSubData :         return "CopyBufferSubData";
            default:
                ZEPHYR_ASSERT(false, "Unknown Function requested");
                return "";
        }
    }
    std::string toString(const ShaderProgramType& pShaderProgramType)
    {
        switch (pShaderProgramType)
        {
            case ShaderProgramType::Vertex:   return "VertexShader";
            case ShaderProgramType::Geometry: return "GeometryShader";
            case ShaderProgramType::Fragment: return "FragmentShader";
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderProgramType requested");
                return "";
        }
    }
    std::string toString(const DataType& pDataType)
    {
        switch (pDataType)
        {
            case DataType::Float :                return "Float";
            case DataType::Vec2 :                 return "vec2";
            case DataType::Vec3 :                 return "vec3";
            case DataType::Vec4 :                 return "vec4";
            case DataType::Double :               return "Double";
            case DataType::DVec2 :                return "DVec2";
            case DataType::DVec3 :                return "DVec3";
            case DataType::DVec4 :                return "DVec4";
            case DataType::Int :                  return "Int";
            case DataType::IVec2 :                return "IVec2";
            case DataType::IVec3 :                return "IVec3";
            case DataType::IVec4 :                return "IVec4";
            case DataType::UnsignedInt :          return "UnsignedInt";
            case DataType::UVec2 :                return "UVec2";
            case DataType::UVec3 :                return "UVec3";
            case DataType::UVec4 :                return "UVec4";
            case DataType::Bool :                 return "Bool";
            case DataType::BVec2 :                return "BVec2";
            case DataType::BVec3 :                return "BVec3";
            case DataType::BVec4 :                return "BVec4";
            case DataType::Mat2 :                 return "Mat2";
            case DataType::Mat3 :                 return "Mat3";
            case DataType::Mat4 :                 return "Mat4";
            case DataType::Mat2x3 :               return "Mat2x3";
            case DataType::Mat2x4 :               return "Mat2x4";
            case DataType::Mat3x2 :               return "Mat3x2";
            case DataType::Mat3x4 :               return "Mat3x4";
            case DataType::Mat4x2 :               return "Mat4x2";
            case DataType::Mat4x3 :               return "Mat4x3";
            case DataType::Dmat2 :                return "Dmat2";
            case DataType::Dmat3 :                return "Dmat3";
            case DataType::Dmat4 :                return "Dmat4";
            case DataType::Dmat2x3 :              return "Dmat2x3";
            case DataType::Dmat2x4 :              return "Dmat2x4";
            case DataType::Dmat3x2 :              return "Dmat3x2";
            case DataType::Dmat3x4 :              return "Dmat3x4";
            case DataType::Dmat4x2 :              return "Dmat4x2";
            case DataType::Dmat4x3 :              return "Dmat4x3";
            case DataType::Sampler1D :            return "Sampler1D";
            case DataType::Sampler2D :            return "Sampler2D";
            case DataType::Sampler3D :            return "Sampler3D";
            case DataType::SamplerCube :          return "SamplerCube";
            case DataType::Sampler1DShadow :      return "Sampler1DShadow";
            case DataType::Sampler2DShadow :      return "Sampler2DShadow";
            case DataType::Sampler1DArray :       return "Sampler1DArray";
            case DataType::Sampler2DArray :       return "Sampler2DArray";
            case DataType::Sampler1DArrayShadow : return "Sampler1DArrayShadow";
            case DataType::Sampler2DArrayShadow : return "Sampler2DArrayShadow";
            case DataType::Sampler2DMS :          return "Sampler2DMS";
            case DataType::Sampler2DMSArray :     return "Sampler2DMSArray";
            case DataType::SamplerCubeShadow :    return "SamplerCubeShadow";
            case DataType::SamplerBuffer :        return "SamplerBuffer";
            case DataType::Sampler2DRect :        return "Sampler2DRect";
            case DataType::Sampler2DRectShadow :  return "Sampler2DRectShadow";
            case DataType::Isampler1D :           return "Isampler1D";
            case DataType::Isampler2D :           return "Isampler2D";
            case DataType::Isampler3D :           return "Isampler3D";
            case DataType::IsamplerCube :         return "IsamplerCube";
            case DataType::Isampler1DArray :      return "Isampler1DArray";
            case DataType::Isampler2DArray :      return "Isampler2DArray";
            case DataType::Isampler2DMS :         return "Isampler2DMS";
            case DataType::Isampler2DMSArray :    return "Isampler2DMSArray";
            case DataType::IsamplerBuffer :       return "IsamplerBuffer";
            case DataType::Isampler2DRect :       return "Isampler2DRect";
            case DataType::Usampler1D :           return "Usampler1D";
            case DataType::Usampler2D :           return "Usampler2D";
            case DataType::Usampler3D :           return "Usampler3D";
            case DataType::UsamplerCube :         return "UsamplerCube";
            case DataType::Usampler2DArray :      return "Usampler2DArray";
            case DataType::Usampler2DMS :         return "Usampler2DMS";
            case DataType::Usampler2DMSArray :    return "Usampler2DMSArray";
            case DataType::UsamplerBuffer :       return "UsamplerBuffer";
            case DataType::Usampler2DRect :       return "Usampler2DRect";
            case DataType::Unknown :              return "Unknown";
            default:
                ZEPHYR_ASSERT(false, "Unknown DataType requested");
                return "";
        }
    }
    std::string toString(const GLSLVariableType& pVariableType)
    {
        switch (pVariableType)
        {
            case GLSLVariableType::Uniform :      return "'loose' Uniform variable";
            case GLSLVariableType::UniformBlock : return "Uniform block variable";
            case GLSLVariableType::BufferBlock :  return "Buffer block variable";
            default:
                ZEPHYR_ASSERT(false, "Unknown GLSLVariableType requested");
                return "";
        }
    }
    std::string toString(const BufferType& pBufferType)
    {
        switch (pBufferType)
        {
            case BufferType::ArrayBuffer :            return "Array Buffer";
            case BufferType::AtomicCounterBuffer :    return "Atomic Counter Buffer";
            case BufferType::CopyReadBuffer :         return "Copy Read Buffer";
            case BufferType::CopyWriteBuffer :        return "Copy Write Buffer";
            case BufferType::DispatchIndirectBuffer : return "Dispatch Indirect Buffer";
            case BufferType::DrawIndirectBuffer :     return "Draw Indirect Buffer";
            case BufferType::ElementArrayBuffer :     return "Element Array Buffer";
            case BufferType::PixelPackBuffer :        return "Pixel Pack Buffer";
            case BufferType::PixelUnpackBuffer :      return "Pixel Unpack Buffer";
            case BufferType::QueryBuffer :            return "Query Buffer";
            case BufferType::ShaderStorageBuffer :    return "Shader Storage Buffer";
            case BufferType::TextureBuffer :          return "Texture Buffer";
            case BufferType::TransformFeedbackBuffer :return "Transform Feedback Buffer";
            case BufferType::UniformBuffer :          return "Uniform Buffer";
            default:
                ZEPHYR_ASSERT(false, "Unknown BufferType requested");
                return "";
        }
    }
    std::string toString(const BufferUsage& pBufferUsage)
    {
        switch (pBufferUsage)
        {
            case BufferUsage::StreamDraw:  return "Stream Draw";
            case BufferUsage::StreamRead:  return "Stream Read";
            case BufferUsage::StreamCopy:  return "Stream Copy";
            case BufferUsage::StaticDraw:  return "Static Draw";
            case BufferUsage::StaticRead:  return "Static Read";
            case BufferUsage::StaticCopy:  return "Static Copy";
            case BufferUsage::DynamicDraw: return "Dynamic Draw";
            case BufferUsage::DynamicRead: return "Dynamic Read";
            case BufferUsage::DynamicCopy: return "Dynamic Copy";
            default:
                ZEPHYR_ASSERT(false, "Unknown pBufferUsage requested");
                return "";
        }
    }
    std::string toString(const ShaderResourceType& pResourceType)
    {
        switch (pResourceType)
        {
            case ShaderResourceType::Uniform :                         return "Uniform";
            case ShaderResourceType::UniformBlock :                    return "UniformBlock";
            case ShaderResourceType::ShaderStorageBlock :              return "ShaderStorageBlock";
            case ShaderResourceType::BufferVariable :                  return "BufferVariable";
            case ShaderResourceType::Buffer :                          return "Buffer";
            case ShaderResourceType::ProgramInput :                    return "ProgramInput";
            case ShaderResourceType::ProgramOutput :                   return "ProgramOutput";
            case ShaderResourceType::AtomicCounterBuffer :             return "AtomicCounterBuffer";
            //case ShaderResourceType::AtomicCounterShader, :          return //"AtomicCounterShader";
            case ShaderResourceType::VertexSubroutineUniform :         return "VertexSubroutineUniform";
            case ShaderResourceType::FragmentSubroutineUniform :       return "FragmentSubroutineUniform";
            case ShaderResourceType::GeometrySubroutineUniform :       return "GeometrySubroutineUniform";
            case ShaderResourceType::ComputeSubroutineUniform :        return "ComputeSubroutineUniform";
            case ShaderResourceType::TessControlSubroutineUniform :    return "TessControlSubroutineUniform";
            case ShaderResourceType::TessEvaluationSubroutineUniform : return "TessEvaluationSubroutineUniform";
            case ShaderResourceType::TransformFeedbackBuffer :         return "TransformFeedbackBuffer";
            case ShaderResourceType::TransformFeedbackVarying :        return "TransformFeedbackVarying";
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderResourceType requested");
                return "";
    }
    }
    std::string toString(const ShaderResourceProperty& pShaderResourceProperty)
    {
        switch (pShaderResourceProperty)
        {
            case ShaderResourceProperty::NameLength :                       return "NameLength";
            case ShaderResourceProperty::Type :                             return "Type";
            case ShaderResourceProperty::ArraySize :                        return "ArraySize";
            case ShaderResourceProperty::Offset :                           return "Offset";
            case ShaderResourceProperty::BlockIndex :                       return "BlockIndex";
            case ShaderResourceProperty::ArrayStride :                      return "ArrayStride";
            case ShaderResourceProperty::MatrixStride :                     return "MatrixStride";
            case ShaderResourceProperty::IsRowMajor :                       return "IsRowMajor";
            case ShaderResourceProperty::AtomicCounterBufferIndex :         return "AtomicCounterBufferIndex";
            case ShaderResourceProperty::TextureBuffer :                    return "TextureBuffer";
            case ShaderResourceProperty::BufferBinding :                    return "BufferBinding";
            case ShaderResourceProperty::BufferDataSize :                   return "BufferDataSize";
            case ShaderResourceProperty::NumActiveVariables :               return "NumActiveVariables";
            case ShaderResourceProperty::ActiveVariables :                  return "ActiveVariables";
            case ShaderResourceProperty::ReferencedByVertexShader :         return "ReferencedByVertexShader";
            case ShaderResourceProperty::ReferencedByTessControlShader :    return "ReferencedByTessControlShader";
            case ShaderResourceProperty::ReferencedByTessEvaluationShader : return "ReferencedByTessEvaluationShader";
            case ShaderResourceProperty::ReferencedByGeometryShader :       return "ReferencedByGeometryShader";
            case ShaderResourceProperty::ReferencedByFragmentShader :       return "ReferencedByFragmentShader";
            case ShaderResourceProperty::ReferencedByComputeShader :        return "ReferencedByComputeShader";
            case ShaderResourceProperty::NumCompatibleSubroutines :         return "NumCompatibleSubroutines";
            case ShaderResourceProperty::CompatibleSubroutines :            return "CompatibleSubroutines";
            case ShaderResourceProperty::TopLevelArraySize :                return "TopLevelArraySize";
            case ShaderResourceProperty::TopLevelArrayStride :              return "TopLevelArrayStride";
            case ShaderResourceProperty::Location :                         return "Location";
            case ShaderResourceProperty::LocationIndex :                    return "LocationIndex";
            case ShaderResourceProperty::IsPerPatch :                       return "IsPerPatch";
            case ShaderResourceProperty::LocationComponent :                return "LocationComponent";
            case ShaderResourceProperty::TransformFeedbackBufferIndex :     return "TransformFeedbackBufferIndex";
            case ShaderResourceProperty::TransformFeedbackBufferStride :    return "TransformFeedbackBufferStride";
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderResourceProperty requested");
                return "";
        }
    }
    std::string toString(const DepthTestType& pDepthTestType)
    {
        switch (pDepthTestType)
        {
            case DepthTestType::Always :       return "Always";
            case DepthTestType::Never :        return "Never";
            case DepthTestType::Less :         return "Less";
            case DepthTestType::Equal :        return "Equal";
            case DepthTestType::NotEqual :     return "Not equal";
            case DepthTestType::Greater :      return "Greater than";
            case DepthTestType::LessEqual :    return "Less than or equal";
            case DepthTestType::GreaterEqual : return "Greater than or equal";
            default:
                ZEPHYR_ASSERT(false, "Unknown DepthTestType requested");
                return "";
    }
    }
    std::string toString(const BlendFactorType& pBlendFactorType)
    {
        switch (pBlendFactorType)
    {
            case BlendFactorType::Zero :                      return "Zero";
            case BlendFactorType::One :                       return "One";
            case BlendFactorType::SourceColour :              return "Source Colour";
            case BlendFactorType::OneMinusSourceColour :      return "One Minus Source Colour";
            case BlendFactorType::DestinationColour :         return "Destination Colour";
            case BlendFactorType::OneMinusDestinationColour : return "One Minus Destination Colour";
            case BlendFactorType::SourceAlpha :               return "Source Alpha";
            case BlendFactorType::OneMinusSourceAlpha :       return "One Minus Source Alpha";
            case BlendFactorType::DestinationAlpha :          return "Destination Alpha";
            case BlendFactorType::OneMinusDestinationAlpha :  return "One Minus Destination Alpha";
            case BlendFactorType::ConstantColour :            return "Constant Colour";
            case BlendFactorType::OneMinusConstantColour :    return "One Minus Constant Colour";
            case BlendFactorType::ConstantAlpha :             return "Constant Alpha";
            case BlendFactorType::OneMinusConstantAlpha :     return "One Minus Constant Alpha";
            default:
                ZEPHYR_ASSERT(false, "Unknown BlendFactorType requested");
                return "";
        }
    }
    std::string toString(const CullFacesType& pCullFacesType)
    {
        switch (pCullFacesType)
        {
            case CullFacesType::Back :         return "Back";
            case CullFacesType::Front :        return "Front";
            case CullFacesType::FrontAndBack : return "Front and Back";
            default:
                ZEPHYR_ASSERT(false, "Unknown CullFacesType requested");
                return "";
    }
    }
    std::string toString(const FrontFaceOrientation& pFrontFaceOrientation)
    {
        switch (pFrontFaceOrientation)
        {
            case FrontFaceOrientation::Clockwise :        return "Clockwise";
            case FrontFaceOrientation::CounterClockwise : return "CounterClockwise";
            default:
                ZEPHYR_ASSERT(false, "Unknown FrontFaceOrientation requested");
                return "";
    }
    }
    std::string toString(const PolygonMode& pPolygonMode)
    {
        switch (pPolygonMode)
        {
            case PolygonMode::Point : return "Point";
            case PolygonMode::Line :  return "Line";
            case PolygonMode::Fill :  return "Fill";
            default:
                ZEPHYR_ASSERT(false, "Unknown PolygonMode requested");
                return "";
    }
    }
    std::string toString(const PrimitiveMode& pPrimitiveMode)
    {
        switch (pPrimitiveMode)
        {;
            case PrimitiveMode::Points:                 return "Points";
            case PrimitiveMode::LineStrip:              return "LineStrip";
            case PrimitiveMode::LineLoop:               return "LineLoop";
            case PrimitiveMode::Lines:                  return "Lines";
            case PrimitiveMode::LineStripAdjacency:     return "LineStripAdjacency";
            case PrimitiveMode::LinesAdjacency:         return "LinesAdjacency";
            case PrimitiveMode::TriangleStrip:          return "TriangleStrip";
            case PrimitiveMode::TriangleFan:            return "TriangleFan";
            case PrimitiveMode::Triangles:              return "Triangles";
            case PrimitiveMode::TriangleStripAdjacency: return "TriangleStripAdjacency";
            case PrimitiveMode::TrianglesAdjacency:     return "TrianglesAdjacency";
            case PrimitiveMode::Patches:                return "Patches";
            default:
                ZEPHYR_ASSERT(false, "Unknown PrimitiveMode requested");
                return "";
        }
    }
    std::string toString(const FramebufferTarget& pFramebufferTarget)
    {
        switch (pFramebufferTarget)
        {
            case FramebufferTarget::DrawFramebuffer: return "DrawFramebuffer";
            case FramebufferTarget::ReadFramebuffer: return "ReadFramebuffer";
            case FramebufferTarget::Framebuffer:     return "Framebuffer";
            default:
                ZEPHYR_ASSERT(false, "Unknown FramebufferTarget requested");
                return "";
    }
    }
    std::string ToDefaultErrorMessage(const ErrorType &pErrorType)
    {
        // Messages from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetError.xhtml
        switch (pErrorType)
        {
            case ErrorType::InvalidEnum:                 return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag";
            case ErrorType::InvalidValue:                return "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag";
            case ErrorType::InvalidOperation:            return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.The offending command is ignored andhas no other side effect than to set the error flag";
            case ErrorType::InvalidFramebufferOperation: return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag";
            case ErrorType::OutOfMemory:                 return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded";
            case ErrorType::StackUnderflow:              return "GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow";
            case ErrorType::StackOverflow:               return "GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow";
            default:
                ZEPHYR_ASSERT(false, "Unknown ErrorType requested");
                return 0;
        }
    }

    int convert(const DataType& pDataType)
    {
        switch (pDataType)
        {
            case DataType::Float :                return GL_FLOAT;
            case DataType::Vec2 :                 return GL_FLOAT_VEC2;
            case DataType::Vec3 :                 return GL_FLOAT_VEC3;
            case DataType::Vec4 :                 return GL_FLOAT_VEC4;
            case DataType::Double :               return GL_DOUBLE;
            case DataType::DVec2 :                return GL_DOUBLE_VEC2;
            case DataType::DVec3 :                return GL_DOUBLE_VEC3;
            case DataType::DVec4 :                return GL_DOUBLE_VEC4;
            case DataType::Int :                  return GL_INT;
            case DataType::IVec2 :                return GL_INT_VEC2;
            case DataType::IVec3 :                return GL_INT_VEC3;
            case DataType::IVec4 :                return GL_INT_VEC4;
            case DataType::UnsignedInt :          return GL_UNSIGNED_INT;
            case DataType::UVec2 :                return GL_UNSIGNED_INT_VEC2;
            case DataType::UVec3 :                return GL_UNSIGNED_INT_VEC3;
            case DataType::UVec4 :                return GL_UNSIGNED_INT_VEC4;
            case DataType::Bool :                 return GL_BOOL;
            case DataType::BVec2 :                return GL_BOOL_VEC2;
            case DataType::BVec3 :                return GL_BOOL_VEC3;
            case DataType::BVec4 :                return GL_BOOL_VEC4;
            case DataType::Mat2 :                 return GL_FLOAT_MAT2;
            case DataType::Mat3 :                 return GL_FLOAT_MAT3;
            case DataType::Mat4 :                 return GL_FLOAT_MAT4;
            case DataType::Mat2x3 :               return GL_FLOAT_MAT2x3;
            case DataType::Mat2x4 :               return GL_FLOAT_MAT2x4;
            case DataType::Mat3x2 :               return GL_FLOAT_MAT3x2;
            case DataType::Mat3x4 :               return GL_FLOAT_MAT3x4;
            case DataType::Mat4x2 :               return GL_FLOAT_MAT4x2;
            case DataType::Mat4x3 :               return GL_FLOAT_MAT4x3;
            case DataType::Dmat2 :                return GL_DOUBLE_MAT2;
            case DataType::Dmat3 :                return GL_DOUBLE_MAT3;
            case DataType::Dmat4 :                return GL_DOUBLE_MAT4;
            case DataType::Dmat2x3 :              return GL_DOUBLE_MAT2x3;
            case DataType::Dmat2x4 :              return GL_DOUBLE_MAT2x4;
            case DataType::Dmat3x2 :              return GL_DOUBLE_MAT3x2;
            case DataType::Dmat3x4 :              return GL_DOUBLE_MAT3x4;
            case DataType::Dmat4x2 :              return GL_DOUBLE_MAT4x2;
            case DataType::Dmat4x3 :              return GL_DOUBLE_MAT4x3;
            case DataType::Sampler1D :            return GL_SAMPLER_1D;
            case DataType::Sampler2D :            return GL_SAMPLER_2D;
            case DataType::Sampler3D :            return GL_SAMPLER_3D;
            case DataType::SamplerCube :          return GL_SAMPLER_CUBE;
            case DataType::Sampler1DShadow :      return GL_SAMPLER_1D_SHADOW;
            case DataType::Sampler2DShadow :      return GL_SAMPLER_2D_SHADOW;
            case DataType::Sampler1DArray :       return GL_SAMPLER_1D_ARRAY;
            case DataType::Sampler2DArray :       return GL_SAMPLER_2D_ARRAY;
            case DataType::Sampler1DArrayShadow : return GL_SAMPLER_1D_ARRAY_SHADOW;
            case DataType::Sampler2DArrayShadow : return GL_SAMPLER_2D_ARRAY_SHADOW;
            case DataType::Sampler2DMS :          return GL_SAMPLER_2D_MULTISAMPLE;
            case DataType::Sampler2DMSArray :     return GL_SAMPLER_2D_MULTISAMPLE_ARRAY;
            case DataType::SamplerCubeShadow :    return GL_SAMPLER_CUBE_SHADOW;
            case DataType::SamplerBuffer :        return GL_SAMPLER_BUFFER;
            case DataType::Sampler2DRect :        return GL_SAMPLER_2D_RECT;
            case DataType::Sampler2DRectShadow :  return GL_SAMPLER_2D_RECT_SHADOW;
            case DataType::Isampler1D :           return GL_INT_SAMPLER_1D;
            case DataType::Isampler2D :           return GL_INT_SAMPLER_2D;
            case DataType::Isampler3D :           return GL_INT_SAMPLER_3D;
            case DataType::IsamplerCube :         return GL_INT_SAMPLER_CUBE;
            case DataType::Isampler1DArray :      return GL_INT_SAMPLER_1D_ARRAY;
            case DataType::Isampler2DArray :      return GL_INT_SAMPLER_2D_ARRAY;
            case DataType::Isampler2DMS :         return GL_INT_SAMPLER_2D_MULTISAMPLE;
            case DataType::Isampler2DMSArray :    return GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
            case DataType::IsamplerBuffer :       return GL_INT_SAMPLER_BUFFER;
            case DataType::Isampler2DRect :       return GL_INT_SAMPLER_2D_RECT;
            case DataType::Usampler1D :           return GL_UNSIGNED_INT_SAMPLER_1D;
            case DataType::Usampler2D :           return GL_UNSIGNED_INT_SAMPLER_2D;
            case DataType::Usampler3D :           return GL_UNSIGNED_INT_SAMPLER_3D;
            case DataType::UsamplerCube :         return GL_UNSIGNED_INT_SAMPLER_CUBE;
            case DataType::Usampler2DArray :      return GL_UNSIGNED_INT_SAMPLER_2D_ARRAY;
            case DataType::Usampler2DMS :         return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
            case DataType::Usampler2DMSArray :    return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
            case DataType::UsamplerBuffer :       return GL_UNSIGNED_INT_SAMPLER_BUFFER;
            case DataType::Usampler2DRect :       return GL_UNSIGNED_INT_SAMPLER_2D_RECT;
            default:
                ZEPHYR_ASSERT(false, "Unknown DataType requested");
                return 0;
        }
    }
    int convert(const GLSLVariableType& pVariableType)
    {
        switch (pVariableType)
        {
            // Both loose uniform variables and UniformBlockVariables are treated as GL_UNIFORM in OpenGL
            case GLSLVariableType::Uniform:      return GL_UNIFORM;
            case GLSLVariableType::UniformBlock: return GL_UNIFORM;
            case GLSLVariableType::BufferBlock:  return GL_BUFFER_VARIABLE;
            default:
                ZEPHYR_ASSERT(false, "Unknown GLSLVariableType requested");
                return 0;
        }
    }
    int convert(const BufferType& pBufferType)
    {
        switch (pBufferType)
        {
            case BufferType::ArrayBuffer :             return GL_ARRAY_BUFFER;
            case BufferType::AtomicCounterBuffer :     return GL_ATOMIC_COUNTER_BUFFER;
            case BufferType::CopyReadBuffer :          return GL_COPY_READ_BUFFER;
            case BufferType::CopyWriteBuffer :         return GL_COPY_WRITE_BUFFER;
            case BufferType::DispatchIndirectBuffer :  return GL_DISPATCH_INDIRECT_BUFFER;
            case BufferType::DrawIndirectBuffer :      return GL_DRAW_INDIRECT_BUFFER;
            case BufferType::ElementArrayBuffer :      return GL_ELEMENT_ARRAY_BUFFER;
            case BufferType::PixelPackBuffer :         return GL_PIXEL_PACK_BUFFER;
            case BufferType::PixelUnpackBuffer :       return GL_PIXEL_UNPACK_BUFFER;
            case BufferType::QueryBuffer :             return GL_QUERY_BUFFER;
            case BufferType::ShaderStorageBuffer :     return GL_SHADER_STORAGE_BUFFER;
            case BufferType::TextureBuffer :           return GL_TEXTURE_BUFFER;
            case BufferType::TransformFeedbackBuffer : return GL_TRANSFORM_FEEDBACK_BUFFER;
            case BufferType::UniformBuffer :           return GL_UNIFORM_BUFFER;
            default:
                ZEPHYR_ASSERT(false, "Unknown BufferType requested");
                return 0;
        }
    }
    int convert(const BufferUsage& pBufferUsage)
    {
        switch (pBufferUsage)
        {
            case BufferUsage::StreamDraw:  return GL_STREAM_DRAW;
            case BufferUsage::StreamRead:  return GL_STREAM_READ;
            case BufferUsage::StreamCopy:  return GL_STREAM_COPY;
            case BufferUsage::StaticDraw:  return GL_STATIC_DRAW;
            case BufferUsage::StaticRead:  return GL_STATIC_READ;
            case BufferUsage::StaticCopy:  return GL_STATIC_COPY;
            case BufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
            case BufferUsage::DynamicRead: return GL_DYNAMIC_READ;
            case BufferUsage::DynamicCopy: return GL_DYNAMIC_COPY;
            default:
                ZEPHYR_ASSERT(false, "Unknown pBufferUsage requested");
                return 0;
        }
    }
    int convert(const ShaderProgramType& pShaderProgramType)
    {
        switch (pShaderProgramType)
        {
            case ShaderProgramType::Vertex:   return GL_VERTEX_SHADER;
            case ShaderProgramType::Geometry: return GL_GEOMETRY_SHADER;
            case ShaderProgramType::Fragment: return GL_FRAGMENT_SHADER;
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderProgramType requested");
                return 0;
        }
    }
    int convert(const ShaderResourceType& pResourceType)
    {
        switch (pResourceType)
    	{
            case ShaderResourceType::Uniform:                         return GL_UNIFORM;
            case ShaderResourceType::UniformBlock:                    return GL_UNIFORM_BLOCK;
            case ShaderResourceType::ShaderStorageBlock:              return GL_SHADER_STORAGE_BLOCK;
            case ShaderResourceType::BufferVariable:                  return GL_BUFFER_VARIABLE;
            case ShaderResourceType::Buffer:                          return GL_BUFFER;
            case ShaderResourceType::ProgramInput:                    return GL_PROGRAM_INPUT;
            case ShaderResourceType::ProgramOutput:                   return GL_PROGRAM_OUTPUT;
            case ShaderResourceType::AtomicCounterBuffer:             return GL_ATOMIC_COUNTER_BUFFER;
            //case ShaderResourceType::AtomicCounterShader:           return GL_ATOMIC_COUNTER_SHADER;
            case ShaderResourceType::VertexSubroutineUniform:         return GL_VERTEX_SUBROUTINE_UNIFORM;
            case ShaderResourceType::FragmentSubroutineUniform:       return GL_FRAGMENT_SUBROUTINE_UNIFORM;
            case ShaderResourceType::GeometrySubroutineUniform:       return GL_GEOMETRY_SUBROUTINE_UNIFORM;
            case ShaderResourceType::ComputeSubroutineUniform:        return GL_COMPUTE_SUBROUTINE_UNIFORM;
            case ShaderResourceType::TessControlSubroutineUniform:    return GL_TESS_CONTROL_SUBROUTINE_UNIFORM;
            case ShaderResourceType::TessEvaluationSubroutineUniform: return GL_TESS_EVALUATION_SUBROUTINE_UNIFORM;
            case ShaderResourceType::TransformFeedbackBuffer:         return GL_TRANSFORM_FEEDBACK_BUFFER;
            case ShaderResourceType::TransformFeedbackVarying:        return GL_TRANSFORM_FEEDBACK_VARYING;
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderResourceType requested");
                return 0;
        }
    }
    int convert(const ShaderResourceProperty& pShaderResourceProperty)
    {
        switch (pShaderResourceProperty)
    	{
            case ShaderResourceProperty::NameLength:                       return GL_NAME_LENGTH;
            case ShaderResourceProperty::Type:                             return GL_TYPE;
            case ShaderResourceProperty::ArraySize:                        return GL_ARRAY_SIZE;
            case ShaderResourceProperty::Offset:                           return GL_OFFSET;
            case ShaderResourceProperty::BlockIndex:                       return GL_BLOCK_INDEX;
            case ShaderResourceProperty::ArrayStride:                      return GL_ARRAY_STRIDE;
            case ShaderResourceProperty::MatrixStride:                     return GL_MATRIX_STRIDE;
            case ShaderResourceProperty::IsRowMajor:                       return GL_IS_ROW_MAJOR;
            case ShaderResourceProperty::AtomicCounterBufferIndex:         return GL_ATOMIC_COUNTER_BUFFER_INDEX;
            case ShaderResourceProperty::TextureBuffer:                    return GL_TEXTURE_BUFFER;
            case ShaderResourceProperty::BufferBinding:                    return GL_BUFFER_BINDING;
            case ShaderResourceProperty::BufferDataSize:                   return GL_BUFFER_DATA_SIZE;
            case ShaderResourceProperty::NumActiveVariables:               return GL_NUM_ACTIVE_VARIABLES;
            case ShaderResourceProperty::ActiveVariables:                  return GL_ACTIVE_VARIABLES;
            case ShaderResourceProperty::ReferencedByVertexShader:         return GL_REFERENCED_BY_VERTEX_SHADER;
            case ShaderResourceProperty::ReferencedByTessControlShader:    return GL_REFERENCED_BY_TESS_CONTROL_SHADER;
            case ShaderResourceProperty::ReferencedByTessEvaluationShader: return GL_REFERENCED_BY_TESS_EVALUATION_SHADER;
            case ShaderResourceProperty::ReferencedByGeometryShader:       return GL_REFERENCED_BY_GEOMETRY_SHADER;
            case ShaderResourceProperty::ReferencedByFragmentShader:       return GL_REFERENCED_BY_FRAGMENT_SHADER;
            case ShaderResourceProperty::ReferencedByComputeShader:        return GL_REFERENCED_BY_COMPUTE_SHADER;
            case ShaderResourceProperty::NumCompatibleSubroutines:         return GL_NUM_COMPATIBLE_SUBROUTINES;
            case ShaderResourceProperty::CompatibleSubroutines:            return GL_COMPATIBLE_SUBROUTINES;
            case ShaderResourceProperty::TopLevelArraySize:                return GL_TOP_LEVEL_ARRAY_SIZE;
            case ShaderResourceProperty::TopLevelArrayStride:              return GL_TOP_LEVEL_ARRAY_STRIDE;
            case ShaderResourceProperty::Location:                         return GL_LOCATION;
            case ShaderResourceProperty::LocationIndex:                    return GL_LOCATION_INDEX;
            case ShaderResourceProperty::IsPerPatch:                       return GL_IS_PER_PATCH;
            case ShaderResourceProperty::LocationComponent:                return GL_LOCATION_COMPONENT;
            case ShaderResourceProperty::TransformFeedbackBufferIndex:     return GL_TRANSFORM_FEEDBACK_BUFFER_INDEX;
            case ShaderResourceProperty::TransformFeedbackBufferStride:    return GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE;
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderResourceProperty requested");
                return 0;
        }
    }
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
            default:
                ZEPHYR_ASSERT(false, "Unknown FramebufferTarget requested");
                return 0;
        }
    }
}

std::optional<std::vector<std::string>> GLState::GetErrorMessagesOverride(const GLType::Function& pCallingFunction, const GLType::ErrorType& pErrorType) const
{
    const static std::unordered_map<GLType::Function, std::unordered_map<GLType::ErrorType, std::vector<std::string>>> functionErrorTypeMapping =
    {{
        {GLType::Function::Viewport,
            {
                {GLType::ErrorType::InvalidValue, {"Either width or height is negative"}}
            }
        },
        {GLType::Function::DrawArrays,
            {
                {GLType::ErrorType::InvalidEnum,      {"Mode is not an accepted value"}},
                {GLType::ErrorType::InvalidValue,     {"Count is negative"}},
                {GLType::ErrorType::InvalidOperation, {"Non-zero buffer object name is bound to an enabled array and the buffer object's data store is currently mapped",
                                                       "Geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object"}}
            }
        },
        {GLType::Function::DrawArraysInstanced,
            {
                {GLType::ErrorType::InvalidEnum,      {"mode is not one of GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES GL_LINES_ADJACENCY, GL_LINE_STRIP_ADJACENCY, GL_TRIANGLES_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY or GL_PATCHES"}},
                {GLType::ErrorType::InvalidValue,     {"count or instancecount is negative."}},
                {GLType::ErrorType::InvalidOperation, {"geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object.",
                                                       "a non-zero buffer object name is bound to an enabled array and the buffer object's data store is currently mapped"}}
            }
        },
        {GLType::Function::DrawElements,
            {
                {GLType::ErrorType::InvalidEnum,     {"Mode is not an accepted value"}},
                {GLType::ErrorType::InvalidValue,    {"Count is negative"}},
                {GLType::ErrorType::InvalidOperation,{"Geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object",
                                                      "Non-zero buffer object name is bound to an enabled array or the element array and the buffer object's data store is currently mapped"}}
            }
        },
        {GLType::Function::DrawElementsInstanced,
            {
                {GLType::ErrorType::InvalidEnum,     {"mode is not one of GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, or GL_TRIANGLES."}},
                {GLType::ErrorType::InvalidValue,    {"count or instancecount is negative"}},
                {GLType::ErrorType::InvalidOperation,{"a geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object.",
                                                      "a non-zero buffer object name is bound to an enabled array and the buffer object's data store is currently mapped"}}
            }
        },
        {GLType::Function::BindFramebuffer,
            {
                {GLType::ErrorType::InvalidEnum,      {"Target is not GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER or GL_FRAMEBUFFER"}},
                {GLType::ErrorType::InvalidOperation, {"Framebuffer is not zero or the name of a framebuffer previously returned from a call to glGenFramebuffers"}}
            }
        },
        {GLType::Function::CreateShader,
            {
                {GLType::ErrorType::InvalidEnum, {"pShaderType is not an accepted value"}}
            }
        },
        {GLType::Function::CreateShader,
            {
                {GLType::ErrorType::InvalidValue,     {"pShader is not a value generated by OpenGL", "Count is less than 0"}},
                {GLType::ErrorType::InvalidOperation, {"pShader is not a shader object"}}
            }
        },
        {GLType::Function::CompileShader,
            {
                {GLType::ErrorType::InvalidValue,     {"pShader is not a value generated by OpenGL"}},
                {GLType::ErrorType::InvalidOperation, {"pShader is not a shader object"}}
            }
        },
        {GLType::Function::AttachShader,
            {
                {GLType::ErrorType::InvalidValue,     {"Either program or shader is not a value generated by OpenGL"}},
                {GLType::ErrorType::InvalidOperation, {"Program is not a program object", "Shader is not a shader object", "Shader is already attached to program"}}
            }
        },
        {GLType::Function::LinkProgram,
            {
                {GLType::ErrorType::InvalidValue,     {"Program is not a value generated by OpenGL"}},
                {GLType::ErrorType::InvalidOperation, {"Program is not a program object", "Program is the currently active program object and transform feedback mode is active"}}
            }
        },
        {GLType::Function::DeleteShader,
            {
                {GLType::ErrorType::InvalidValue,     {"Shader is not a value generated by OpenGL"}}
            }
        },
        {GLType::Function::UseProgram,
            {
                {GLType::ErrorType::InvalidValue,     {"Program is neither 0 nor a value generated by OpenGL."}},
                {GLType::ErrorType::InvalidOperation, {"Program is not a program object.", "Program could not be made part of current state.", "Transform feedback mode is active."}}
            }
        },
        {GLType::Function::BindBuffer,
            {
                {GLType::ErrorType::InvalidEnum,  {"Target is not one of the allowable values."}},
                {GLType::ErrorType::InvalidValue, {"Buffer is not a name previously returned from a call to glGenBuffers."}}
            }
        },
        {GLType::Function::DeleteBuffer,
            {
                {GLType::ErrorType::InvalidValue, {"Generated if mHandle is negative."}}
            }
        },
        {GLType::Function::BufferData,
            {
                {GLType::ErrorType::InvalidEnum, {"Target is not one of the accepted buffer targets.", "Usage is not one of the accepted usage types"}},
                {GLType::ErrorType::InvalidValue, {"Size is negative."}},
                {GLType::ErrorType::InvalidOperation, {"Reserved buffer object name 0 is bound to target", "The GL_BUFFER_IMMUTABLE_STORAGE flag of the buffer object is GL_TRUE."}},
                {GLType::ErrorType::OutOfMemory, {"GL is unable to create a data store with the specified size." }}
            }
        },
        {GLType::Function::BufferSubData,
            {
                {GLType::ErrorType::InvalidEnum, {"target is not one of the accepted buffer targets."}},
                {GLType::ErrorType::InvalidOperation, {"zero is bound to target"
                                                     , "any part of the specified range of the buffer object is mapped with glMapBufferRange or glMapBuffer, unless it was mapped with the GL_MAP_PERSISTENT_BIT bit set in the glMapBufferRange access flags."
                                                     , "the value of the GL_BUFFER_IMMUTABLE_STORAGE flag of the buffer object is GL_TRUE and the value of GL_BUFFER_STORAGE_FLAGS for the buffer object does not have the GL_DYNAMIC_STORAGE_BIT bit set."}},
                {GLType::ErrorType::InvalidValue, {"offset or size is negative, or offset + size is greater than the value of GL_BUFFER_SIZE for the specified buffer object."}},
            }
        },
        {GLType::Function::BindBufferRange,
            {
                {GLType::ErrorType::InvalidEnum,      {"Target is not one of GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER."}},
                {GLType::ErrorType::InvalidValue,     {"Index is greater than or equal to the number of target-specific indexed binding points", "Size is less than or equal to zero, or if offset + size is greater than the value of GL_BUFFER_SIZE."}},
                {GLType::ErrorType::InvalidOperation, {"Reserved buffer object name 0 is bound to target", "The GL_BUFFER_IMMUTABLE_STORAGE flag of the buffer object is GL_TRUE."}}
            }
        },
        {GLType::Function::UniformBlockBinding,
            {
                {GLType::ErrorType::InvalidValue, {"uniformBlockIndex is not an active uniform block index of program.", "uniformBlockBinding is greater than or equal to the value of GL_MAX_UNIFORM_BUFFER_BINDINGS.", "program is not the name of a program object generated by the GL"}}
            }
        },
        {GLType::Function::ShaderStorageBlockBinding,
            {
                {GLType::ErrorType::InvalidValue,     {"program is not the name of either a program or shader object", "storageBlockIndex is not an active shader storage block index in program", "storageBlockBinding is greater than or equal to the value of MAX_SHADER_STORAGE_BUFFER_BINDINGS."}},
                {GLType::ErrorType::InvalidOperation, {"program is the name of a shader object." }}
            }
        },
        {GLType::Function::CopyBufferSubData,
            {
                {GLType::ErrorType::InvalidEnum,      {"readTarget or writeTarget is not one of the buffer binding targets listed above."}},
                {GLType::ErrorType::InvalidValue,     {"readOffset, writeOffset or size is negative", "readOffset + size is greater than the size of the source buffer object (its value of GL_BUFFER_SIZE)", "writeOffset + size is greater than the size of the destination buffer object.", "the source and destination are the same buffer object, and the ranges [readOffset, readOffset + size) and [writeOffset, writeOffset + size) overlap."}},
                {GLType::ErrorType::InvalidOperation, {"zero is bound to readTarget or writeTarget.", "either the source or destination buffer object is mapped with glMapBufferRange or glMapBuffer, unless they were mapped with the GL_MAP_PERSISTENT bit set in the glMapBufferRange access flags." }}
            }
        },
    }};

    auto functionIt = functionErrorTypeMapping.find(pCallingFunction);
    if (functionIt != functionErrorTypeMapping.end())
    {
        auto functionErrorMessageIt = functionIt->second.find(pErrorType);
        if (functionErrorMessageIt != functionIt->second.end())
        {
            return functionErrorMessageIt->second;
        }
    }
    return std::nullopt;
}

std::string GLState::getErrorMessage() const
{
    std::set<GLType::ErrorType> errors;
    GLenum glError(glGetError());
    while (glError != GL_NO_ERROR)
    {
        switch (glError)
        {
            case GL_INVALID_OPERATION:             errors.insert(GLType::ErrorType::InvalidOperation);             break;
            case GL_INVALID_ENUM:                  errors.insert(GLType::ErrorType::InvalidEnum);                  break;
            case GL_INVALID_VALUE:                 errors.insert(GLType::ErrorType::InvalidValue);                 break;
            case GL_OUT_OF_MEMORY:                 errors.insert(GLType::ErrorType::OutOfMemory);                  break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errors.insert(GLType::ErrorType::InvalidFramebufferOperation);  break;
        }

        glError = glGetError();
    }

    if (errors.empty())
        return "";

    std::string errorString = "Found OpenGL error(s):";
    for (const auto& error : errors)
        errorString += "\n" + GLType::ToDefaultErrorMessage(error);

    return errorString;
}

std::string GLState::getErrorMessage(const GLType::Function& pCallingFunction) const
{
    std::set<GLType::ErrorType> errors;
    GLenum glError(glGetError());
    while (glError != GL_NO_ERROR)
    {
        switch (glError)
        {
            case GL_INVALID_OPERATION:             errors.insert(GLType::ErrorType::InvalidOperation);             break;
            case GL_INVALID_ENUM:                  errors.insert(GLType::ErrorType::InvalidEnum);                  break;
            case GL_INVALID_VALUE:                 errors.insert(GLType::ErrorType::InvalidValue);                 break;
            case GL_OUT_OF_MEMORY:                 errors.insert(GLType::ErrorType::OutOfMemory);                  break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errors.insert(GLType::ErrorType::InvalidFramebufferOperation);  break;
        }

        glError = glGetError();
    }

    if (errors.empty())
        return "";

    std::string errorString = "Found OpenGL error(s) using function gl" + GLType::toString(pCallingFunction) + ":";
    for (const auto& error : errors)
    {
        auto overrideMessages = GetErrorMessagesOverride(pCallingFunction, error);

        if (overrideMessages.has_value())
        {
            for (const auto& message : overrideMessages.value())
                errorString += "\n" + message;
        }
        else
            errorString += "\n" + GLType::ToDefaultErrorMessage(error);
    }
    return errorString;
}