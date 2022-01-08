#pragma once

#include "string"
#include "array"
#include "glm/fwd.hpp"

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
    enum class Attribute : unsigned int { Position3D, Normal3D, ColourRGB, TextureCoordinate2D, Count }; // The possible vertex attributes supported by OpenGLAPI GLSL shaders.

    int getAttributeLocation(const Attribute& pAttribute) const;       // Returns the location value of a specified attribute. This is specified as layout (location = 0) in GLSL shaders.
    int getAttributeComponentCount(const Attribute& pAttribute) const; // Returns the number of components attribute consists of. E.g. vec3 returns 3 as its composed of 3 components (X, Y and Z)

    void use() const; // Set this shader as the currently active one in OpenGL state. Necessary to call before setUniform.
    void setUniform(const std::string& pName, const bool& pValue);
    void setUniform(const std::string& pName, const int& pValue);
    void setUniform(const std::string& pName, const glm::mat4& pValue);

private:
    int getUniformLocation(const std::string &pName);
    void load();

    std::string mName;
    std::string mSourcePath;
    unsigned int mHandle;

    static inline const Shader* shaderInUse = nullptr; // Keeps track of current Shader object set to use by OpenGL state. Used for error checking in checkForUseErrors().
    enum class Type { Vertex, Fragment, Program };

    static std::string getAttributeName(const Attribute &pAttribute); // Returns the attribute as a string matching the naming used within GLSL shaders.
    static bool checkForUseErrors(const Shader &pCalledFrom);
    static bool hasCompileErrors(const Type &pType, const unsigned int pID);
};