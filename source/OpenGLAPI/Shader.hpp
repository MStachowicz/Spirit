#pragma once

#include "string"
#include "glm/fwd.hpp"

// Handles the loading of GLSL shaders from file.
// Provides functions to set GLSL uniform variables.
// Provides functions to find the locations of vertex attributes and their descriptions.
class Shader
{
public:
    Shader(const std::string &pName);


    enum class Attribute { Position3D, Normal3D, ColourRGB, TextureCoordinate2D }; // The possible vertex attributes supported by OpenGLAPI GLSL shaders.
    int getAttributeLocation(const Attribute& pAttribute) const;       // Returns the location value of a specified attribute. This is specified as layout (location = 0) in GLSL shaders.
    int getAttributeComponentCount(const Attribute& pAttribute) const; // Returns the number of components attribute consists of. E.g. vec3 returns 3 as its composed of 3 components (X, Y and Z)

    void use(); // Set this shader as the currently active one in OpenGL state. Necessary to call before setUniform.
    void setUniform(const std::string& pName, const bool& pValue);
    void setUniform(const std::string& pName, const int& pValue);
    void setUniform(const std::string& pName, const glm::mat4& pValue);

private:
    int getUniformLocation(const std::string &pName);
    void load();

    std::string mName;
    std::string mSourcePath;
    unsigned int mHandle;

    static inline Shader *shaderInUse = nullptr; // Keeps track of current Shader object set to use by OpenGL state. Used for error checking in checkForUseErrors().
    enum class Type { Vertex, Fragment, Program };

    static std::string getAttributeName(const Attribute &pAttribute); // Returns the attribute as a string matching the naming used within GLSL shaders.
    static bool checkForUseErrors(const Shader &pCalledFrom);
    static bool hasCompileErrors(const Type &pType, const unsigned int pID);
};