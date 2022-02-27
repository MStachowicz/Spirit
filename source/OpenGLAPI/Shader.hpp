#pragma once

#include "glm/fwd.hpp"

#include "string"
#include "set"

// Handles the loading of GLSL shaders from file.
// Provides functions to set GLSL uniform variables.
// Provides functions to find the locations of vertex attributes and their descriptions.
class Shader
{
public:
    Shader(const std::string &pName);

    template <typename T>
	static constexpr auto toIndex(T pEnum) noexcept // Returns the underlying type. Used to convert Shader::Attributes to indexes into arrays in mMeshGPUDataManager
	{
		return static_cast<std::underlying_type_t<T>>(pEnum);
	}

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

    const std::set<Shader::Attribute>& getRequiredAttributes() const { return mRequiredAttributes; };
    const std::string& getName() const { return mName; };
    const int& getTexturesUnitsCount() const { return mTextureUnits; };

    void use() const; // Set this shader as the currently active one in OpenGL state. Necessary to call before setUniform.
    void setUniform(const std::string& pName, const bool& pValue) const;
    void setUniform(const std::string& pName, const int& pValue) const;
    void setUniform(const std::string& pName, const float& pValue) const;
    void setUniform(const std::string& pName, const glm::vec2& pValue) const;
    void setUniform(const std::string& pName, const glm::vec3& pValue) const;
    void setUniform(const std::string& pName, const glm::vec4& pValue) const;
    void setUniform(const std::string& pName, const glm::mat2& pValue) const;
    void setUniform(const std::string& pName, const glm::mat3& pValue) const;
    void setUniform(const std::string& pName, const glm::mat4& pValue) const;

    // Returns the number of components the specified attribute consists of.
    // E.g. "vec3" in GLSL shaders would return 3 as it's composed of 3 components (X, Y and Z)
    static int getAttributeComponentCount(const Attribute& pAttribute);
    // Returns the location of a specified attribute type.
    // All shaders repeat the same attribute layout positions so this is a static function.
    // Specified as "layout (location = X)" in GLSL shaders.
    static int getAttributeLocation(const Attribute& pAttribute);
private:
    void load();
    // Search source code for any per-vertex attributes a Mesh will require to be drawn by this shader.
    void initialiseRequiredAttributes(const std::string& pSourceCode);
    int getUniformLocation(const std::string &pName) const;

    std::string mName;
    std::string mSourcePath;
    unsigned int mHandle;
    std::set<Shader::Attribute> mRequiredAttributes; // The required Attributes a mesh must have to be rendered using this Shader.
    int mTextureUnits;

    static inline const Shader* shaderInUse = nullptr; // Keeps track of current Shader object set with use(). Used for error checking in checkForUseErrors().
	static inline const size_t maxTextureUnits = 2; // The limit on the number of texture units available in the shaders using sampler2D
    enum class Type { Vertex, Fragment, Program };

    static int findOccurrences(const std::string& pStringToSearch, const std::string& pSubStringToFind);
    static std::string getAttributeName(const Attribute &pAttribute); // Returns the attribute as a string matching the naming used within GLSL shaders.
    static bool checkForUseErrors(const Shader &pCalledFrom);
    static bool hasCompileErrors(const Type &pType, const unsigned int pID);
};