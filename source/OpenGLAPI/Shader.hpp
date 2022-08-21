#pragma once

#include "GLState.hpp"

#include <string>
#include <set>
#include <algorithm>

// Handles the loading of GLSL shaders from file.
// Provides functions to set GLSL uniform variables.
// Provides functions to find the locations of vertex attributes and their descriptions.
class Shader
{
public:
    Shader(const std::string& pName, GLState& pGLState);

    // These are per-vertex attributes found in GLSL shaders.
    // Each attribute must be named the same in the GLSL files. getAttributeName() returns the expected string in the shader.
    // Each attribute must be in the same location in all shaders, specified as "layout (location = X)"". getAttributeLocation() returns the location X.
    enum class Attribute : unsigned int
    {
        Position3D              = 0,
        Normal3D                = 1,
        ColourRGB               = 2,
        TextureCoordinate2D     = 3,
        Count
    }; // The possible vertex attributes supported by OpenGLAPI GLSL shaders.

    const std::string& getName()       const { return mName; };
    bool isInstanced() const { return mIsInstanced; };
    const int& getTexturesUnitsCount() const { return mTextureUnits; };

    void use(GLState& pGLState) const; // Set this shader as the currently active one in OpenGL state. Necessary to call before setUniform.

    template<class T>
    void setUniform(GLState& pGLState, const std::string& pVariableName, const T& pValue)
    {
        auto& variable = std::find_if(std::begin(mUniformVariables), std::end(mUniformVariables),
            [&pVariableName](const GLData::UniformVariable& pVariable) { return pVariable.mName.value() == pVariableName; });

        if (variable != std::end(mUniformVariables))
        {
            use(pGLState); // #Optimisation - Only perform this use() once when setting a series of variables on one shader.
            variable->Set(pGLState, pValue);
            return;
        }
        else
            ZEPHYR_ASSERT(false, "Uniform variable '{}' not found in shader '{}'", pVariableName, mName);
    }

    GLData::ShaderStorageBlockVariable* getShaderBlockVariable(const std::string& pVariableName)
    {
        for (auto& shaderBlock : mShaderBufferBlocks)
        {
            auto& variable = std::find_if(std::begin(shaderBlock.mVariables), std::end(shaderBlock.mVariables),
                [&pVariableName](const GLData::ShaderStorageBlockVariable& pVariable) { return pVariable.mName.value() == pVariableName; });

            if (variable != shaderBlock.mVariables.end())
                return &(*variable);
        }

        ZEPHYR_ASSERT(false, "ShaderStorageBlockVariable '{}' not found in shader '{}'", pVariableName, mName);
        return nullptr;
    }

    // Returns the number of components the specified attribute consists of.
    // E.g. "vec3" in GLSL shaders would return 3 as it's composed of 3 components (X, Y and Z)
    static int getAttributeComponentCount(const Attribute& pAttribute);
    // Returns the location of a specified attribute type.
    // All shaders repeat the same attribute layout positions so this is a static function.
    // Specified as "layout (location = X)" in GLSL shaders.
    static int getAttributeLocation(const Attribute& pAttribute);
private:
    // Search source code for any per-vertex attributes a Mesh will require to be drawn by this shader.
    void scanForAttributes(const std::string& pSourceCode);

    std::string mName;
    std::string mSourcePath;
    unsigned int mHandle;
    bool mIsInstanced;
    int mTextureUnits; // The number of available textures to the shader. Found in shader file as 'uniform sampler2D textureX'
    std::set<Shader::Attribute> mAttributes; // The vertex attributes the shader requires to execute a draw call.
    // TODO: make this an array of size GL_MAX_X_UNIFORM_BLOCKS + (X = Split the uniform blocks per shader stage)
    // Each shader stage has a limit on the number of separate uniform buffer binding locations. These are queried using
    // glGetIntegerv with GL_MAX_VERTEX_UNIFORM_BLOCKS, GL_MAX_GEOMETRY_UNIFORM_BLOCKS, or GL_MAX_FRAGMENT_UNIFORM_BLOCKS.
    std::vector<GLData::UniformBlock> mUniformBlocks;
    // All the 'loose' uniform variables that exist in the shader. These do not belong to UniformBlock's.
    std::vector<GLData::UniformVariable> mUniformVariables;
    std::vector<GLData::ShaderStorageBlock> mShaderBufferBlocks;

    static bool isBufferShared(const std::string& pShaderStorageBlockName, const std::string& pSourceCode);
    static inline const size_t maxTextureUnits = 2; // The limit on the number of texture units available in the shaders
    static std::string getAttributeName(const Attribute &pAttribute); // Returns the attribute as a string matching the naming used within GLSL shaders.
};