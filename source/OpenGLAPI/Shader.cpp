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
		vertexShader = pGLState.CreateShader(GLType::ShaderProgramType::Vertex);
		std::string source = File::readFromFile(vertexShaderPath);
		pGLState.ShaderSource(vertexShader, source);
		pGLState.CompileShader(vertexShader);
		initialiseRequiredAttributes(source);
	}

	unsigned int fragmentShader;
	{
		fragmentShader = pGLState.CreateShader(GLType::ShaderProgramType::Fragment);
		std::string source = File::readFromFile(fragmentShaderPath);
		pGLState.ShaderSource(fragmentShader, source);
		pGLState.CompileShader(fragmentShader);
		initialiseRequiredAttributes(source);
	}

	{
		mHandle = pGLState.CreateProgram();
		pGLState.AttachShader(mHandle, vertexShader);
		pGLState.AttachShader(mHandle, fragmentShader);
		pGLState.LinkProgram(mHandle);
	}

	{ // Setup the available texture units.
		ZEPHYR_ASSERT(mTextureUnits <= maxTextureUnits, "Texture units available must be below the max.");
		// We have to tell OpenGL which texture unit each shader 'uniform sampler2D' belongs to by setting each sampler using glUniform1i.
		// We only have to set this once. This relies on initialiseRequiredAttributes() being called before to set mTextureUnits.
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

	{ // Find and setup all the UniformBlocks in this Shader using OpenGL Program introspection.
		const int blockCount = GLState::getActiveUniformBlockCount(mHandle);
		for (int blockIndex = 0; blockIndex < blockCount; blockIndex++)
		{
			mUniformBlocks.push_back(GLState::getUniformBlock(mHandle, blockIndex));
			pGLState.bindUniformBlock(mUniformBlocks.back());
		}
	}

	// Delete the shaders after linking as they're no longer needed
	pGLState.DeleteShader(vertexShader);
	pGLState.DeleteShader(fragmentShader);
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

void Shader::use(GLState& pGLState) const
{
	pGLState.UseProgram(mHandle);
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

void Shader::setUniform(GLState& pGLState, const  std::string& pName, const bool& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform1i(pGLState.GetUniformLocation(mHandle, pName), (int)pValue); // Setting a boolean is treated as integer for gl shaders
}

void Shader::setUniform(GLState& pGLState, const  std::string& pName, const int& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform1i(pGLState.GetUniformLocation(mHandle, pName), (GLint)pValue);
}

void Shader::setUniform(GLState& pGLState, const  std::string& pName, const float& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform1f(pGLState.GetUniformLocation(mHandle, pName), pValue);
}

void Shader::setUniform(GLState& pGLState, const  std::string& pName, const glm::vec2& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform2fv(pGLState.GetUniformLocation(mHandle, pName), 1, &pValue[0]);
}

void Shader::setUniform(GLState& pGLState, const  std::string &pName, const glm::vec3 &pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform3fv(pGLState.GetUniformLocation(mHandle, pName), 1, &pValue[0]);
}

void Shader::setUniform(GLState& pGLState, const  std::string &pName, const glm::vec4 &pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniform4fv(pGLState.GetUniformLocation(mHandle, pName), 1, &pValue[0]);
}


void Shader::setUniform(GLState& pGLState, const  std::string& pName, const glm::mat2& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniformMatrix2fv(pGLState.GetUniformLocation(mHandle, pName), 1, GL_FALSE, &pValue[0][0]);
}

void Shader::setUniform(GLState& pGLState, const  std::string& pName, const glm::mat3& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniformMatrix3fv(pGLState.GetUniformLocation(mHandle, pName), 1, GL_FALSE, &pValue[0][0]);
}

void Shader::setUniform(GLState& pGLState, const  std::string& pName, const glm::mat4& pValue) const
{
	ZEPHYR_ASSERT(checkForUseErrors(*this), "Trying to set uniforms on a shader ({}) without calling use() before.", mName);
	glUniformMatrix4fv(pGLState.GetUniformLocation(mHandle, pName), 1, GL_FALSE, glm::value_ptr(pValue));
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