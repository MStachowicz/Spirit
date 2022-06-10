#include "GLState.hpp"

#include "Logger.hpp"

#include "glad/gl.h"
#include "imgui.h"

#include "glm/mat4x4.hpp" // mat4, dmat4
#include "glm/gtc/type_ptr.hpp" //  glm::value_ptr

#include "set"

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
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DrawElements));
}
void GLState::drawArrays(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount)
{
    glDrawArrays(convert(pPrimitiveMode), 0, pCount);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DrawArrays));
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
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::DeleteShader));
}

int GLState::GetUniformLocation(const unsigned int& pShaderProgramHandle, const std::string& pName)
{
    GLint location = glGetUniformLocation(pShaderProgramHandle, pName.c_str());
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::GetUniformLocation));
    ZEPHYR_ASSERT(location != -1, "pName does not correspond to an active uniform variable in program or pName starts with the reserved prefix 'gl_' or pName is associated with an atomic counter or a named uniform block.");
    return location;
}

void GLState::bindUniformBlock(GLData::UniformBlock& pUniformBlock)
{
    UniformBlockBindingPoint* bindingPoint = nullptr;

    // #C++20 Convert to std::contains on vector.
    const auto it = std::find_if(mUniformBlockBindingPoints.begin(), mUniformBlockBindingPoints.end(), [&pUniformBlock](const UniformBlockBindingPoint& bindingPoint)
    { return bindingPoint.mName == pUniformBlock.mName; });

    if (it == mUniformBlockBindingPoints.end())
    {
        // If there is no binding point for this UniformBlock create it and its UBO.
        // This allows the uniform variables inside the block to be set using setUniformBlockVariable()
        // This makes the uniform block share resource with all other matching blocks as long as they use the same mBindingPoint and have matching interfaces.
        mUniformBlockBindingPoints.push_back(UniformBlockBindingPoint());
        bindingPoint = &mUniformBlockBindingPoints.back();
        bindingPoint->mInstances++;
        bindingPoint->mBindingPoint  = static_cast<unsigned int>(mUniformBlockBindingPoints.size() - 1);
        bindingPoint->mName          = pUniformBlock.mName;
        bindingPoint->mVariables     = pUniformBlock.mVariables;

        bindingPoint->mUBO.generate();
        bindingPoint->mUBO.bind();
        // Reserve the size of the UniformBlock in the GPU memory
        auto dataSize = pUniformBlock.mBufferDataSize;
        glBufferData(GL_UNIFORM_BUFFER, dataSize, NULL, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint->mBindingPoint, bindingPoint->mUBO.getHandle(), 0, dataSize);
    }
    else
    {
        // If the UniformBlock has been encountered before. We bind it to the same mBindingPoint of the previously
        // created UniformBlockBufferBacking meaning the blocks share the GPU memory.
        bindingPoint = &(*it);
        bindingPoint->mInstances++;
        bindingPoint->mUBO.bind();
    }

    ZEPHYR_ASSERT(bindingPoint != nullptr, "Could not find a valid binding point for Uniform Block '{}'", pUniformBlock.mName);
    pUniformBlock.mBindingPoint = bindingPoint->mBindingPoint;

    glUniformBlockBinding(pUniformBlock.mParentShaderHandle, pUniformBlock.mBlockIndex, bindingPoint->mBindingPoint);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::UniformBlockBinding));
}

void GLState::setBlockUniform(const GLData::UniformVariable& pVariable, const float& pValue)
{
	ZEPHYR_ASSERT(pVariable.mType == GLType::DataType::Float, "Attempting to set float data to {} variable '{}' (uniform block variable)", GLType::toString(pVariable.mType), pVariable.mName);
	static const auto size = sizeof(pValue);
	glBufferSubData(GL_UNIFORM_BUFFER, pVariable.mOffset, size, &pValue);
}
void GLState::setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec2& pValue)
{
	ZEPHYR_ASSERT(pVariable.mType == GLType::DataType::Vec2, "Attempting to set vec2 data to {} variable '{}' (uniform block variable)", GLType::toString(pVariable.mType), pVariable.mName);
	static const auto size = sizeof(pValue);
	glBufferSubData(GL_UNIFORM_BUFFER, pVariable.mOffset, size, glm::value_ptr(pValue));
}
void GLState::setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec3& pValue)
{
	ZEPHYR_ASSERT(pVariable.mType == GLType::DataType::Vec3, "Attempting to set vec3 data to {} variable '{}' (uniform block variable)", GLType::toString(pVariable.mType), pVariable.mName);
	static const auto size = sizeof(pValue);
	glBufferSubData(GL_UNIFORM_BUFFER, pVariable.mOffset, size, glm::value_ptr(pValue));
}
void GLState::setBlockUniform(const GLData::UniformVariable& pVariable, const glm::vec4& pValue)
{
	ZEPHYR_ASSERT(pVariable.mType == GLType::DataType::Vec4, "Attempting to set vec4 data to {} variable '{}' (uniform block variable)", GLType::toString(pVariable.mType), pVariable.mName);
	static const auto size = sizeof(pValue);
	glBufferSubData(GL_UNIFORM_BUFFER, pVariable.mOffset, size, glm::value_ptr(pValue));
}
void GLState::setBlockUniform(const GLData::UniformVariable& pVariable, const glm::mat4& pValue)
{
	ZEPHYR_ASSERT(pVariable.mType == GLType::DataType::Mat4, "Attempting to set mat4 data to {} variable '{}' (uniform block variable)", GLType::toString(pVariable.mType), pVariable.mName);
	static const auto size = sizeof(pValue);
	glBufferSubData(GL_UNIFORM_BUFFER, pVariable.mOffset, size, glm::value_ptr(pValue));
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

GLData::UniformVariable GLState::getUniformVariable(const unsigned int& pShaderProgramHandle, const unsigned int& pUniformVariableIndex) const
{
    // Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
    // https://www.khronos.org/opengl/wiki/Program_Introspection

    static const std::array<GLenum, 9> propertyQuery = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_LOCATION, GL_BLOCK_INDEX, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR};

    std::array<GLint, propertyQuery.size()> propertyValues = {0};
    glGetProgramResourceiv(pShaderProgramHandle, GL_UNIFORM, pUniformVariableIndex, static_cast<GLsizei>(propertyQuery.size()), propertyQuery.data(), static_cast<GLsizei>(propertyValues.size()), NULL, propertyValues.data());

    GLData::UniformVariable uniformVariable;
    uniformVariable.mName.resize(propertyValues[0]);
    glGetProgramResourceName(pShaderProgramHandle, GL_UNIFORM, pUniformVariableIndex, propertyValues[0], NULL, uniformVariable.mName.data());
    ZEPHYR_ASSERT(!uniformVariable.mName.empty(), "Failed to get name of uniform variable in shader with handle {}", pShaderProgramHandle);
    uniformVariable.mName.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

    uniformVariable.mType         = GLType::convert(propertyValues[1]);
	uniformVariable.mOffset       = propertyValues[2];
	uniformVariable.mLocation     = propertyValues[3];
	uniformVariable.mBlockIndex   = propertyValues[4];
	uniformVariable.mArraySize    = propertyValues[5];
	uniformVariable.mArrayStride  = propertyValues[6];
	uniformVariable.mMatrixStride = propertyValues[7];
	uniformVariable.mIsRowMajor   = propertyValues[8];
    return uniformVariable;
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
    uniformBlock.mBlockIndex           = glGetUniformBlockIndex(pShaderProgramHandle, uniformBlock.mName.c_str());
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
            uniformBlock.mVariables.push_back(getUniformVariable(pShaderProgramHandle, uniformBlock.mVariableIndices[variableIndex]));
    }

    return uniformBlock;
}

void GLState::setViewport(const int& pWidth, const int& pHeight)
{
    mViewport[2] = pWidth;
    mViewport[3] = pHeight;
    glViewport(0, 0, pWidth, pHeight);
    ZEPHYR_ASSERT_MSG(getErrorMessage(GLType::Function::Viewport));
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
        ZEPHYR_ASSERT(mInitialised, "EBO has not been generated before pushData, call generate before pushData");
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pIndexData.size() * sizeof(int), &pIndexData.front(), GL_STATIC_DRAW);
    }
    void EBO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised UBO");
        glDeleteBuffers(1, &mHandle);
        mInitialised = false;
    }
     void UBO::generate()
    {
        ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated UBO")
        glGenBuffers(1, &mHandle);
        mInitialised = true;
    }
    void UBO::bind() const
    {
        ZEPHYR_ASSERT(mInitialised, "UBO has not been generated before bind, call generate before bind");
        glBindBuffer(GL_UNIFORM_BUFFER, mHandle);
    }
    void UBO::release()
    {
        ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised UBO");
        glDeleteBuffers(1, &mHandle);
        mInitialised = false;
    }
    void UBO::pushData(const int &pSize, const int &pUniformIndex)
    {
        ZEPHYR_ASSERT(mInitialised, "UBO has not been generated before pushData, call generate before pushData");
        // Reserve the memory for the data size with glBufferData then define the
        // range of the buffer that links to a uniform binding point with glBindBufferRange
        glBufferData(GL_UNIFORM_BUFFER, pSize, NULL, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, pUniformIndex, mHandle, 0, pSize);
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
            case DataType::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown DataType requested");
                return 0;
        }
    }

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
                return DataType::Count;
        }
    }

    std::string toString(const Function& pFunction)
    {
        switch (pFunction)
        {
            case Function::UniformBlockBinding : return "UniformBlockBinding";
            case Function::Viewport :            return "Viewport";
            case Function::DrawElements :        return "DrawElements";
            case Function::DrawArrays :          return "DrawArrays";
            case Function::BindFramebuffer :     return "BindFramebuffer";
            case Function::CreateShader :        return "CreateShader";
            case Function::ShaderSource :        return "ShaderSource";
            case Function::CompileShader :       return "CompileShader";
            case Function::CreateProgram :       return "CreateProgram";
            case Function::AttachShader :        return "AttachShader";
            case Function::LinkProgram :         return "LinkProgram";
            case Function::DeleteShader :        return "DeleteShader";
            case Function::UseProgram :          return "UseProgram";
            case Function::GetUniformLocation :  return "GetUniformLocation";
            default:
                ZEPHYR_ASSERT(false, "Unknown Function requested");
                return "";
        }
    }

    std::string toString(const ShaderProgramType& pShaderProgramType)
    {
        switch (pShaderProgramType)
        {
            case ShaderProgramType::Vertex:     return "VertexShader";
            case ShaderProgramType::Geometry:   return "GeometryShader";
            case ShaderProgramType::Fragment:   return "FragmentShader";
            case ShaderProgramType::Count:
            default:
                ZEPHYR_ASSERT(false, "Unknown ShaderProgramType requested");
                return "";
        }
    }
    int convert(const ShaderProgramType& pShaderProgramType)
    {
        switch (pShaderProgramType)
        {
            case ShaderProgramType::Vertex:     return GL_VERTEX_SHADER;
            case ShaderProgramType::Geometry:   return GL_GEOMETRY_SHADER;
            case ShaderProgramType::Fragment:   return GL_FRAGMENT_SHADER;
            case ShaderProgramType::Count:
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

            case ShaderResourceType::Count:
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

            case ShaderResourceProperty::Count:
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

std::string GLState::getErrorMessage()
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
        errorString += "\n" + GLType::toString(error);

    return errorString;
}

std::string GLState::getErrorMessage(const GLType::Function& pCallingFunction)
{
    const static std::array<std::unordered_map<GLType::ErrorType, std::vector<std::string>>, util::toIndex(GLType::Function::Count)> functionErrorTypeMapping =
    {{
        { // UniformBlockBinding
            { GLType::ErrorType::InvalidValue, {
                "uniformBlockIndex is not an active uniform block index of program"
                , "uniformBlockBinding is greater than or equal to the value of GL_MAX_UNIFORM_BUFFER_BINDINGS"
                , "program is not the name of a program object generated by the GL"
            }}
        },
        { // Viewport
            { GLType::ErrorType::InvalidValue,     { "Either width or height is negative" }}
        },
        { // DrawElements
            { GLType::ErrorType::InvalidEnum,      {"Mode is not an accepted value"}},
            { GLType::ErrorType::InvalidValue,     {"Count is negative"}},
            { GLType::ErrorType::InvalidOperation, {
                "Geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object",
                "Non-zero buffer object name is bound to an enabled array or the element array and the buffer object's data store is currently mapped"
            }}
        },
        { // DrawArrays
            { GLType::ErrorType::InvalidEnum,      { "Mode is not an accepted value" }},
            { GLType::ErrorType::InvalidValue,     {"Count is negative" }},
            { GLType::ErrorType::InvalidOperation, {
                "Non-zero buffer object name is bound to an enabled array and the buffer object's data store is currently mapped",
                "Geometry shader is active and mode is incompatible with the input primitive type of the geometry shader in the currently installed program object"
            }}
        },
        { // BindFramebuffer
            { GLType::ErrorType::InvalidEnum,      { "Target is not GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER or GL_FRAMEBUFFER" }},
            { GLType::ErrorType::InvalidOperation, { "Framebuffer is not zero or the name of a framebuffer previously returned from a call to glGenFramebuffers" }}
        },
        { // CreateShader
            { GLType::ErrorType::InvalidEnum,      { "pShaderType is not an accepted value" }}
        },
        { // ShaderSource
            { GLType::ErrorType::InvalidValue,      { "pShader is not a value generated by OpenGL", "Count is less than 0" }},
            { GLType::ErrorType::InvalidOperation,  { "pShader is not a shader object" }}
        },
        { // CompileShader
            { GLType::ErrorType::InvalidValue,      { "pShader is not a value generated by OpenGL" }},
            { GLType::ErrorType::InvalidOperation,  { "pShader is not a shader object" }}
        },
        {}, // CreateProgram
        { // AttachShader
            { GLType::ErrorType::InvalidValue,      { "Either program or shader is not a value generated by OpenGL" }},
            { GLType::ErrorType::InvalidOperation,  { "Program is not a program object", "Shader is not a shader object", "Shader is already attached to program" }}
        },
        { // LinkProgram
            { GLType::ErrorType::InvalidValue,      { "Program is not a value generated by OpenGL" }},
            { GLType::ErrorType::InvalidOperation,  { "Program is not a program object", "Program is the currently active program object and transform feedback mode is active" }}
        },
        { // DeleteShader
            { GLType::ErrorType::InvalidValue,      { "Shader is not a value generated by OpenGL" }}
        },
        { // UseProgram
            { GLType::ErrorType::InvalidValue,      { "Program is neither 0 nor a value generated by OpenGL." }},
            { GLType::ErrorType::InvalidOperation,  { "Program is not a program object.", "Program could not be made part of current state.", "Transform feedback mode is active." }}
        },
        { // GetUniformLocation
            { GLType::ErrorType::InvalidValue,      { "program is not a value generated by OpenGL." }},
            { GLType::ErrorType::InvalidOperation,  { "Program is not a program object.", "Program has not been successfully linked." }}
        },
    }};

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
        const auto& errorMessageOverrides = functionErrorTypeMapping[util::toIndex(pCallingFunction)];
        const auto errorTypeIterator = errorMessageOverrides.find(error);

        if (errorTypeIterator != errorMessageOverrides.end())
        {
            for (const auto &errorMessage : errorTypeIterator->second)
            {
                errorString += "\n" + errorMessage;
            }
        }
        else
        {// If functionErrorTypeMapping doesn't override the ErrorType, use the default toString
            errorString += "\n" + GLType::toString(error);
        }
    }
    return errorString;
}