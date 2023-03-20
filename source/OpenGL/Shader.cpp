#include "Shader.hpp"

#include "Logger.hpp"
#include "Utility.hpp"
#include "File.hpp"

#include <sstream>

namespace OpenGL
{
    Shader::Shader(const std::string& pName, GLState& pGLState)
        : mName(pName)
        , mIsInstanced(false)
        , mTextureUnits(0)
    {
        const auto shaderPath = Utility::File::GLSLShaderDirectory / mName;

        unsigned int vertexShader;
        auto vertexShaderPath = shaderPath;
        vertexShaderPath.replace_extension("vert");
        ASSERT(Utility::File::exists(vertexShaderPath), "Vertex shader does not exist at path '{}'", vertexShaderPath.string());
        std::string vertexSource = Utility::File::readFromFile(vertexShaderPath);
        {
            vertexShader = pGLState.CreateShader(GLType::ShaderProgramType::Vertex);
            pGLState.ShaderSource(vertexShader, vertexSource);
            pGLState.CompileShader(vertexShader);
            scanForAttributes(vertexSource);
        }

        unsigned int fragmentShader;
        {
            auto fragmentShaderPath = shaderPath;
            fragmentShaderPath.replace_extension("frag");
            ASSERT(Utility::File::exists(fragmentShaderPath), "Fragment shader does not exist at path {}", fragmentShaderPath.string());
            fragmentShader     = pGLState.CreateShader(GLType::ShaderProgramType::Fragment);
            std::string source = Utility::File::readFromFile(fragmentShaderPath);
            pGLState.ShaderSource(fragmentShader, source);
            pGLState.CompileShader(fragmentShader);
        }

        std::optional<unsigned int> geometryShader;
        {
            auto geomShaderPath = shaderPath;
            geomShaderPath.replace_extension("geom");
            if (Utility::File::exists(shaderPath))
            {
                geometryShader     = pGLState.CreateShader(GLType::ShaderProgramType::Geometry);
                std::string source = Utility::File::readFromFile(shaderPath);
                pGLState.ShaderSource(geometryShader.value(), source);
                pGLState.CompileShader(geometryShader.value());
            }
        }

        {
            mHandle = pGLState.CreateProgram();
            pGLState.AttachShader(mHandle, vertexShader);
            pGLState.AttachShader(mHandle, fragmentShader);
            if (geometryShader.has_value())
                pGLState.AttachShader(mHandle, geometryShader.value());

            pGLState.LinkProgram(mHandle);
        }

        { // Setup all the unform variables for the linked shader program using OpenGL program introspection
            { // UniformBlock setup
                const int blockCount = pGLState.getActiveUniformBlockCount(mHandle);
                for (int blockIndex = 0; blockIndex < blockCount; blockIndex++)
                {
                    mUniformBlocks.push_back(pGLState.getUniformBlock(mHandle, blockIndex));
                    pGLState.RegisterUniformBlock(mUniformBlocks.back());
                }
            }
            { // Loose UniformVariable setup
                const int uniformCount = pGLState.getActiveUniformCount(mHandle);
                for (int uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
                {
                    GLData::UniformVariable uniform = GLData::UniformVariable(mHandle, uniformIndex);

                    // Only add 'loose' uniforms, uniform block variables are handled in GLState::getUniformBlock
                    if (uniform.mBlockIndex == -1)
                        mUniformVariables.push_back(uniform);
                }
            }

            { // Setup available Shader buffer objects.
                const int blockCount = pGLState.getShaderStorageBlockCount(mHandle);
                for (int blockIndex = 0; blockIndex < blockCount; blockIndex++)
                {
                    mShaderBufferBlocks.push_back(pGLState.getShaderStorageBlock(mHandle, blockIndex));
                    mShaderBufferBlocks.back().mShared = isBufferShared(mShaderBufferBlocks.back().mName, vertexSource);
                    pGLState.RegisterShaderStorageBlock(mShaderBufferBlocks.back());
                }
            }

            {
                for (const auto& uniformBlock : mUniformBlocks)
                {
                    for (const auto& uniformBlockVariable : uniformBlock.mVariables)
                    {
                        if (uniformBlockVariable.mDataType == GLType::DataType::Sampler2D || uniformBlockVariable.mDataType == GLType::DataType::SamplerCube)
                            mTextureUnits++;
                    }
                }
                for (const auto& uniformVariable : mUniformVariables)
                {
                    if (uniformVariable.mDataType == GLType::DataType::Sampler2D || uniformVariable.mDataType == GLType::DataType::SamplerCube)
                        mTextureUnits++;
                }
                ASSERT(mTextureUnits <= maxTextureUnits, "Texture units available must be below the max.");
            }

            if (vertexSource.find(".models[gl_InstanceID]") != std::string::npos)
            {
                for (auto& shaderBlock : mShaderBufferBlocks)
                {
                    auto variable = std::find_if(std::begin(shaderBlock.mVariables), std::end(shaderBlock.mVariables), [](const auto& pVariable)
                                                  { return pVariable.mName.value() == "InstancedData.models[0]"; });

                    if (variable != shaderBlock.mVariables.end())
                    {
                        mIsInstanced = true;
                        break;
                    }
                }
            }
        }

        { // Setup the available texture units.
            // We have to tell OpenGL which texture unit each shader 'uniform sampler2D' belongs to by setting each sampler using glUniform1i.
            // We only have to set this once.
            if (mTextureUnits > 0)
            {
                use(pGLState);
                for (int j = 0; j < mTextureUnits; j++)
                {
                    const std::string textureUniformName = "texture" + std::to_string(j);
                    setUniform(pGLState, textureUniformName, j);
                }
            }
        }

        // Delete the shaders after linking as they're no longer needed
        pGLState.DeleteShader(vertexShader);
        pGLState.DeleteShader(fragmentShader);
        if (geometryShader.has_value())
            pGLState.DeleteShader(geometryShader.value());

        LOG_INFO("OpenGL::Shader '{}' loaded given ID: {}", mName, mHandle);
    }

    void Shader::scanForAttributes(const std::string& pSourceCode)
    {
        if (mAttributes.find(Attribute::Position3D) == mAttributes.end())
            if (pSourceCode.find(getAttributeName(Attribute::Position3D)) != std::string::npos)
                mAttributes.insert(Attribute::Position3D);

        if (mAttributes.find(Attribute::Normal3D) == mAttributes.end())
            if (pSourceCode.find(getAttributeName(Attribute::Normal3D)) != std::string::npos)
                mAttributes.insert(Attribute::Normal3D);

        if (mAttributes.find(Attribute::ColourRGB) == mAttributes.end())
            if (pSourceCode.find(getAttributeName(Attribute::ColourRGB)) != std::string::npos)
                mAttributes.insert(Attribute::ColourRGB);

        if (mAttributes.find(Attribute::TextureCoordinate2D) == mAttributes.end())
            if (pSourceCode.find(getAttributeName(Attribute::TextureCoordinate2D)) != std::string::npos)
                mAttributes.insert(Attribute::TextureCoordinate2D);

        ASSERT(!mAttributes.empty() && mAttributes.size() <= Utility::toIndex(Attribute::Count), "{} is not a valid number of attributes for a shader.", mAttributes.size());
    }

    void Shader::use(GLState& pGLState) const
    {
        pGLState.UseProgram(mHandle);
    }

    int Shader::getAttributeLocation(const Attribute& pAttribute)
    {
        int location = static_cast<int>(Utility::toIndex(pAttribute));
        return location;

        // Non static version of getAttributeLocation, not necessary as attribute locations are
        // consistent across all Zephyr GLSL shaders.

        // const GLint location = glGetAttribLocation(mHandle, getAttributeName(pAttribute).c_str());
        // ASSERT(location != -1, "Failed to find the location of {} in shader {}.", getAttributeName(pAttribute).c_str(), mName);
        // return static_cast<int>(location);
    }

    int Shader::getAttributeComponentCount(const Attribute& pAttribute)
    {
        switch (pAttribute)
        {
            case Attribute::Position3D:
                return 3; // X, Y and Z position components
            case Attribute::Normal3D:
                return 3; // X, Y and Z direction components
            case Attribute::ColourRGB:
                return 3; // Red, Green and Blue components
            case Attribute::TextureCoordinate2D:
                return 2; // X and Y components
            default:
                ASSERT(false, "Could not determine the size of the attribute pAttribute");
                return 0;
        }
    }

    std::string Shader::getAttributeName(const Attribute& pAttribute)
    {
        if (pAttribute == Attribute::Position3D)
            return "VertexPosition";
        else if (pAttribute == Attribute::Normal3D)
            return "VertexNormal";
        else if (pAttribute == Attribute::ColourRGB)
            return "VertexColour";
        else if (pAttribute == Attribute::TextureCoordinate2D)
            return "VertexTexCoord";
        else
            ASSERT(false, "Could not convert Shader::Attribute '{}' to an std::string", pAttribute);
        return "";
    }

    bool Shader::isBufferShared(const std::string& pShaderStorageBlockName, const std::string& pSourceCode)
    {
        std::istringstream ss(pSourceCode);
        std::string line;

        while (std::getline(ss, line))
        {
            auto find = line.find(pShaderStorageBlockName);
            if (find != std::string::npos)
            {
                if (line.find("layout(shared)") != std::string::npos)
                    return true;
                break;
            }
        }

        return false;
    }
} // namespace OpenGL