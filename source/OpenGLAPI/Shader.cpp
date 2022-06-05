#include "Shader.hpp"

#include "FileSystem.hpp"
#include "Utility.hpp"
#include "Logger.hpp"

#include "GLState.hpp"
#include "glad/gl.h"

#include "glm/mat4x4.hpp" // mat4, dmat4
#include "glm/gtc/type_ptr.hpp" //  glm::value_ptr

 Shader::Shader(const std::string& pName, GLState& pGLState)
	: mName(pName)
	, mSourcePath(File::GLSLShaderDirectory)
	, mTextureUnits(0)
{
	const std::string vertexShaderPath   = mSourcePath + mName + ".vert";
	const std::string fragmentShaderPath = mSourcePath + mName + ".frag";
	ZEPHYR_ASSERT(File::exists(vertexShaderPath), "Vertex shader does not exist at path {}", vertexShaderPath);
	ZEPHYR_ASSERT(File::exists(fragmentShaderPath), "Fragment shader does not exist at path {}", fragmentShaderPath);

	unsigned int vertexShader;
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		std::string source = File::readFromFile(vertexShaderPath);
		const char *vertexShaderSource = source.c_str();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		ZEPHYR_ASSERT(!hasCompileErrors(Type::Vertex, vertexShader), "Failed to compile vertex shader {}.vert", mName);
		initialiseRequiredAttributes(source);
	}

	unsigned int fragmentShader;
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		std::string source = File::readFromFile(fragmentShaderPath);
		const char *fragmentShaderSource = source.c_str();
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		ZEPHYR_ASSERT(!hasCompileErrors(Type::Fragment, fragmentShader), "Failed to compile fragment shader {}.frag", mName);
		initialiseRequiredAttributes(source);
	}

	{
		mHandle = glCreateProgram();
		glAttachShader(mHandle, vertexShader);
		glAttachShader(mHandle, fragmentShader);
		glLinkProgram(mHandle);
		ZEPHYR_ASSERT(!hasCompileErrors(Type::Program, mHandle), "Failed to link shader {}", mName)
	}

	{ // Setup the available texture units.
		// We have to tell OpenGL which texture unit each shader 'uniform sampler2D' belongs to by setting each sampler using glUniform1i.
		// We only have to set this once. This relies on initialiseRequiredAttributes() being called before to set mTextureUnits.
		if (mTextureUnits > 0)
		{
			use();
			for (int j = 0; j < mTextureUnits; j++)
			{
				const std::string textureUniformName = "texture" + std::to_string(j);
				setUniform(textureUniformName, j);
			}
		}
	}

	{ // Find and setup all the UniformBlocks in this Shader using OpenGL Program introspection.
		const int blockCount = GLState::getActiveUniformBlockCount(mHandle);
		for (int blockIndex = 0; blockIndex < blockCount; blockIndex++)
		{
			mUniformBlocks.push_back(GLState::getUniformBlock(mHandle, blockIndex));
			pGLState.bindUniformBlock(mUniformBlocks.back());
		}
	}

	// Delete the shaders after linking as they're no longer needed
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	ZEPHYR_ASSERT(mTextureUnits <= maxTextureUnits, "Texture units available must be below the max.");
	LOG_INFO("OpenGL::Shader '{}' loaded given ID: {}", mName, mHandle);
}

void Shader::initialiseRequiredAttributes(const std::string& pSourceCode)
{
	if (mRequiredAttributes.find(Attribute::Position3D) == mRequiredAttributes.end())
		if(pSourceCode.find(getAttributeName(Attribute::Position3D)) != std::string::npos)
			mRequiredAttributes.insert(Attribute::Position3D);

	if (mRequiredAttributes.find(Attribute::Normal3D) == mRequiredAttributes.end())
		if (pSourceCode.find(getAttributeName(Attribute::Normal3D)) != std::string::npos)
			mRequiredAttributes.insert(Attribute::Normal3D);

	if (mRequiredAttributes.find(Attribute::ColourRGB) == mRequiredAttributes.end())
		if (pSourceCode.find(getAttributeName(Attribute::ColourRGB)) != std::string::npos)
			mRequiredAttributes.insert(Attribute::ColourRGB);

	if (mRequiredAttributes.find(Attribute::TextureCoordinate2D) == mRequiredAttributes.end())
		if (pSourceCode.find(getAttributeName(Attribute::TextureCoordinate2D)) != std::string::npos)
			mRequiredAttributes.insert(Attribute::TextureCoordinate2D);

	mTextureUnits += findOccurrences(pSourceCode, "sampler2D");
	mTextureUnits += findOccurrences(pSourceCode, "samplerCube");

	ZEPHYR_ASSERT(!mRequiredAttributes.empty() && mRequiredAttributes.size() <= util::toIndex(Attribute::Count), "{} is not a valid number of attributes for a shader.", mRequiredAttributes.size());
}

void Shader::use() const
{
	glUseProgram(mHandle);
	shaderInUse = this;
}

int Shader::getAttributeLocation(const Attribute& pAttribute)
{
	int location = static_cast<int>(util::toIndex(pAttribute));
	return location;

	// Non static version of getAttributeLocation, not necessary as attribute locations are
	// consistent across all Zephyr GLSL shaders.

	//const GLint location = glGetAttribLocation(mHandle, getAttributeName(pAttribute).c_str());
	//ZEPHYR_ASSERT(location != -1, "Failed to find the location of {} in shader {}.", getAttributeName(pAttribute).c_str(), mName);
	//return static_cast<int>(location);
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
		ZEPHYR_ASSERT(false, "Could not determine the size of the attribute pAttribute");
		return 0;
	}
}

int Shader::getUniformLocation(const std::string &pName) const
{
	int location = glGetUniformLocation(mHandle, pName.c_str());
	ZEPHYR_ASSERT(location != GL_INVALID_VALUE, "ShaderID is not a value generated by OpenGL");
	ZEPHYR_ASSERT(location != GL_INVALID_OPERATION, "ShaderID is not a program object or has not been successfully linked");
	ZEPHYR_ASSERT(location != -1, "UniformName does not correspond to an active uniform variable in ShaderID or UniformName starts with the reserved prefix 'gl_'");
	return location;
}

bool Shader::checkForUseErrors(const Shader& pCalledFrom)
{
	if (!shaderInUse)
	{
		LOG_ERROR("No shader has been set to current in OpenGL state, call Shader::Use() before trying to set a uniform.");
		return false;
	}
	if (shaderInUse != &pCalledFrom)
	{
		LOG_ERROR("Trying to set a uniform on a shader not current in OpenGL state, call Shader::Use() before trying to set a uniform.");
		return false;
	}
	else
		return true;
}

void Shader::setUniform(const std::string& pName, const bool& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform1i(getUniformLocation(pName), (int)pValue); // Setting a boolean is treated as integer for gl shaders
}

void Shader::setUniform(const std::string& pName, const int& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform1i(getUniformLocation(pName), (GLint)pValue);
}

void Shader::setUniform(const std::string& pName, const float& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform1f(getUniformLocation(pName), pValue);
}

void Shader::setUniform(const std::string& pName, const glm::vec2& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform2fv(getUniformLocation(pName), 1, &pValue[0]);
}

void Shader::setUniform(const std::string &pName, const glm::vec3 &pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform3fv(getUniformLocation(pName), 1, &pValue[0]);
}

void Shader::setUniform(const std::string &pName, const glm::vec4 &pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform4fv(getUniformLocation(pName), 1, &pValue[0]);
}


void Shader::setUniform(const std::string& pName, const glm::mat2& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniformMatrix2fv(getUniformLocation(pName), 1, GL_FALSE, &pValue[0][0]);
}

void Shader::setUniform(const std::string& pName, const glm::mat3& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniformMatrix3fv(getUniformLocation(pName), 1, GL_FALSE, &pValue[0][0]);
}

void Shader::setUniform(const std::string& pName, const glm::mat4& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniformMatrix4fv(getUniformLocation(pName), 1, GL_FALSE, glm::value_ptr(pValue));
}

std::string Shader::getAttributeName(const Attribute& pAttribute)
{
	if (pAttribute == Attribute::Position3D)
		return "VertexPosition";
	else if (pAttribute == Attribute::Normal3D)
		return "VertexNormal";
	else if (pAttribute == Attribute::ColourRGB)
		return  "VertexColour";
	else if (pAttribute == Attribute::TextureCoordinate2D)
		return "VertexTexCoord";
	else
		ZEPHYR_ASSERT(false, "Could not convert Shader::Attribute '{}' to an std::string", pAttribute);
		return "";
}

bool Shader::hasCompileErrors(const Type& pType, const unsigned int pID)
{
	int success;
	if (pType == Type::Program)
	{
		glGetProgramiv(pID, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetProgramInfoLog(pID, 1024, NULL, infoLog);
			LOG_ERROR("Program linking failed with info: {}", infoLog);
			return true;
		}
	}
	else
	{
		glGetShaderiv(pID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetShaderInfoLog(pID, 1024, NULL, infoLog);
			LOG_ERROR("Shader compilation failed with info: {}", infoLog);
			return true;
		}
	}

	return false;
}

int Shader::findOccurrences(const std::string& pStringToSearch, const std::string& pSubStringToFind)
{
	int occurrences = 0;
	std::string::size_type position = 0;
	while ((position = pStringToSearch.find(pSubStringToFind, position)) != std::string::npos)
	{
		++occurrences;
		position += pSubStringToFind.length();
	}

	return occurrences;
}